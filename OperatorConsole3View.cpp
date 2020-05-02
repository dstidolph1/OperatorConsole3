
// OperatorConsole3View.cpp : implementation of the COperatorConsole3View class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "OperatorConsole3.h"
#endif

#include "OperatorConsole3Doc.h"
#include "OperatorConsole3View.h"
#include "SaveMultipleFramesInfo.h"
#include <gdiplus.h>
#include <wincodec.h>
#include <fstream>
#include "MatlabTestCode.h"
#include "MainFrm.h"

using namespace Gdiplus;

#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Windowscodecs.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Global declaration

//#define USE_SOFTWARE_FOR_STATION
#define NUM_AVERAGE_IMAGES 10

using _com_util::CheckError;
using container = std::vector<std::vector<bool>>;

_COM_SMARTPTR_TYPEDEF(IWICImagingFactory, __uuidof(IWICImagingFactory));
_COM_SMARTPTR_TYPEDEF(IWICBitmapEncoder, __uuidof(IWICBitmapEncoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameEncode, __uuidof(IWICBitmapFrameEncode));
_COM_SMARTPTR_TYPEDEF(IWICStream, __uuidof(IWICStream));
_COM_SMARTPTR_TYPEDEF(IWICPalette, __uuidof(IWICPalette));

// COperatorConsole3View

IMPLEMENT_DYNCREATE(COperatorConsole3View, CScrollView)

BEGIN_MESSAGE_MAP(COperatorConsole3View, CScrollView)
	ON_COMMAND(ID_CAMERA_OPERATORCONSOLELOCK, &COperatorConsole3View::OnCameraOperatorConsoleLock)
	ON_COMMAND(ID_CAMERA_PRESSOPERATORCONSOLEBUTTON, &COperatorConsole3View::OnCameraPressOperatorConsoleButton)
	ON_COMMAND(ID_CAMERA_USETOP8BITS, &COperatorConsole3View::OnCameraUseTop8Bits)
	ON_COMMAND(ID_CAMERA_USEMIDDLE8BITS, &COperatorConsole3View::OnCameraUseMiddle8Bits)
	ON_COMMAND(ID_CAMERA_USEBOTTOM8BITS, &COperatorConsole3View::OnCameraUseBottom8Bits)
	ON_COMMAND(ID_CAMERA_SAVESINGLEIMAGETODIS, &COperatorConsole3View::OnCameraSaveSingleImageToDisk)
	ON_COMMAND(ID_CAMERA_SAVESEQUENCETODISK, &COperatorConsole3View::OnCameraSaveSequenceToDisk)
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_DRAWREGISTRATIONMARKS, &COperatorConsole3View::OnViewDrawRegistrationMarks)
	ON_COMMAND(ID_VIEW_ZOOM14, &COperatorConsole3View::OnViewZoom14)
	ON_COMMAND(ID_VIEW_ZOOM12, &COperatorConsole3View::OnViewZoom12)
	ON_COMMAND(ID_VIEW_ZOOM11, &COperatorConsole3View::OnViewZoom11)
	ON_COMMAND(ID_VIEW_ZOOM21, &COperatorConsole3View::OnViewZoom21)
	ON_COMMAND(ID_VIEW_ZOOM31, &COperatorConsole3View::OnViewZoom31)
	ON_COMMAND(ID_VIEW_ZOOM41, &COperatorConsole3View::OnViewZoom41)
	ON_COMMAND(ID_CAMERA_SAVESINGLE10BITIMAGETOTIFFFILE, &COperatorConsole3View::OnCameraSaveSingle10BitImageToTiffFile)
	ON_COMMAND(ID_CAMERA_SAVESEQUENCE10BITIMAGESTOTIFFFILES, &COperatorConsole3View::OnCameraSaveSequence10BitImagesToTiffFiles)
	ON_COMMAND(ID_CAMERA_RUNFOCUSTESTING, &COperatorConsole3View::OnCameraRunfocustesting)
END_MESSAGE_MAP()

// COperatorConsole3View construction/destruction

COperatorConsole3View::COperatorConsole3View() noexcept : 
	m_ActiveTestRunning(false), m_SaveFrameCount(0), m_programState(eStateWaitForCameraLock),
	m_pBitmapInfo(nullptr), m_sizeBitmapInfo(0), m_OperatorConsoleLockEngaged(false),
	m_OperatorConsoleSwitchPressed(false), m_CameraRunning(false), m_ThreadRunning(false),
	m_ThreadShutdown(false), m_DrawingPicture(false), m_SaveEveryFrame8(false),
	m_SaveEveryFrame16(false), m_DrawRegistrationMarks(false), m_zoomDivision(1),
	m_zoomMultiplier(1), m_shrinkDisplay(false), m_magnifyDisplay(false),
	m_width(PICTURE_WIDTH),	m_height(PICTURE_HEIGHT), m_maxPixelValueInSquare(0), m_FrameNumber(0),
	m_MaxSaveFrames(10), m_RunFocusTest(false), m_pBitmapInfoSaved(nullptr), m_ShutdownEvent(FALSE, TRUE, NULL, NULL),
	m_MatlabImageTestReadyEvent(FALSE, TRUE, NULL, NULL), m_DrawFocusTestResults(false), m_bMagStripeEngaged(false),
	m_bCX3ButonPressed(false), m_bRunTestQuickMTF50(false), m_bRunTestFullChartMTF50(false), m_bRunTestFullChartSNR(false),
	m_DrawFullChartMTF50(false), m_DrawFullChartSNR(false)

{
	// TODO: add construction code here
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	registrationCoordinates.resize(4);
	registrationCoordinates[0] = CPoint(775, 604);
	registrationCoordinates[1] = CPoint(1825, 603);
	registrationCoordinates[2] = CPoint(1834, 1397);
	registrationCoordinates[3] = CPoint(776, 1395);
	
}

COperatorConsole3View::~COperatorConsole3View()
{
	if (m_pBitmapInfo)
	{
		delete m_pBitmapInfo;
		m_pBitmapInfo = nullptr;
	}
}

BOOL COperatorConsole3View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

void COperatorConsole3View::DisplayFocusResult(CDC* pDC, int x, int y, double value)
{
	CString sValue(std::to_string(value).c_str());
	pDC->TextOut(x, y, sValue);
}

void COperatorConsole3View::DrawRegistrationPoint(CDC *pDC, int x, int y)
{
	int radius = 53;
	if (m_shrinkDisplay)
	{
		radius = radius / m_zoomDivision;
		x = x / m_zoomDivision;
		y = y / m_zoomDivision;
	}
	else if (m_magnifyDisplay)
	{
		radius = radius * m_zoomMultiplier;
		x = x * m_zoomMultiplier;
		y = y * m_zoomMultiplier;
	}

	pDC->Ellipse(x - radius, y - radius, x + radius, y + radius);
	pDC->MoveTo(x, y - radius);
	pDC->LineTo(x, y + radius);
	pDC->MoveTo(x - radius, y);
	pDC->LineTo(x + radius, y);
}

// COperatorConsole3View drawing
void COperatorConsole3View::OnDraw(CDC* pDC)
{
	CScrollView::OnPrepareDC(pDC, NULL);

	int width = m_width;
	int height = m_height;
	if (m_shrinkDisplay)
	{
		width = m_width / m_zoomDivision;
		height = m_height / m_zoomDivision;
	}
	else if (m_magnifyDisplay)
	{
		width = m_width * m_zoomMultiplier;
		height = m_height * m_zoomMultiplier;
	}
	// Setting the StretchBltMode to COLORONCOLOR eliminates the horrible dithering for grayscale images.
	SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);
	::StretchDIBits(pDC->GetSafeHdc(), 0, 0, width, height, 0, 0, m_width, m_height, LPVOID(&m_image8Data[0]), m_pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	if (m_DrawRegistrationMarks)
	{
		HGDIOBJ originalPen = pDC->SelectObject(GetStockObject(DC_PEN));
		HGDIOBJ originalBrush = pDC->SelectObject(GetStockObject(HOLLOW_BRUSH));
		pDC->SetDCPenColor(RGB(0, 255, 0));
		DrawRegistrationPoint(pDC, 775, 604);
		DrawRegistrationPoint(pDC, 1825,603);
		DrawRegistrationPoint(pDC, 1834, 1397);
		DrawRegistrationPoint(pDC, 776, 1395);
		pDC->SelectObject(originalPen);
		pDC->SelectObject(originalBrush);
	}
	if (m_DrawFocusTestResults)
	{
		if (m_shrinkDisplay)
		{
			DisplayFocusResult(pDC, 1201 / m_zoomDivision, 515 / m_zoomDivision, m_outputQuickMTF50[0].mtf50);
			DisplayFocusResult(pDC, 815 / m_zoomDivision, 880 / m_zoomDivision, m_outputQuickMTF50[1].mtf50);
			DisplayFocusResult(pDC, 1205 / m_zoomDivision, 1145 / m_zoomDivision, m_outputQuickMTF50[2].mtf50);
			DisplayFocusResult(pDC, 1700 / m_zoomDivision, 950 / m_zoomDivision, m_outputQuickMTF50[3].mtf50);
			DisplayFocusResult(pDC, 1180 / m_zoomDivision, 1450 / m_zoomDivision, m_outputQuickMTF50[4].mtf50);
		}
		else if (m_magnifyDisplay)
		{
			DisplayFocusResult(pDC, 1201 * m_zoomMultiplier, 515 * m_zoomMultiplier, m_outputQuickMTF50[0].mtf50);
			DisplayFocusResult(pDC, 815 * m_zoomMultiplier, 880 * m_zoomMultiplier, m_outputQuickMTF50[1].mtf50);
			DisplayFocusResult(pDC, 1205 * m_zoomMultiplier, 1145 * m_zoomMultiplier, m_outputQuickMTF50[2].mtf50);
			DisplayFocusResult(pDC, 1700 * m_zoomMultiplier, 950 * m_zoomMultiplier, m_outputQuickMTF50[3].mtf50);
			DisplayFocusResult(pDC, 1180 * m_zoomMultiplier, 1450 * m_zoomMultiplier, m_outputQuickMTF50[4].mtf50);
		}
		else
		{
			DisplayFocusResult(pDC, 1201, 515, m_outputQuickMTF50[0].mtf50);
			DisplayFocusResult(pDC, 815, 880, m_outputQuickMTF50[1].mtf50);
			DisplayFocusResult(pDC, 1205, 1145, m_outputQuickMTF50[2].mtf50);
			DisplayFocusResult(pDC, 1700, 950, m_outputQuickMTF50[3].mtf50);
			DisplayFocusResult(pDC, 1180, 1450, m_outputQuickMTF50[4].mtf50);
		}
	}
	else if (m_DrawFullChartMTF50)
	{
		for (auto i = m_outputFullChartMTF50.begin(); i != m_outputFullChartMTF50.end(); i++)
		{
			DisplayFocusResult(pDC, i->x, i->y, i->mtf50);
		}
	}
	else if (m_DrawFullChartSNR)
	{
		size_t index = 0;
		for (auto i = m_outputFullChartSNR.begin(); i != m_outputFullChartSNR.end(); i++)
		{
			if (index < m_GreyBoxes.size())
			{
				int x = m_GreyBoxes[index].x;
				int y = m_GreyBoxes[index].y;
				DisplayFocusResult(pDC, x, y, double(index));
				DisplayFocusResult(pDC, x, y + 10, i->meanIntensity);
				DisplayFocusResult(pDC, x, y + 20, i->RMSNoise);
				DisplayFocusResult(pDC, x, y + 30, i->SignalToNoise);
				index++;
			}
		}
	}
}

bool COperatorConsole3View::OpenCSV(CFile& file, CString filename, CString headers)
{
	bool success = false;
	if (PathFileExists(filename.GetString()))
	{
		if (file.Open(filename, CFile::typeText | CFile::modeNoTruncate | CFile::modeWrite))
		{
			file.SeekToEnd();
			success = true;
		}
	}
	else // File does NOT exist, so we have to open and write headers
	{
		if (file.Open(filename, CFile::typeText | CFile::modeCreate | CFile::modeWrite))
		{
			file.Write(headers, headers.GetLength());
			file.Write("\n", 1);
			success = true;
		}
	}
	return success;
}

void COperatorConsole3View::InitPictureData()
{
	m_width = PICTURE_WIDTH;
	m_height = PICTURE_HEIGHT;
	const auto numPixels = m_width * m_height;
	if (m_pBitmapInfo)
	{
		delete m_pBitmapInfo;
		m_pBitmapInfo = nullptr;
	}
	m_sizeBitmapInfo = sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256);
	m_pBitmapInfo = (BITMAPINFO*)new uint8_t[m_sizeBitmapInfo];
	memset(m_pBitmapInfo, 0, m_sizeBitmapInfo);
	m_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo->bmiHeader.biWidth = m_width;
	m_pBitmapInfo->bmiHeader.biHeight = -m_height;
	m_pBitmapInfo->bmiHeader.biBitCount = 8;
	m_pBitmapInfo->bmiHeader.biPlanes = 1;
	m_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	m_pBitmapInfo->bmiHeader.biSizeImage = numPixels;
	m_pBitmapInfo->bmiHeader.biClrImportant = 0;
	m_pBitmapInfo->bmiHeader.biClrUsed = 0;
	m_pBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	for (int i = 0; i < 256; i++)
	{
		m_pBitmapInfo->bmiColors[i].rgbRed = i;
		m_pBitmapInfo->bmiColors[i].rgbGreen = i;
		m_pBitmapInfo->bmiColors[i].rgbBlue = i;
		m_pBitmapInfo->bmiColors[i].rgbReserved = 0;
	}
	m_image8Data.resize(numPixels);
	m_image8DataTesting.resize(numPixels);
	m_image16Data.resize(numPixels);
	m_image16DataTesting.resize(numPixels);
	m_image32Average.resize(numPixels);
}

void COperatorConsole3View::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	m_width = m_vidCapture.GetWidth();
	m_height = m_vidCapture.GetHeight();
	sizeTotal.cx = m_vidCapture.GetWidth();
	sizeTotal.cy = m_vidCapture.GetHeight();
	SetScrollSizes(MM_TEXT, sizeTotal);
	InitPictureData();
	m_OperatorConsoleLockEngaged = false;
	m_OperatorConsoleSwitchPressed = false;
	m_CameraRunning = false;
	m_ThreadRunning = false;
	m_ThreadShutdown = false;
	m_DrawingPicture = false;
	m_ActiveTestRunning = false;
	m_SaveFrameCount = 0;
	m_programState = eStateWaitForCameraLock;
	m_FrameNumber = 0;
	m_MaxSaveFrames = 10;
	LoadImage8ToFile(".\\CenteredImageAt150.bmp");
	m_GreyBoxes.push_back(CPoint(1353, 1334)); //1
	m_GreyBoxes.push_back(CPoint(1249, 1334)); //2
	m_GreyBoxes.push_back(CPoint(1454, 1310)); //3
	m_GreyBoxes.push_back(CPoint(1142, 1312)); //4
	m_GreyBoxes.push_back(CPoint(1559, 1253)); //5
	m_GreyBoxes.push_back(CPoint(1040, 1254)); //6
	m_GreyBoxes.push_back(CPoint(1615, 1152)); //7
	m_GreyBoxes.push_back(CPoint(996, 1150)); //8
	m_GreyBoxes.push_back(CPoint(1640, 945)); //9
	m_GreyBoxes.push_back(CPoint(957, 1047)); //10
	m_GreyBoxes.push_back(CPoint(1640, 945)); //11
	m_GreyBoxes.push_back(CPoint(957, 944)); //12
	m_GreyBoxes.push_back(CPoint(1616, 836)); //13
	m_GreyBoxes.push_back(CPoint(997, 838)); //14
	m_GreyBoxes.push_back(CPoint(1559, 734)); //15
	m_GreyBoxes.push_back(CPoint(1041, 733)); //16
	m_GreyBoxes.push_back(CPoint(1458, 677)); //17
	m_GreyBoxes.push_back(CPoint(1147, 675)); //18
	m_GreyBoxes.push_back(CPoint(1353, 652)); //19
	m_GreyBoxes.push_back(CPoint(1250, 651)); //20
	HRESULT hr = m_vidCapture.InitCOM();
	if (SUCCEEDED(hr))
	{
		OnCameraUseTop8Bits();
		AfxBeginThread(ThreadProc, this); // <<== START THE THREAD
		AfxBeginThread(AnalyzeFrameThreadProc, this);
	}
	else
	{
	}

}

#define VENDER_CMD_NONE 21
#define VENDER_CMD_POLL_MAG_STATUS 22
#define VENDER_CMD_POLL_BUTTON_STATUS 23

bool COperatorConsole3View::MagLockEngaged()
{
#if defined(USE_SOFTWARE_FOR_STATION)
	return m_bMagStripeEngaged;
#else
	bool magLockEngaged = false;
	if (m_vidCapture.CameraControl() && m_vidCapture.VideoProcAmp())
	{
		long value = -1, flags = 0;
		HRESULT hr = m_vidCapture.CameraControl()->Get(CameraControl_Focus, &value, &flags);
		hr = m_vidCapture.CameraControl()->Set(CameraControl_Focus, VENDER_CMD_POLL_MAG_STATUS, flags);
		hr = m_vidCapture.VideoProcAmp()->Get(VideoProcAmp_Hue, &value, &flags);
		magLockEngaged = (0 == value);
		hr = m_vidCapture.CameraControl()->Get(CameraControl_Focus, &value, &flags);
		hr = m_vidCapture.CameraControl()->Set(CameraControl_Focus, VENDER_CMD_NONE, flags);
	}
	return magLockEngaged;
#endif
}

bool COperatorConsole3View::SwitchPressed()
{
#if defined(USE_SOFTWARE_FOR_STATION)
	return m_bCX3ButonPressed;
#else
	bool switchPressed = false;
	if (m_vidCapture.CameraControl() && m_vidCapture.VideoProcAmp())
	{
		long value = -1, flags = 0;
		HRESULT hr = m_vidCapture.CameraControl()->Get(CameraControl_Focus, &value, &flags);
		hr = m_vidCapture.CameraControl()->Set(CameraControl_Focus, VENDER_CMD_POLL_BUTTON_STATUS, flags);
		hr = m_vidCapture.VideoProcAmp()->Get(VideoProcAmp_Hue, &value, &flags);
		switchPressed = (1 == value);
		hr = m_vidCapture.CameraControl()->Get(CameraControl_Focus, &value, &flags);
		hr = m_vidCapture.CameraControl()->Set(CameraControl_Focus, VENDER_CMD_NONE, flags);
	}
	return switchPressed;
#endif
}


OperatorConsoleState COperatorConsole3View::HandleWaitForCameraLock(bool newState)
{
	if (MagLockEngaged())
	{
		return eStateFocusingCamera;
	}
	else
	{
		m_DrawFocusTestResults = false;
		m_DrawFullChartMTF50 = false;
		m_DrawFullChartSNR = false;
		m_RunFocusTest = false;
		m_bRunTestQuickMTF50 = false;
		CDC* pDC = GetDC();
		if (pDC)
		{
			CRect rc;
			GetClientRect(&rc);
			CBrush brBkGnd;
			brBkGnd.CreateSolidBrush(RGB(252, 255, 255));
			int x = rc.Width() / 2 - 100;
			int y = rc.Height() / 2;
			pDC->FillRect(&rc, &brBkGnd);
			pDC->TextOut(x, y, "Magnet Switch not locked - video turned off");
			ReleaseDC(pDC);
		}
		return eStateWaitForCameraLock;
	}
}

OperatorConsoleState COperatorConsole3View::HandleFocusingCamera(bool newState)
{
	if (newState)
	{
		m_DrawFocusTestResults = false;
		m_DrawFullChartMTF50 = false;
		m_DrawFullChartSNR = false;
		m_RunFocusTest = true;
		m_CameraRunning = true;
		m_DrawingPicture = true;
		m_bRunTestQuickMTF50 = true;
		HRESULT hr = m_vidCapture.StartVideoCapture();
		if (SUCCEEDED(hr))
		{
			m_vidCapture.ReadCameraInfo(m_CameraInfo);
			m_CameraID = m_CameraInfo.ToString();
			PostMessage(MSG_SET_CAMERA_INFO, 0, reinterpret_cast<LPARAM>(&m_CameraInfo));
		}
		else
		{
			MessageBox("Failure to start video capture", "Error", MB_OK);
			return eStateWaitForCameraLock;
		}
	}
	CRect rcWhiteSquare(1188,582,1317,733);
	if (MagLockEngaged())
	{
		if (m_CameraRunning)
		{
			if (GetFrame(m_image8Data, m_image16Data))
			{
				if (m_maxPixelValueInSquare > 0)
				{
					ReadLock lock(m_pictureLock);
					CDC* pDC = GetDC();
					OnDraw(pDC);
					ReleaseDC(pDC);
					if (!m_ActiveTestRunning && m_bRunTestQuickMTF50)
					{
						m_ActiveTestRunning = true;
						//memcpy(&m_image8DataTesting[0], &m_image8Data[0], m_image8Data.size()); // Copy to backup which will be tested
						memcpy(&m_image8DataTesting[0], &m_image8Data[0], m_image8Data.size()); // Copy to backup which will be tested
						m_MatlabImageTestReadyEvent.SetEvent();
					}
				}
				if (m_SaveEveryFrame8 && (m_SaveFrameCount < m_MaxSaveFrames))
				{
					WriteLock lock(m_pictureLock);
					CString filename;
					filename.Format("%s\\%sFrame-%04d.bmp", m_PictureSavingFolder.GetBuffer(), m_PictureBaseName.GetBuffer(), m_FrameNumber);
					SaveImage8ToFile(filename);
					m_SaveFrameCount++;
					if (m_SaveFrameCount >= m_MaxSaveFrames)
					{
						m_SaveEveryFrame8 = false;
						CMenu* pMenu = GetParentMenu();
						if (pMenu)
							pMenu->CheckMenuItem(ID_CAMERA_SAVESEQUENCETODISK, MF_UNCHECKED | MF_BYCOMMAND);

					}
				}
				if (m_SaveEveryFrame16 && (m_SaveFrameCount < m_MaxSaveFrames))
				{
					WriteLock lock(m_pictureLock);
					CStringW filename;
					CStringW folder = UTF8toUTF16(m_PictureSavingFolder);
					CStringW baseName = UTF8toUTF16(m_PictureBaseName);
					filename.Format(L"%s\\%sFrame-%04d.tif", folder.GetBuffer(), baseName.GetBuffer(), m_FrameNumber);
					SaveImage16ToFile(filename);
					m_SaveFrameCount++;
					if (m_SaveFrameCount >= m_MaxSaveFrames)
					{
						m_SaveEveryFrame16 = false;
						CMenu* pMenu = GetParentMenu();
						if (pMenu)
							pMenu->CheckMenuItem(ID_CAMERA_SAVESEQUENCETODISK, MF_UNCHECKED | MF_BYCOMMAND);

					}
				}
			}
			if (SwitchPressed())
				return eStateTestingCamera;
			return eStateFocusingCamera;
		}
		else
			return eStateWaitForCameraLock;
	}
	else
		return eStateWaitForCameraLock;
}

bool COperatorConsole3View::GetFrame(std::vector<uint8_t>& image8Data, std::vector<uint16_t>& image16Data)
{
	bool success = false;
	CRect rcWhiteSquare(1188, 582, 1317, 733);
	{
		WriteLock lock(m_pictureLock);
		HRESULT hr = m_vidCapture.GetCameraFrame(image8Data, image16Data, rcWhiteSquare, m_maxPixelValueInSquare); // This will load the bitmap with the current frame
		m_FrameNumber++;
		success = SUCCEEDED(hr);
	}
	return success;
}

OperatorConsoleState COperatorConsole3View::HandleTestingCamera(bool newState)
{
	static bool makingAverage = false;
	static int imageCount = 0;
	m_bRunTestQuickMTF50 = false;
	m_DrawFocusTestResults = false;
	m_DrawFullChartMTF50 = false;
	m_DrawFullChartSNR = false;
	m_RunFocusTest = false;
	if (MagLockEngaged())
	{
		if (newState)
		{
			m_bRunTestFullChartSNR = true;
			makingAverage = true;
			imageCount = 0;
			// clear image average buffer
			size_t size = m_image32Average.size();
			size_t sizeElement = sizeof(m_image32Average[0]);
			size_t sizeMem = size * sizeElement;
			memset(&m_image32Average[0], 0, sizeMem);
		}
		bool frameValid = GetFrame(m_image8Data, m_image16Data);
		if (frameValid)
		{
			ReadLock lock(m_pictureLock);
			CDC* pDC = GetDC();
			OnDraw(pDC);
			ReleaseDC(pDC);
			m_image16DataTesting = m_image16Data;
			m_MatlabImageTestReadyEvent.SetEvent(); // go ahead with the first test - just once
			if (makingAverage)
			{
				imageCount++;
				if (imageCount < NUM_AVERAGE_IMAGES)
				{
					for (size_t i = 0; i < m_image16Data.size(); i++)
					{
						m_image32Average[i] += m_image16Data[i];
					}
				}
				else
				{
					makingAverage = false;
					for (size_t i = 0; i < m_image16Data.size(); i++)
					{
						m_image16DataTesting[i] = m_image32Average[i] / imageCount;
					}
					m_bRunTestFullChartSNR = false;
					m_bRunTestFullChartMTF50 = true;
					m_MatlabImageTestReadyEvent.SetEvent();
				}
			}
		}
		// do test
		return eStateTestingCamera;
	}
	else
		return eStateWaitForCameraLock;
}

OperatorConsoleState COperatorConsole3View::HandleReportResults(bool newState)
{
	if (MagLockEngaged())
	{
		if (newState)
		{
			m_DrawFocusTestResults = false;
			m_DrawFullChartMTF50 = false;
			m_DrawFullChartSNR = false;
			m_RunFocusTest = false;
			m_bRunTestQuickMTF50 = false;
		}
		return eStateReportResults;
	}
	else
		return eStateWaitForCameraLock;
}

UINT __cdecl COperatorConsole3View::AnalyzeFrameThreadProc(LPVOID pParam)
{
	COperatorConsole3View* pThis = (COperatorConsole3View*)pParam;
	HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr) && pThis->m_matlabTestCode.Initialize())
	{
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		LARGE_INTEGER timeStart, timeEnd;
		HANDLE handles[2];
		handles[0] = pThis->m_ShutdownEvent;
		handles[1] = pThis->m_MatlabImageTestReadyEvent;
		while (!pThis->m_ThreadShutdown)
		{
			DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			if ((WAIT_OBJECT_0+1) == waitResult)
			{
				::QueryPerformanceCounter(&timeStart);
				pThis->m_ActiveTestRunning = true;
				CFile file;
				if (pThis->m_bRunTestQuickMTF50)
				{
					pThis->m_DrawFocusTestResults = pThis->m_matlabTestCode.RunTestQuickMTF50(pThis->m_image8DataTesting, pThis->m_width, pThis->m_height, pThis->registrationCoordinates, pThis->m_outputQuickMTF50);
					if (pThis->m_DrawFocusTestResults &&
						pThis->OpenCSV(file, "FocusTest.csv", "CameraID,FrameNum,MTF50-1,MTF50-2,MTF50-3,MTF50-4,MTF50-5"))
					{
						CString sText;
						sText.Format("%s,%d", pThis->m_CameraID.c_str(), pThis->m_FrameNumber);
						file.Write(sText.GetString(), sText.GetLength());
						for (auto i = pThis->m_outputQuickMTF50.begin(); i != pThis->m_outputQuickMTF50.end(); i++)
						{
							file.Write(",", 1);
							std::string text = std::to_string(i->mtf50);
							file.Write(text.c_str(), UINT(text.length()));
						}
						file.Write("\n", 1);
					}
				}
				else if (pThis->m_bRunTestFullChartMTF50)
				{
					pThis->m_DrawFullChartMTF50 = pThis->m_matlabTestCode.RunTestFullChartMTF50(pThis->m_image16DataTesting, pThis->m_width, pThis->m_height, pThis->m_outputFullChartMTF50);
					if (pThis->m_DrawFullChartMTF50)
					{
						std::string header = "CameraID,FrameNum";
						for (size_t i = 0; i < pThis->m_outputFullChartMTF50.size(); i++)
						{
							header += ",x-" + std::to_string(i) + ",y-" + std::to_string(i) + ",MFT50-" + std::to_string(i);
						}
						if (pThis->OpenCSV(file, "FullChartMTF50.csv", header.c_str()))
						{
							CString sText;
							sText.Format("%s,%d", pThis->m_CameraID.c_str(), pThis->m_FrameNumber);
							file.Write(sText.GetString(), sText.GetLength());
							for (auto i = pThis->m_outputFullChartMTF50.begin(); i != pThis->m_outputFullChartMTF50.end(); i++)
							{
								std::string text = std::to_string(i->x);
								file.Write(",", 1);
								file.Write(text.c_str(), UINT(text.length()));
								text = std::to_string(i->y);
								file.Write(",", 1);
								file.Write(text.c_str(), UINT(text.length()));
								text = std::to_string(i->mtf50);
								file.Write(",", 1);
								file.Write(text.c_str(), UINT(text.length()));
							}
							file.Write("\n", 1);
						}
					}
				}
				else if (pThis->m_bRunTestFullChartSNR)
				{
					pThis->m_DrawFullChartSNR = pThis->m_matlabTestCode.RunTestFullChartSNR(pThis->m_image16DataTesting, pThis->m_width, pThis->m_height, pThis->m_outputFullChartSNR);
					if (pThis->m_DrawFullChartSNR)
					{
						std::string header = "CameraID,FrameNum";
						for (size_t i = 0; i < pThis->m_outputFullChartSNR.size(); i++)
						{
							header += ",MeanIntensity-" + std::to_string(i);
							header += ",RMSNoise-" + std::to_string(i);
							header += ",SignalToNoise-" + std::to_string(i);
						}
						if (pThis->OpenCSV(file, "FullChartSNR.csv", header.c_str()))
						{
							CString sText;
							sText.Format("%s,%d", pThis->m_CameraID.c_str(), pThis->m_FrameNumber);
							file.Write(sText.GetString(), sText.GetLength());
							for (auto i = pThis->m_outputFullChartSNR.begin(); i != pThis->m_outputFullChartSNR.end(); i++)
							{
								file.Write(",", 1);
								std::string text = std::to_string(i->meanIntensity);
								file.Write(text.c_str(), UINT(text.length()));
								file.Write(",", 1);
								text = std::to_string(i->meanIntensity);
								file.Write(text.c_str(), UINT(text.length()));
								file.Write(",", 1);
								text = std::to_string(i->meanIntensity);
								file.Write(text.c_str(), UINT(text.length()));
							}
							file.Write("\n", 1);
						}
					}
					pThis->m_bRunTestFullChartSNR = false; // just run one time
				}
				pThis->m_MatlabImageTestReadyEvent.ResetEvent();
				pThis->m_ActiveTestRunning = false;
				::QueryPerformanceCounter(&timeEnd);
				LONGLONG diff = (timeEnd.QuadPart - timeStart.QuadPart) / (Frequency.QuadPart / 1000);
			}
		}
		pThis->m_matlabTestCode.Shutdown();
	}
	return 0;
}

UINT __cdecl COperatorConsole3View::ThreadProc(LPVOID pParam)
{
	COperatorConsole3View* pThis = (COperatorConsole3View*)pParam;
	pThis->m_ThreadRunning = true;
	HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	DWORD dwFrameTime = 1000 / 13; // 66 ms
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	LARGE_INTEGER timeStart, timeEnd;
	bool stateChange = true;
	OperatorConsoleState nextState = pThis->m_programState;
	while (!pThis->m_ThreadShutdown)
	{
		::QueryPerformanceCounter(&timeStart);
		if (pThis->m_vidCapture.CameraControl())
		{
			if (pThis->m_ThreadShutdown || (!pThis->MagLockEngaged() && pThis->m_CameraRunning))
			{
				pThis->m_vidCapture.vcStopCaptureVideo();
				pThis->m_CameraRunning = false;
			}
			switch (pThis->m_programState)
			{
			case eStateWaitForCameraLock:
				nextState = pThis->HandleWaitForCameraLock(stateChange);
				break;
			case eStateFocusingCamera:
				nextState = pThis->HandleFocusingCamera(stateChange);
				break;
			case eStateTestingCamera:
				nextState = pThis->HandleTestingCamera(stateChange);
				break;
			case eStateReportResults:
				nextState = pThis->HandleReportResults(stateChange);
				break;
			}
			if (nextState != pThis->m_programState)
			{
				pThis->m_programState = nextState;
				stateChange = true;
			}
			else
				stateChange = false;
		}
		QueryPerformanceCounter(&timeEnd);
		LONGLONG diff = (timeEnd.QuadPart - timeStart.QuadPart) / (Frequency.QuadPart / 1000);
		int nSleepTime = dwFrameTime - int(diff);
		if ((nSleepTime < 0) || (nSleepTime > 1000))
			nSleepTime = 1;
		char buffer[MAX_PATH];
		sprintf_s(buffer, sizeof(buffer), "Frame took %lldms, sleeping for %dms\n", diff, nSleepTime);
		OutputDebugStringA(buffer);
		Sleep(nSleepTime);
	}
	pThis->m_ThreadRunning = false;
	return 0;   // thread completed successfully
}

void COperatorConsole3View::SaveImage16ToFile(const wchar_t *pathname)
{
	// Create factory
	IWICImagingFactoryPtr sp_factory { nullptr };
	HRESULT hr = sp_factory.CreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER);
	if (SUCCEEDED(hr))
	{
		// Create stream
		IWICStreamPtr sp_stream{ nullptr };
		hr = sp_factory->CreateStream(&sp_stream);
		if (SUCCEEDED(hr))
		{
			hr = sp_stream->InitializeFromFilename(pathname, GENERIC_WRITE);
			// Create encoder
			IWICBitmapEncoderPtr sp_encoder{ nullptr };
			hr = sp_factory->CreateEncoder(GUID_ContainerFormatTiff, nullptr, &sp_encoder);
			if (SUCCEEDED(hr))
			{
				// Initialize encoder with stream
				hr = sp_encoder->Initialize(sp_stream, WICBitmapEncoderNoCache);
				if (SUCCEEDED(hr))
				{
					// Create new frame
					IWICBitmapFrameEncodePtr sp_frame{ nullptr };
					IPropertyBag2Ptr sp_properties{ nullptr };
					hr = sp_encoder->CreateNewFrame(&sp_frame, &sp_properties);
					if (SUCCEEDED(hr))
					{
						// This is how you customize the TIFF output.
						PROPBAG2 option = { 0 };
						option.pstrName = L"TiffCompressionMethod";
						VARIANT varValue;
						VariantInit(&varValue);
						varValue.vt = VT_UI1;
						varValue.bVal = WICTiffCompressionZIP;
						hr = sp_properties->Write(1, &option, &varValue);
						if (SUCCEEDED(hr))
						{
							hr = sp_frame->Initialize(sp_properties);
						}
						if (SUCCEEDED(hr))
						{
							hr = sp_frame->SetSize(m_width, m_height);
						}
						if (SUCCEEDED(hr))
						{
							// Set pixel format
							// SetPixelFormat() requires a pointer to non-const
							auto pf{ GUID_WICPixelFormat16bppGray };
							hr = sp_frame->SetPixelFormat(&pf);
							if (!::IsEqualGUID(pf, GUID_WICPixelFormat16bppGray))
							{
								// Report unsupported pixel format
								CheckError(WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT);
							}
						}
						if (SUCCEEDED(hr))
						{
							UINT stride = m_width * 2;
							UINT bufferSize = m_height * stride;
							hr = sp_frame->WritePixels(m_height, stride, bufferSize, LPBYTE(&m_image16Data[0]));
						}
						// Commit frame
						if (SUCCEEDED(hr))
							sp_frame->Commit();
					}
					// Commit image
					if (SUCCEEDED(hr))
						sp_encoder->Commit();
				}
			}
		}
	}
}

bool COperatorConsole3View::LoadImage8ToFile(const char* filename)
{
	bool success = false;
	// The file... We open it with it's constructor
	std::ifstream file(filename, std::ios::binary);
	if (file)
	{
		BITMAPFILEHEADER bmpFileHeader;
		size_t sizeBitmapInfo = sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD));
		m_pBitmapInfoSaved = (BITMAPINFO*)new uint8_t[sizeBitmapInfo];
		file.read((char*)&bmpFileHeader, sizeof(BITMAPFILEHEADER));
		file.read((char*)m_pBitmapInfoSaved, sizeBitmapInfo);
		auto width = m_pBitmapInfoSaved->bmiHeader.biWidth;
		auto height = m_pBitmapInfoSaved->bmiHeader.biHeight;
		if (height < 0)
			height = -height;
		auto imageSize = width * height;
		m_savedImageData.resize(imageSize);
		file.read((char*)&m_savedImageData[0], imageSize);
		success = true;
	}
	else
	{
		std::cout << "Failure to open bitmap file.\n";
	}
	return success;
}

void COperatorConsole3View::SaveImage8ToFile(const char* filename)
{
	BITMAPFILEHEADER bmfh = { 0 };
	bmfh.bfType = 0x4d42;  // 'BM'
	int nSizeHdr = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
	DWORD dwSizeImage = PICTURE_WIDTH * PICTURE_HEIGHT;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + nSizeHdr + dwSizeImage;
	// meaning of bfSize open to interpretation (bytes, words, dwords?) -- we won't use it
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
	try {
		CFile file;
		if (file.Open(filename, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive))
		{
			file.Write((LPVOID)&bmfh, sizeof(BITMAPFILEHEADER));
			file.Write((LPVOID)m_pBitmapInfo, m_sizeBitmapInfo);
			{
				ReadLock lock(m_pictureLock);
				UINT cbSize = UINT(m_image8Data.size());
				file.Write((LPVOID)&m_image8Data[0], cbSize);
}
			file.Close();
		}
	}
	catch (CException* pe) {
		pe->Delete();
	}

}

#ifdef _DEBUG
void COperatorConsole3View::AssertValid() const
{
	CScrollView::AssertValid();
}

void COperatorConsole3View::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

COperatorConsole3Doc* COperatorConsole3View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COperatorConsole3Doc)));
	return (COperatorConsole3Doc*)m_pDocument;
}
#endif //_DEBUG

// COperatorConsole3View message handlers

CMenu* COperatorConsole3View::GetParentMenu()
{
	CMenu* menu = AfxGetMainWnd()->GetMenu();
	return menu;
}

void COperatorConsole3View::OnCameraOperatorConsoleLock()
{
	m_OperatorConsoleLockEngaged = !m_OperatorConsoleLockEngaged;
	m_bMagStripeEngaged = m_OperatorConsoleLockEngaged;
	CMenu* pMenu = GetParentMenu();
	if (pMenu)
	{
		if (m_OperatorConsoleLockEngaged)
		{
			pMenu->CheckMenuItem(ID_CAMERA_OPERATORCONSOLELOCK, MF_CHECKED | MF_BYCOMMAND);
		}
		else
		{
			pMenu->CheckMenuItem(ID_CAMERA_OPERATORCONSOLELOCK, MF_UNCHECKED | MF_BYCOMMAND);

		}
	}
}

void COperatorConsole3View::OnDestroy()
{
	// TODO: Add your message handler code here and/or call default
	m_ThreadShutdown = true;
	while (m_ThreadRunning)
	{
		Sleep(1);
	}
	CScrollView::OnDestroy();
}

void COperatorConsole3View::OnCameraPressOperatorConsoleButton()
{
	m_OperatorConsoleSwitchPressed = !m_OperatorConsoleSwitchPressed;
	if (m_OperatorConsoleSwitchPressed) {
		CMenu* pMenu = GetParentMenu();
		pMenu->CheckMenuItem(ID_CAMERA_PRESSOPERATORCONSOLEBUTTON, MF_CHECKED | MF_BYCOMMAND);
	}
	else {
		CMenu* pMenu = GetParentMenu();
		pMenu->CheckMenuItem(ID_CAMERA_PRESSOPERATORCONSOLEBUTTON, MF_UNCHECKED | MF_BYCOMMAND);
	}
}


void COperatorConsole3View::OnCameraUseTop8Bits()
{
	CMenu* pMenu = GetParentMenu();
	if (pMenu && ::IsMenu(pMenu->GetSafeHmenu()))
	{
		pMenu->CheckMenuItem(ID_CAMERA_USETOP8BITS, MF_CHECKED | MF_BYCOMMAND);
		pMenu->CheckMenuItem(ID_CAMERA_USEMIDDLE8BITS, MF_UNCHECKED | MF_BYCOMMAND);
		pMenu->CheckMenuItem(ID_CAMERA_USEBOTTOM8BITS, MF_UNCHECKED | MF_BYCOMMAND);
	}
	m_vidCapture.SetBitShift(BitShiftType::shift2);
}


void COperatorConsole3View::OnCameraUseMiddle8Bits()
{
	CMenu* pMenu = GetParentMenu();
	if (pMenu && ::IsMenu(pMenu->GetSafeHmenu()))
	{
		pMenu->CheckMenuItem(ID_CAMERA_USETOP8BITS, MF_UNCHECKED | MF_BYCOMMAND);
		pMenu->CheckMenuItem(ID_CAMERA_USEMIDDLE8BITS, MF_CHECKED | MF_BYCOMMAND);
		pMenu->CheckMenuItem(ID_CAMERA_USEBOTTOM8BITS, MF_UNCHECKED | MF_BYCOMMAND);
	}
	m_vidCapture.SetBitShift(BitShiftType::shift1);
}


void COperatorConsole3View::OnCameraUseBottom8Bits()
{
	CMenu* pMenu = GetParentMenu();
	if (pMenu && ::IsMenu(pMenu->GetSafeHmenu()))
	{
		pMenu->CheckMenuItem(ID_CAMERA_USETOP8BITS, MF_UNCHECKED | MF_BYCOMMAND);
		pMenu->CheckMenuItem(ID_CAMERA_USEMIDDLE8BITS, MF_UNCHECKED | MF_BYCOMMAND);
		pMenu->CheckMenuItem(ID_CAMERA_USEBOTTOM8BITS, MF_CHECKED | MF_BYCOMMAND);
	}
	m_vidCapture.SetBitShift(BitShiftType::shift0);
}


void COperatorConsole3View::OnCameraSaveSingleImageToDisk()
{
	char strFilter[] = { "BMP Files (*.bmp)|*.bmp|" };
	CFileDialog FileDlg(FALSE, CString(".bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CString(strFilter));
	FileDlg.m_ofn.lpstrInitialDir = m_PictureSavingFolder;
	if (FileDlg.DoModal() == IDOK)
	{
		WriteLock lock(m_pictureLock);
		m_PictureSavingFolder = FileDlg.GetFolderPath();
		CString filename = FileDlg.GetPathName();
		SaveImage8ToFile(filename);
	}
}


void COperatorConsole3View::OnCameraSaveSequenceToDisk()
{
	if (!m_SaveEveryFrame8)
	{
		SaveMultipleFramesInfo dlg;
		SYSTEMTIME now;
		GetLocalTime(&now);
		m_PictureBaseName.Format("%04d-%02d-%02d_%02d-%02d-%02d_", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond);
		dlg.FilenameLeadingText = m_PictureBaseName;
		dlg.MaxFrameCount = std::to_string(m_MaxSaveFrames).c_str();
		dlg.FolderPath = m_PictureSavingFolder;
		if (dlg.DoModal() == IDOK)
		{
			m_PictureSavingFolder = dlg.FolderPath;
			m_MaxSaveFrames = atoi(dlg.MaxFrameCount.GetBuffer());
			m_PictureBaseName = dlg.FilenameLeadingText;
			m_FrameNumber = 0;
			m_SaveFrameCount = 0;
			m_SaveEveryFrame8 = true;
			CMenu* pMenu = GetParentMenu();
			pMenu->CheckMenuItem(ID_CAMERA_SAVESEQUENCETODISK, MF_CHECKED | MF_BYCOMMAND);
		}
	}
	else
	{
		m_SaveEveryFrame8 = false;
		CMenu* pMenu = GetParentMenu();
		pMenu->CheckMenuItem(ID_CAMERA_SAVESEQUENCETODISK, MF_UNCHECKED | MF_BYCOMMAND);
	}
}

afx_msg void COperatorConsole3View::OnMouseMove(UINT nFlags, CPoint point)
{

	if (m_CameraRunning)
	{
		CString text;
		int color = -1;
		CPoint ptScroll = GetDeviceScrollPosition();
		point += ptScroll;
		if ((point.y >= 0) && (point.y <= m_height))
		{
			if ((point.x >= 0) && (point.x <= m_width))
			{
				int offset = point.y * m_width + point.x;
				color = m_image8Data[offset];
			}
		}
		text.Format("Color %d at %d,%d - highest pixel value in white square = %d", color, point.x, point.y, m_maxPixelValueInSquare);
		SetStatusBarText(text);
	}
	CScrollView::OnMouseMove(nFlags, point);
}

void COperatorConsole3View::SetStatusBarText(CString text)
{
	CStatusBar* pStatusBar = (CStatusBar*)AfxGetApp()->m_pMainWnd->GetDescendantWindow(AFX_IDW_STATUS_BAR);
	if (pStatusBar)
		pStatusBar->SetPaneText(0, text);
}

void COperatorConsole3View::OnViewDrawRegistrationMarks()
{
	m_DrawRegistrationMarks = !m_DrawRegistrationMarks;
	if (m_DrawRegistrationMarks)
	{
		CMenu* pMenu = GetParentMenu();
		pMenu->CheckMenuItem(ID_VIEW_DRAWREGISTRATIONMARKS, MF_CHECKED | MF_BYCOMMAND);
	}
	else
	{
		CMenu* pMenu = GetParentMenu();
		pMenu->CheckMenuItem(ID_VIEW_DRAWREGISTRATIONMARKS, MF_UNCHECKED | MF_BYCOMMAND);
	}
}


void COperatorConsole3View::OnViewZoom14()
{
	m_magnifyDisplay = false;
	m_shrinkDisplay = true;
	m_zoomDivision = 4;
	m_zoomMultiplier = 1;
	CMenu* pMenu = GetParentMenu();
	pMenu->CheckMenuItem(ID_VIEW_ZOOM14, MF_CHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM12, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM11, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM21, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM31, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM41, MF_UNCHECKED | MF_BYCOMMAND);
	CSize sizeTotal;
	sizeTotal.cx = m_width / 4;
	sizeTotal.cy = m_height / 4;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


void COperatorConsole3View::OnViewZoom12()
{
	m_magnifyDisplay = false;
	m_shrinkDisplay = true;
	m_zoomDivision = 2;
	m_zoomMultiplier = 1;
	CMenu* pMenu = GetParentMenu();
	pMenu->CheckMenuItem(ID_VIEW_ZOOM14, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM12, MF_CHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM11, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM21, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM31, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM41, MF_UNCHECKED | MF_BYCOMMAND);
	CSize sizeTotal;
	sizeTotal.cx = m_width/2;
	sizeTotal.cy = m_height/2;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


void COperatorConsole3View::OnViewZoom11()
{
	m_magnifyDisplay = false;
	m_shrinkDisplay = false;
	m_zoomDivision = 1;
	m_zoomMultiplier = 1;
	CMenu* pMenu = GetParentMenu();
	pMenu->CheckMenuItem(ID_VIEW_ZOOM14, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM12, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM11, MF_CHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM21, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM31, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM41, MF_UNCHECKED | MF_BYCOMMAND);
	CSize sizeTotal;
	sizeTotal.cx = m_width;
	sizeTotal.cy = m_height;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


void COperatorConsole3View::OnViewZoom21()
{
	m_magnifyDisplay = true;
	m_shrinkDisplay = false;
	m_zoomDivision = 1;
	m_zoomMultiplier = 2;
	CMenu* pMenu = GetParentMenu();
	pMenu->CheckMenuItem(ID_VIEW_ZOOM14, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM12, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM11, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM21, MF_CHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM31, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM41, MF_UNCHECKED | MF_BYCOMMAND);
	CSize sizeTotal;
	sizeTotal.cx = m_width*2;
	sizeTotal.cy = m_height*2;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


void COperatorConsole3View::OnViewZoom31()
{
	m_magnifyDisplay = true;
	m_shrinkDisplay = false;
	m_zoomDivision = 1;
	m_zoomMultiplier = 3;
	CMenu* pMenu = GetParentMenu();
	pMenu->CheckMenuItem(ID_VIEW_ZOOM14, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM12, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM11, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM21, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM31, MF_CHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM41, MF_UNCHECKED | MF_BYCOMMAND);
	CSize sizeTotal;
	sizeTotal.cx = m_width*3;
	sizeTotal.cy = m_height*3;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


void COperatorConsole3View::OnViewZoom41()
{
	m_magnifyDisplay = true;
	m_shrinkDisplay = false;
	m_zoomDivision = 1;
	m_zoomMultiplier = 4;
	CMenu* pMenu = GetParentMenu();
	pMenu->CheckMenuItem(ID_VIEW_ZOOM14, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM12, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM11, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM21, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM31, MF_UNCHECKED | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_VIEW_ZOOM41, MF_CHECKED | MF_BYCOMMAND);
	CSize sizeTotal;
	sizeTotal.cx = m_width*4;
	sizeTotal.cy = m_height*4;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

CStringW COperatorConsole3View::UTF8toUTF16(const CStringA& utf8)
{
	CStringW utf16;
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if (len > 1)
	{
		wchar_t* ptr = utf16.GetBuffer(len - 1);
		if (ptr) MultiByteToWideChar(CP_UTF8, 0, utf8, -1, ptr, len);
		utf16.ReleaseBuffer();
	}
	return utf16;
}
void COperatorConsole3View::OnCameraSaveSingle10BitImageToTiffFile()
{
	char strFilter[] = { "TIF Files (*.tif)|*.tif|" };
	CFileDialog FileDlg(FALSE, CString(".tif"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CString(strFilter), this, 0, TRUE);
	FileDlg.m_ofn.lpstrInitialDir = m_PictureSavingFolder;
	if (FileDlg.DoModal() == IDOK)
	{
		WriteLock lock(m_pictureLock);
		m_PictureSavingFolder = FileDlg.GetFolderPath();
		CStringW filename = UTF8toUTF16(FileDlg.GetPathName());
		SaveImage16ToFile(filename);
	}
}

void COperatorConsole3View::OnCameraSaveSequence10BitImagesToTiffFiles()
{
	if (!m_SaveEveryFrame16)
	{
		SaveMultipleFramesInfo dlg;
		SYSTEMTIME now;
		GetLocalTime(&now);
		m_PictureBaseName.Format("%04d-%02d-%02d_%02d-%02d-%02d_", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond);
		dlg.FilenameLeadingText = m_PictureBaseName;
		dlg.MaxFrameCount = std::to_string(m_MaxSaveFrames).c_str();
		dlg.FolderPath = m_PictureSavingFolder;
		if (dlg.DoModal() == IDOK)
		{
			m_PictureSavingFolder = dlg.FolderPath;
			m_MaxSaveFrames = atoi(dlg.MaxFrameCount.GetBuffer());
			m_PictureBaseName = dlg.FilenameLeadingText;
			m_FrameNumber = 0;
			m_SaveFrameCount = 0;
			m_SaveEveryFrame16 = true;
			CMenu* pMenu = GetParentMenu();
			pMenu->CheckMenuItem(ID_CAMERA_SAVESEQUENCETODISK, MF_CHECKED | MF_BYCOMMAND);
		}
	}
	else
	{
		m_SaveEveryFrame16 = false;
		CMenu* pMenu = GetParentMenu();
		pMenu->CheckMenuItem(ID_CAMERA_SAVESEQUENCETODISK, MF_UNCHECKED | MF_BYCOMMAND);
	}
}


void COperatorConsole3View::OnCameraRunfocustesting()
{
	m_RunFocusTest = !m_RunFocusTest;
	if (m_RunFocusTest)
	{
		CMenu* pMenu = GetParentMenu();
		pMenu->CheckMenuItem(ID_CAMERA_RUNFOCUSTESTING, MF_CHECKED | MF_BYCOMMAND);
	}
	else
	{
		CMenu* pMenu = GetParentMenu();
		pMenu->CheckMenuItem(ID_CAMERA_RUNFOCUSTESTING, MF_UNCHECKED | MF_BYCOMMAND);
	}
}
