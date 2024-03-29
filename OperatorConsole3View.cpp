
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
#include "EyelockCamera.h"

using namespace Gdiplus;

#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Windowscodecs.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Global declaration
//#define USE_SOFTWARE_FOR_STATION
//#define DRAW_MAIN_THREAD

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
	m_OperatorConsoleSwitchPressed(false), m_CameraRunning(false), m_ProgramStateThreadRunning(false),
	m_ThreadShutdown(false), m_SaveEveryFrame8(false),
	m_SaveEveryFrame16(false), m_DrawRegistrationMarks(true), m_zoomDivision(1),
	m_zoomMultiplier(1), m_shrinkDisplay(false), m_magnifyDisplay(false),
	m_maxPixelValueInSquare(0), m_FrameNumber(0),
	m_MaxSaveFrames(10), m_RunFocusTest(false), m_pBitmapInfoSaved(nullptr), m_ShutdownEvent(FALSE, TRUE, NULL, NULL),
	m_MatlabImageTestReadyEvent(FALSE, FALSE, NULL, NULL), m_DrawFocusTestResults(false), m_bMagStripeEngaged(false),
	m_bCX3ButonPressed(false), m_bRunTestQuickMTF50(false), m_bRunTestFullChartMTF50(false), m_bRunTestFullChartSNR(false),
	m_DrawFullChartMTF50(false), m_DrawFullChartSNR(false), m_bFullChartMTF50Done(false), m_bFullChartSNRDone(false),
	m_bQuickMTF50Done(false), m_stateChange(false), m_TestingThreadRunning(false),
	m_pProgramStateThread(nullptr), m_pTestingThread(nullptr), m_numImagesToAverage(20), m_regPtMoving(-1),
	m_ShowInstallDiffusionFilter(false), m_ShowRemoveDiffusionFilter(false), m_ShowTestingDiffusionFilter(false),
	m_bOverrideMagLock(false)
{
	// TODO: add construction code here
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (prop.Load())
	{
		size_t nFiducials = prop.m_Fiducials.size();
		registrationCoordinates.resize(nFiducials);
		for (size_t i = 0; i < nFiducials; i++)
		{
			registrationCoordinates[i] = prop.m_Fiducials[i];
		}
		m_registrationPointsRadius = prop.m_FiducialRadius;
		m_numImagesToAverage = prop.m_NumImagesAverage;
		size_t nFocusPoints = prop.m_QuickPoints.size();
		m_FocusPoints.resize(nFocusPoints);
		for (size_t i = 0; i < nFocusPoints; i++)
		{
			m_FocusPoints[i] = prop.m_QuickPoints[i];
		}

	}
	else
	{
		registrationCoordinates.resize(4);
		registrationCoordinates[0] = CPoint(768, 575);
		registrationCoordinates[1] = CPoint(1818, 589);
		registrationCoordinates[2] = CPoint(1810, 1375);
		registrationCoordinates[3] = CPoint(763, 1370);
	}
}

COperatorConsole3View::~COperatorConsole3View()
{
	if (m_pBitmapInfo)
	{
		delete m_pBitmapInfo;
		m_pBitmapInfo = nullptr;
	}
	if (m_pBitmapInfoSaved)
	{
		delete m_pBitmapInfoSaved;
		m_pBitmapInfoSaved = nullptr;
	}
}

BOOL COperatorConsole3View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

void COperatorConsole3View::DisplayText(CDC* pDC, CPoint pt, CString text)
{
	CPoint ptScreen = ImageToDisplay(pt);
	CSize szText;
	GetTextExtentPoint32(pDC->GetSafeHdc(), text.GetString(), text.GetLength(), &szText);
	int x = ptScreen.x - (szText.cx / 2);
	int y = ptScreen.y - (szText.cy / 2);
	pDC->TextOut(x, y, text.GetString());
}

void COperatorConsole3View::DisplayFocusResult(CDC* pDC, CPoint pt, double value)
{
	CString sValue;
	sValue.Format("%.1f",value);
	DisplayText(pDC, pt, sValue);
}

int COperatorConsole3View::DisplayToImage(int num)
{
	if (m_shrinkDisplay)
	{
		num = num * m_zoomDivision;
	}
	else if (m_magnifyDisplay)
	{
		num = num / m_zoomMultiplier;
	}
	return num;
}

int COperatorConsole3View::ImageToDisplay(int num)
{
	if (m_shrinkDisplay)
	{
		num = num / m_zoomDivision;
	}
	else if (m_magnifyDisplay)
	{
		num = num * m_zoomMultiplier;
	}
	return num;
}

CPoint COperatorConsole3View::DisplayToImage(CPoint pt)
{
	CPoint returnPt;
	if (m_shrinkDisplay)
	{
		returnPt.x = pt.x * m_zoomDivision;
		returnPt.y = pt.y * m_zoomDivision;
	}
	else if (m_magnifyDisplay)
	{
		returnPt.x = pt.x / m_zoomMultiplier;
		returnPt.y = pt.y / m_zoomMultiplier;
	}
	else
		returnPt = pt;
	return returnPt;
}

CPoint COperatorConsole3View::ImageToDisplay(CPoint pt)
{
	CPoint returnPt;
	if (m_shrinkDisplay)
	{
		returnPt.x = pt.x / m_zoomDivision;
		returnPt.y = pt.y / m_zoomDivision;
	}
	else if (m_magnifyDisplay)
	{
		returnPt.x = pt.x * m_zoomMultiplier;
		returnPt.y = pt.y * m_zoomMultiplier;
	}
	else
		returnPt = pt;
	return returnPt;
}

void COperatorConsole3View::DrawRegistrationPoint(CDC *pDC, CPoint pt)
{
	int radius = ImageToDisplay(m_registrationPointsRadius);
	CPoint ptScreen = ImageToDisplay(pt);

	pDC->Ellipse(ptScreen.x - radius, ptScreen.y - radius, ptScreen.x + radius, ptScreen.y + radius);
	pDC->MoveTo(ptScreen.x, ptScreen.y - radius);
	pDC->LineTo(ptScreen.x, ptScreen.y + radius);
	pDC->MoveTo(ptScreen.x - radius, ptScreen.y);
	pDC->LineTo(ptScreen.x + radius, ptScreen.y);
}

// COperatorConsole3View drawing
void COperatorConsole3View::OnDraw(CDC* pDC)
{
	CScrollView::OnPrepareDC(pDC, NULL);

	int width = m_imageInfo.width_;
	int height = m_imageInfo.height_;
	if (m_shrinkDisplay)
	{
		width = m_imageInfo.width_ / m_zoomDivision;
		height = m_imageInfo.height_ / m_zoomDivision;
	}
	else if (m_magnifyDisplay)
	{
		width = m_imageInfo.width_ * m_zoomMultiplier;
		height = m_imageInfo.height_ * m_zoomMultiplier;
	}
	// Setting the StretchBltMode to COLORONCOLOR eliminates the horrible dithering for grayscale images.
	SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);
	if (!m_image8Data.empty() && (m_pBitmapInfo != nullptr))
	{
		ReadLock lock(m_pictureLock);
		::StretchDIBits(pDC->GetSafeHdc(), 0, 0, width, height, 0, 0, m_imageInfo.width_, m_imageInfo.height_, LPVOID(&m_image8Data[0]), m_pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
	}
	if (m_DrawRegistrationMarks)
	{
		HGDIOBJ originalPen = pDC->SelectObject(GetStockObject(DC_PEN));
		HGDIOBJ originalBrush = pDC->SelectObject(GetStockObject(HOLLOW_BRUSH));
		pDC->SetDCPenColor(RGB(0, 255, 0));
		for(auto i = registrationCoordinates.begin(); i != registrationCoordinates.end(); i++)
			DrawRegistrationPoint(pDC, *i);
		pDC->SelectObject(originalPen);
		pDC->SelectObject(originalBrush);
	}
	if (m_DrawFocusTestResults && !m_outputQuickMTF50.empty())
	{
		double total = 0.0;
		for (auto i = m_outputQuickMTF50.begin(); i != m_outputQuickMTF50.end(); i++)
			total += i->mtf50;
		double average = total / double(m_outputQuickMTF50.size());

		for(int i=0; i< m_FocusPoints.size(); i++)
			DisplayFocusResult(pDC, CPoint(int(m_outputQuickMTF50[i].x), int(m_outputQuickMTF50[i].y)), m_outputQuickMTF50[i].mtf50);
		int x = (registrationCoordinates[1].x - registrationCoordinates[0].x) / 2 + registrationCoordinates[0].x;
		int y = (registrationCoordinates[3].y - registrationCoordinates[0].y) / 2 + registrationCoordinates[0].y;
		DisplayFocusResult(pDC, CPoint(x, y), average);
	}
	else if (m_DrawFullChartMTF50 && !m_outputFullChartMTF50.empty())
	{
		for (auto i = m_outputFullChartMTF50.begin(); i != m_outputFullChartMTF50.end(); i++)
		{
			DisplayFocusResult(pDC, CPoint(int(i->x), int(i->y)), i->mtf50);
		}
	}
	else if (m_DrawFullChartSNR && !m_outputFullChartSNR.empty())
	{
		size_t index = 0;
		for (auto i = m_outputFullChartSNR.begin(); i != m_outputFullChartSNR.end(); i++)
		{
			if (index < m_GreyBoxes.size())
			{
				DisplayFocusResult(pDC, CPoint(int(i->x), int(i->y)), double(index));
				DisplayFocusResult(pDC, CPoint(int(i->x), int(i->y) + 10), i->meanIntensity);
				DisplayFocusResult(pDC, CPoint(int(i->x), int(i->y) + 20), i->RMSNoise);
				DisplayFocusResult(pDC, CPoint(int(i->x), int(i->y) + 30), i->SignalToNoise);
				index++;
			}
		}
	}
	else if (m_ShowInstallDiffusionFilter)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		CPoint pt(rcClient.Width() / 2, rcClient.Height() / 2);
		pt = DisplayToImage(pt);
		pt += GetDeviceScrollPosition();
		DisplayText(pDC, pt, "Please install diffusion filter");
	}
	else if (m_ShowRemoveDiffusionFilter)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		CPoint pt(rcClient.Width() / 2, rcClient.Height() / 2);
		pt = DisplayToImage(pt);
		pt += GetDeviceScrollPosition();
		DisplayText(pDC, pt, "Please Remove diffusion filter");
	}
	else if (m_ShowTestingDiffusionFilter)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		CPoint pt(rcClient.Width() / 2, rcClient.Height() / 2);
		pt = DisplayToImage(pt);
		pt += GetDeviceScrollPosition();
		DisplayText(pDC, pt, "Testing image with diffusion filter - please wait");
	}
	else if (m_programState == eStateReportResults)
	{
		CRect rc;
		GetClientRect(&rc);
		CBrush brBkGnd;
		brBkGnd.CreateSolidBrush(RGB(252, 255, 255));
		CPoint pt(m_imageInfo.width_ / 2 - 200, m_imageInfo.height_ / 2);
		CPoint ptScreen = ImageToDisplay(pt);
		pDC->TextOut(ptScreen.x, ptScreen.y, "Testing for this imager is done - please open and replace with next");
	}
}

bool COperatorConsole3View::OpenJSON(CFile& file, CString filename)
{
	bool success = false;
	SYSTEMTIME st;
	GetLocalTime(&st);
	CString sFilename;
	sFilename.Format("%%USERPROFILE%%\\Documents\\EyeLock\\%s_%s_%d-%02d-%02d-%02d-%02d-%02d.json",
		m_CameraID.c_str(), filename.GetString(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	char path[MAX_PATH];
	ExpandEnvironmentStringsA(sFilename.GetString(), path, MAX_PATH);
	if (file.Open(path, CFile::modeCreate | CFile::modeWrite))
	{
		success = true;
	}
	return success;
}

void COperatorConsole3View::InitPictureData(int width, int height)
{
	m_imageInfo.width_ = width;
	m_imageInfo.height_ = height;
	const auto numPixels = m_imageInfo.width_ * m_imageInfo.height_;
	if (m_pBitmapInfo)
	{
		delete m_pBitmapInfo;
		m_pBitmapInfo = nullptr;
	}
	m_sizeBitmapInfo = sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256);
	m_pBitmapInfo = (BITMAPINFO*)new uint8_t[m_sizeBitmapInfo];
	memset(m_pBitmapInfo, 0, m_sizeBitmapInfo);
	m_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo->bmiHeader.biWidth = m_imageInfo.width_;
	m_pBitmapInfo->bmiHeader.biHeight = -m_imageInfo.height_;
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
	m_loggerFactory.getDefaultInstance()->AddTarget(new Logging::CLogTargetFile("OperatorConsole.log", Logging::LOG_LEVEL_DEBUG));
	LOGMSG_DEBUG("Startup");
	CSize sizeTotal;
	// TODO: calculate the total size of this view
	m_imageInfo.width_ = m_vidCapture.GetWidth();
	m_imageInfo.height_ = m_vidCapture.GetHeight();
	sizeTotal.cx = m_vidCapture.GetWidth();
	sizeTotal.cy = m_vidCapture.GetHeight();
	SetScrollSizes(MM_TEXT, sizeTotal);
	InitPictureData(sizeTotal.cx, sizeTotal.cy);
	m_OperatorConsoleLockEngaged = false;
	m_OperatorConsoleSwitchPressed = false;
	m_CameraRunning = false;
	m_ProgramStateThreadRunning = false;
	m_TestingThreadRunning = false;
	m_ThreadShutdown = false;
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
		m_pTestingThread = AfxBeginThread(ThreadProc, this); // <<== START THE THREAD
#if defined(USE_MATLAB)
		m_pProgramStateThread = AfxBeginThread(AnalyzeFrameThreadProc, this);
#endif
	}
	else
	{
	}

}

OperatorConsoleState COperatorConsole3View::SetProgramState(OperatorConsoleState nextState)
{
	if (nextState != m_programState)
	{
		LOGMSG_DEBUG("Changing state from " + std::to_string(m_programState) + " to next state " + std::to_string(nextState));
		m_programState = nextState;
		m_stateChange = true;
		std::string text(m_CameraID);
		text += " - ";
		switch (nextState)
		{
		case eStateWaitForCameraLock: text += "Waiting for camera lock to engage"; break;
		case eStateFocusingCamera: text += "Please focus camera and press button when ready"; break;
		case eStateTesting1Camera: text += "Running Signal-To-Noise Ratio test"; break;
		case eStateTesting2Camera: text += "Building average image and running full MTF50 test"; break;
		case eStateWaitForDiffusionFilter: text += "Please install diffusion filter"; break;
		case eStateTestingDiffusionFilter: text += "Building average image and running diffusion test"; break;
		case eStateWaitRemoveDiffusionFilter: text += "Please remove diffusion filter"; break;
		case eStateReportResults: text += "Done with tests - please open/close magnetic lock and test new camera"; break;
		default: text += "Unknown state: " + std::to_string(nextState); break;
		}
		AfxGetMainWnd()->SetWindowTextA(text.c_str());
	}
	else
	{
		m_stateChange = false;
	}
	return nextState;
}

#define VENDER_CMD_NONE 21
#define VENDER_CMD_POLL_MAG_STATUS 22
#define VENDER_CMD_POLL_BUTTON_STATUS 23

bool COperatorConsole3View::MagLockEngaged()
{
#if defined(USE_SOFTWARE_FOR_STATION)
	return m_bMagStripeEngaged;
#else
	static bool lastState = false;
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
	if (m_bOverrideMagLock)
	{
		magLockEngaged = m_bMagStripeEngaged;
	}
	if (magLockEngaged != lastState)
	{
		LOGMSG_DEBUG("MagLock state changed to " + std::to_string(magLockEngaged));
		lastState = magLockEngaged;
	}
	return magLockEngaged;
#endif
}

bool COperatorConsole3View::SwitchPressed()
{
#if defined(USE_SOFTWARE_FOR_STATION)
	return m_bCX3ButonPressed;
#else
	static bool lastPressed = false;
	bool switchPressed = false;
	if (m_vidCapture.CameraControl() && m_vidCapture.VideoProcAmp() && MagLockEngaged())
	{
		long value = -1, flags = 0;
		HRESULT hr = m_vidCapture.CameraControl()->Get(CameraControl_Focus, &value, &flags);
		hr = m_vidCapture.CameraControl()->Set(CameraControl_Focus, VENDER_CMD_POLL_BUTTON_STATUS, flags);
		hr = m_vidCapture.VideoProcAmp()->Get(VideoProcAmp_Hue, &value, &flags);
		switchPressed = (1 == value);
		hr = m_vidCapture.CameraControl()->Get(CameraControl_Focus, &value, &flags);
		hr = m_vidCapture.CameraControl()->Set(CameraControl_Focus, VENDER_CMD_NONE, flags);
	}
	if (lastPressed != switchPressed)
	{
		LOGMSG_DEBUG("SwitchPressed changed to " + std::to_string(switchPressed));
		lastPressed = switchPressed;
	}
	return switchPressed;
#endif
}

OperatorConsoleState COperatorConsole3View::HandleWaitForCameraLock(bool newState)
{
	if (newState)
	{
		LOGMSG_DEBUG("Enter newState");
		m_DrawFocusTestResults = false;
		m_DrawFullChartMTF50 = false;
		m_DrawFullChartSNR = false;
		m_RunFocusTest = false;
		m_bRunTestQuickMTF50 = false;
		m_bRunTestFullChartMTF50 = false;
		m_bRunTestFullChartSNR = false;
	}
	if (MagLockEngaged())
	{
		return SetProgramState(eStateFocusingCamera);
	}
	else
	{
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
		return SetProgramState(eStateWaitForCameraLock);
	}
}

OperatorConsoleState COperatorConsole3View::HandleFocusingCamera(bool newState)
{
	if (newState)
	{
		LOGMSG_DEBUG("Enter newState");
		if (prop.Load())
		{
			size_t nFiducials = prop.m_Fiducials.size();
			registrationCoordinates.resize(nFiducials);
			for (size_t i = 0; i < nFiducials; i++)
			{
				registrationCoordinates[i] = prop.m_Fiducials[i];
			}
			m_numImagesToAverage = prop.m_NumImagesAverage;
			size_t nFocusPoints = prop.m_QuickPoints.size();
			m_FocusPoints.resize(nFocusPoints);
			for (size_t i = 0; i < nFocusPoints; i++)
			{
				m_FocusPoints[i] = prop.m_QuickPoints[i];
			}
		}
		m_DrawFocusTestResults = true;
		m_DrawFullChartMTF50 = false;
		m_DrawFullChartSNR = false;
		m_RunFocusTest = true;
		m_bRunTestQuickMTF50 = true;
		m_bFullChartSNRDone = false;
		m_CameraRunning = true;
		HRESULT hr = m_vidCapture.StartVideoCapture();
		if (SUCCEEDED(hr))
		{
			m_vidCapture.ReadCameraInfo(m_CameraInfo);
			m_CameraID = m_CameraInfo.ToString();
			InitPictureData(m_vidCapture.GetWidth(), m_vidCapture.GetHeight());
			PostMessage(MSG_SET_CAMERA_INFO, 0, reinterpret_cast<LPARAM>(&m_CameraInfo));
		}
		else
		{
			LOGMSG_ERROR("Failure to start video capture " + std::to_string(hr));
			LOGMSG_ERROR("returning eStateWaitForCameraLock");
			return SetProgramState(eStateWaitForCameraLock);
		}
	}
	CRect rcWhiteSquare(1188,582,1317,733);
	if (MagLockEngaged())
	{
		if (m_CameraRunning)
		{
			CameraImageInfo imageInfo;
			if (GetFrame(m_image8Data, m_image16Data, imageInfo))
			{
				if ((imageInfo.width_ != m_imageInfo.width_) || (imageInfo.height_ != m_imageInfo.height_))
				{
					InitPictureData(imageInfo.width_, imageInfo.height_);
					m_imageInfo = imageInfo;
				}
#if defined(DRAW_MAIN_THREAD)
				Invalidate(FALSE);
#else
				CDC* pDC = GetDC();
				OnDraw(pDC);
				ReleaseDC(pDC);
#endif
				if (!m_ActiveTestRunning)
				{
					m_ActiveTestRunning = true;
					//memcpy(&m_image8DataTesting[0], &m_image8Data[0], m_image8Data.size()); // Copy to backup which will be tested
					LOGMSG_DEBUG("Copying m_image8Data to m_image8DataTesting for focus testing");
					ReadLock lock(m_pictureLock);
					if (!m_image8DataTesting.empty() && (m_image8DataTesting.size() != m_image8Data.size()))
						m_image8DataTesting.resize(m_image8Data.size());
					if (!m_image8DataTesting.empty())
						memcpy(&m_image8DataTesting[0], &m_image8Data[0], m_image8Data.size()); // Copy to backup which will be tested
					LOGMSG_DEBUG("Setting TestEvent for focus testing");
					m_RunFocusTest = true;
					m_DrawFocusTestResults = true;
					m_MatlabImageTestReadyEvent.SetEvent();
				}
				CheckIfSaveFrames(m_image8Data, m_image16Data);
			}
			if (SwitchPressed())
			{
				return SetProgramState(eStateTesting1Camera);
			}
			return SetProgramState(eStateFocusingCamera);
		}
		else
		{
			return SetProgramState(eStateWaitForCameraLock);
		}
	}
	else
	{
		return SetProgramState(eStateWaitForCameraLock);
	}
}

void COperatorConsole3View::CheckIfSaveFrames(std::vector<uint8_t>& image8, std::vector<uint16_t>& image16)
{
	if (m_SaveEveryFrame8 && (m_SaveFrameCount < m_MaxSaveFrames))
	{
		CString filename;
		filename.Format("%s\\%sFrame-%04d.bmp", m_PictureSavingFolder.GetBuffer(), m_PictureBaseName.GetBuffer(), m_FrameNumber);
		SaveImage8ToFile(filename, image8);
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
		CStringW filename;
		CStringW folder = UTF8toUTF16(m_PictureSavingFolder);
		CStringW baseName = UTF8toUTF16(m_PictureBaseName);
		filename.Format(L"%s\\%sFrame-%04d.tif", folder.GetBuffer(), baseName.GetBuffer(), m_FrameNumber);
		SaveImage16ToFile(filename, image16);
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

bool COperatorConsole3View::GetFrame(std::vector<uint8_t>& image8Data, std::vector<uint16_t>& image16Data, CameraImageInfo& imageInfo)
{
	bool success = false;
	CRect rcWhiteSquare(1188, 582, 1317, 733);
	{
		WriteLock lock(m_pictureLock);
		ULONGLONG start = GetTickCount64();
		HRESULT hr = m_vidCapture.GetCameraFrame(image8Data, image16Data, imageInfo); // This will load the bitmap with the current frame
		ULONGLONG end = GetTickCount64();
		char buffer[64];
		ULONGLONG diff = end - start;
		int ms = int(diff);
		double fps = 1000.0 / double(ms);
		char buff[256];
		sprintf_s(buff, sizeof(buff), "%d ms or %f fps\n", ms, fps);
		OutputDebugString(buff);
		m_FrameNumber++;
		success = SUCCEEDED(hr);
	}
	return success;
}

std::wstring COperatorConsole3View::GetImagePath(const wchar_t* testName)
{
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	CStringW path;
	path.Format(L"%%USERPROFILE%%\\Documents\\Eyelock\\%hs_%ls_%d-%02d-%02d-%02d-%02d-%02d.tif",
		m_CameraID.c_str(), testName,tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond);
	return std::wstring(path.GetString());
}

OperatorConsoleState COperatorConsole3View::HandleTesting1Camera(bool newState)
{
	if (MagLockEngaged())
	{
		// Get Video Frame
		bool frameValid = GetFrame(m_image8Data, m_image16Data, m_imageInfo);
		if (frameValid)
		{
			if (newState)
			{
				LOGMSG_DEBUG("Enter newState");
				m_DrawFocusTestResults = false;
				m_DrawFullChartMTF50 = false;
				m_DrawFullChartSNR = false;
				m_RunFocusTest = false;
				m_bRunTestQuickMTF50 = false;
				m_bFullChartSNRDone = false;
			}
#if defined(DRAW_MAIN_THREAD)
			Invalidate(FALSE);
#else
			// Draw video frame
			CDC* pDC = GetDC();
			OnDraw(pDC);
			ReleaseDC(pDC);
#endif
			CheckIfSaveFrames(m_image8Data, m_image16DataTesting);

			if (newState)
			{
				// We have frame - must test it
				m_bRunTestQuickMTF50 = false;
				m_DrawFocusTestResults = false;
				m_DrawFullChartMTF50 = false;
				m_RunFocusTest = false;

				m_bRunTestFullChartSNR = true;
				m_bFullChartSNRDone = false;
				LOGMSG_DEBUG("Setting TestEvent");
				std::wstring path = GetImagePath(L"FullChartSNR");
				m_image16DataTesting = m_image16Data;
				SaveImage16ToFile(path.c_str(), m_image16DataTesting);
				m_MatlabImageTestReadyEvent.SetEvent(); // go ahead with the first test - just once
			}
			else
			{
				LOGMSG_DEBUG("Waiting on FullChartSNR to be done");
				if (m_bFullChartSNRDone) // if test is done, then advance to next step
				{
					LOGMSG_DEBUG("FullChartSNR is done, so advance state");
					return SetProgramState(eStateTesting2Camera);
				}
				else if (!m_ActiveTestRunning)
				{
					m_bRunTestQuickMTF50 = false;
					m_DrawFocusTestResults = false;
					m_DrawFullChartMTF50 = false;
					m_RunFocusTest = false;

					m_bRunTestFullChartSNR = true;
					m_bFullChartSNRDone = false;
					LOGMSG_DEBUG("Retesting from failure - Setting TestEvent");
					std::wstring path = GetImagePath(L"FullChartSNR");
					m_image16DataTesting = m_image16Data;
					SaveImage16ToFile(path.c_str(), m_image16DataTesting);
					m_MatlabImageTestReadyEvent.SetEvent(); // go ahead with the first test - just once
				}
			}
		}
		return SetProgramState(eStateTesting1Camera); // waiting for test to be done
	}
	else
	{
		return SetProgramState(eStateWaitForCameraLock); // back to beginning
	}
}

OperatorConsoleState COperatorConsole3View::HandleTesting2Camera(bool newState)
{
	static bool makingAverage = false;
	static int imageCount = 0;
	if (MagLockEngaged())
	{
		if (newState)
		{
			LOGMSG_DEBUG("Enter newState");
			m_DrawFocusTestResults = false;
			m_DrawFullChartMTF50 = false;
			m_DrawFullChartSNR = false;
			m_RunFocusTest = false;
			m_bRunTestQuickMTF50 = false;
			m_bFullChartSNRDone = false;
			makingAverage = true;
			imageCount = 0;
			// clear image average buffer
			size_t size = m_image32Average.size();
			size_t sizeElement = sizeof(m_image32Average[0]);
			size_t sizeMem = size * sizeElement;
			memset(&m_image32Average[0], 0, sizeMem);
		}
		bool frameValid = GetFrame(m_image8Data, m_image16Data, m_imageInfo);
		if (frameValid)
		{
#if defined(DRAW_MAIN_THREAD)
			Invalidate(FALSE);
#else
			CDC* pDC = GetDC();
			OnDraw(pDC);
			ReleaseDC(pDC);
#endif
			CheckIfSaveFrames(m_image8Data, m_image16Data);

			if (makingAverage)
			{
				imageCount++;
				if (imageCount < m_numImagesToAverage)
				{
					LOGMSG_DEBUG("Averaging in fame " + std::to_string(m_FrameNumber));
					// Add in the current image to the total
					for (size_t i = 0; i < m_image16Data.size(); i++)
					{
						m_image32Average[i] += m_image16Data[i];
					}
				}
				else
				{
					LOGMSG_DEBUG("Build average frame, now test");
					// Compute the average
					makingAverage = false;
					for (size_t i = 0; i < m_image32Average.size(); i++)
					{
						m_image16DataTesting[i] = m_image32Average[i] / imageCount;
					}
					m_bRunTestFullChartMTF50 = true;
					m_bFullChartMTF50Done = false;
					LOGMSG_DEBUG("Setting m_bRunTestFullChartMTF50 to run and setting event");
					std::wstring path = GetImagePath(L"FullChartMTF50");
					SaveImage16ToFile(path.c_str(), m_image16DataTesting);
					m_MatlabImageTestReadyEvent.SetEvent();
				}
			}
			else if (m_bFullChartMTF50Done)
			{
				return SetProgramState(eStateWaitForDiffusionFilter);
			}
			else if (!m_ActiveTestRunning)
			{
				m_bRunTestFullChartMTF50 = true;
				m_bFullChartMTF50Done = false;
				LOGMSG_DEBUG("Setting m_bRunTestFullChartMTF50 to run and setting event");
				m_MatlabImageTestReadyEvent.SetEvent();
			}
		}
		return SetProgramState(eStateTesting2Camera);
	}
	return SetProgramState(eStateWaitForCameraLock); // back to beginning
}

OperatorConsoleState COperatorConsole3View::HandleWaitForDiffusionFilter(bool newState)
{
	static int countDiffusionFilterDetected = 0;
	if (MagLockEngaged())
	{
		if (newState)
		{
			m_DrawFocusTestResults = false;
			m_DrawFullChartSNR = false;
			m_DrawFullChartMTF50 = false;
			m_ShowInstallDiffusionFilter = true;
			m_ShowRemoveDiffusionFilter = false;
			m_ShowTestingDiffusionFilter = false;
			countDiffusionFilterDetected = 0;
		}
		bool frameValid = GetFrame(m_image8Data, m_image16Data, m_imageInfo);
		if (frameValid)
		{
			CheckIfSaveFrames(m_image8Data, m_image16Data);
			Invalidate(FALSE);
			int totalPixelValue = 0;
			for (auto i = m_image8Data.begin(); i != m_image8Data.end(); i++)
				totalPixelValue += *i;
			int averagePixel = totalPixelValue / int(m_image8Data.size());
			if ((averagePixel > prop.m_MinDiffusionValue) && (averagePixel < prop.m_MaxDiffusionValue))
			{
				countDiffusionFilterDetected++;
				if (countDiffusionFilterDetected >= prop.m_CountDiffusionFilterValid)
					return SetProgramState(eStateTestingDiffusionFilter);
			}
		}
		return SetProgramState(eStateWaitForDiffusionFilter);
	}
	else
		return SetProgramState(eStateWaitForCameraLock); // back to beginning
}

OperatorConsoleState COperatorConsole3View::HandleTestWithDiffusionFilter(bool newState)
{
	static int imageCount = 0;
	static bool makingAverage = false;
	if (MagLockEngaged())
	{
		if (newState)
		{
			m_DrawFocusTestResults = false;
			m_DrawFullChartSNR = false;
			m_DrawFullChartMTF50 = false;
			m_ShowInstallDiffusionFilter = false;
			m_ShowRemoveDiffusionFilter = false;
			m_ShowTestingDiffusionFilter = true;
			m_bRunDiffusionTest = false;
			m_bDiffusionTestDone = false;
			imageCount = 0;
			memset(&m_image32Average[0], 0, sizeof(m_image32Average[0]) * m_image32Average.size());
			makingAverage = true;
		}
		bool frameValid = GetFrame(m_image8Data, m_image16Data, m_imageInfo);
		if (frameValid)
		{
			CheckIfSaveFrames(m_image8Data, m_image16Data);
			Invalidate(FALSE);
			// build average image
			if (imageCount < prop.m_NumDiffusionImagesToAverage)
			{
				LOGMSG_DEBUG("Averaging in fame " + std::to_string(m_FrameNumber));
				// Add in the current image to the total
				for (size_t i = 0; i < m_image16Data.size(); i++)
				{
					m_image32Average[i] += m_image16Data[i];
				}
				imageCount++;
				return SetProgramState(eStateTestingDiffusionFilter); // return back here for more averaging
			}
			else
			{
				LOGMSG_DEBUG("Build average frame, now test");
				// Compute the average
				makingAverage = false;
				for (size_t i = 0; i < m_image32Average.size(); i++)
				{
					m_image16DataTesting[i] = m_image32Average[i] / imageCount;
				}
				LOGMSG_DEBUG("Saving Diffusion Test average image");
				std::wstring path = GetImagePath(L"DiffusionTest");
				SaveImage16ToFile(path.c_str(), m_image16DataTesting);
				return SetProgramState(eStateWaitRemoveDiffusionFilter);
			}
		}
		return SetProgramState(eStateTestingDiffusionFilter); // return back here for more averaging
	}
	else
		return SetProgramState(eStateWaitForCameraLock); // back to beginning
}

OperatorConsoleState COperatorConsole3View::HandleWaitForRemovalOfDiffusionFilter(bool newState)
{
	static int countDiffusionFilterNotDetected = 0;
	if (MagLockEngaged())
	{
		if (newState)
		{
			m_DrawFocusTestResults = false;
			m_DrawFullChartSNR = false;
			m_DrawFullChartMTF50 = false;
			m_ShowInstallDiffusionFilter = false;
			m_ShowRemoveDiffusionFilter = true;
			m_ShowTestingDiffusionFilter = false;
			countDiffusionFilterNotDetected = 0;
		}
		bool frameValid = GetFrame(m_image8Data, m_image16Data, m_imageInfo);
		if (frameValid)
		{
			CheckIfSaveFrames(m_image8Data, m_image16Data);
			Invalidate(FALSE);
			int totalPixelValue = 0;
			for (auto i = m_image8Data.begin(); i != m_image8Data.end(); i++)
				totalPixelValue += *i;
			int averagePixel = totalPixelValue / int(m_image8Data.size());
			if (averagePixel > prop.m_MaxDiffusionValue)
			{
				countDiffusionFilterNotDetected++;
				if (countDiffusionFilterNotDetected >= prop.m_CountDiffusionFilterValid)
					return SetProgramState(eStateReportResults);
			}
		}
		return SetProgramState(eStateWaitRemoveDiffusionFilter);
	}
	else
		return SetProgramState(eStateWaitForCameraLock); // back to beginning
}

OperatorConsoleState COperatorConsole3View::HandleReportResults(bool newState)
{
	if (MagLockEngaged())
	{
		if (newState)
		{
			LOGMSG_DEBUG("Enter newState");
			m_DrawFocusTestResults = false;
			m_DrawFullChartMTF50 = false;
			m_DrawFullChartSNR = false;
			m_ShowInstallDiffusionFilter = false;
			m_ShowRemoveDiffusionFilter = false;
			m_ShowTestingDiffusionFilter = false;
			m_RunFocusTest = false;
			m_bRunTestQuickMTF50 = false;
			m_bFullChartSNRDone = false;
		}
		bool frameValid = GetFrame(m_image8Data, m_image16Data, m_imageInfo);
		if (frameValid)
		{
#if defined(DRAW_MAIN_THREAD)
			Invalidate(FALSE);
#else
			CDC* pDC = GetDC();
			if (pDC)
			{
				OnDraw(pDC);
				CRect rc;
				GetClientRect(&rc);
				CBrush brBkGnd;
				brBkGnd.CreateSolidBrush(RGB(252, 255, 255));
				int x = rc.Width() / 2 - 100;
				int y = rc.Height() / 2;
				pDC->TextOut(x, y, "Testing for this imager is done - please open and replace with next");
				ReleaseDC(pDC);
		}
			CheckIfSaveFrames(m_image8Data, m_image16Data);
#endif
		}
		return SetProgramState(eStateReportResults);
	}
	return SetProgramState(eStateWaitForCameraLock); // back to beginning
}

void COperatorConsole3View::WriteString(CFile& file, CString text, bool addComma)
{
	file.Write(text.GetString(), text.GetLength());
	if (addComma)
		file.Write(",", 1);
	file.Write("\n", 1);
}

void COperatorConsole3View::WriteAttrib(CFile& file, std::string name, double value, bool inQuotes, bool addComma)
{
	CString text;
	std::string formatString = "\"%s\" : %.1f";
	if (inQuotes)
		formatString = "\"%s\" : \"%.1f\"";
	text.Format(formatString.c_str(), name.c_str(), value);

	WriteString(file, text, addComma);
}

void COperatorConsole3View::WriteAttrib(CFile& file, std::string name, int value, bool inQuotes, bool addComma)
{
	CString text;
	std::string formatString = "\"%s\" : %d";
	if (inQuotes)
		formatString = "\"%s\" : \"%d\"";
	text.Format(formatString.c_str(), name.c_str(), value);
	WriteString(file, text, addComma);
}

void COperatorConsole3View::WriteAttrib(CFile& file, std::string name, std::string value, bool addComma)
{
	CString text;
	text.Format("\"%s\" : \"%s\"", name.c_str(), value.c_str());
	WriteString(file, text, addComma);
}

UINT __cdecl COperatorConsole3View::AnalyzeFrameThreadProc(LPVOID pParam)
{
	COperatorConsole3View* pThis = (COperatorConsole3View*)pParam;
	LOGMSG_DEBUG("Enter thread - calling matlab Initialize");
	pThis->m_TestingThreadRunning = true;
	HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr) && pThis->m_matlabTestCode.Initialize())
	{
		LOGMSG_DEBUG("Enter");
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		LARGE_INTEGER timeStart, timeEnd;
		HANDLE handles[2];
		handles[0] = pThis->m_ShutdownEvent;
		handles[1] = pThis->m_MatlabImageTestReadyEvent;
		LOGMSG_DEBUG("Running testing loop");
		while (!pThis->m_ThreadShutdown)
		{
			DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			if (WAIT_OBJECT_0 == waitResult)
			{
				pThis->m_ThreadShutdown = true;
			}
			else if ((WAIT_OBJECT_0+1) == waitResult)
			{
				::QueryPerformanceCounter(&timeStart);
				LOGMSG_DEBUG("Received signal to run test");
				pThis->m_ActiveTestRunning = true;
				CFile file;
				if (pThis->m_bRunTestQuickMTF50)
				{
					LOGMSG_DEBUG("Running RunTestQuickMTF50 - turning off m_bQuickMTF50Done");
					pThis->m_bQuickMTF50Done = false;
					AfxGetMainWnd()->SetWindowTextA("Running QuickMTF50");
					if (!pThis->m_image8DataTesting.empty())
						pThis->m_DrawFocusTestResults = pThis->m_matlabTestCode.RunTestQuickMTF50(pThis->m_image8DataTesting, pThis->m_imageInfo.width_, pThis->m_imageInfo.height_, pThis->registrationCoordinates, pThis->m_outputQuickMTF50);
					LOGMSG_DEBUG("After RunTestQuickMTF50 - " + std::to_string(pThis->m_DrawFocusTestResults));
					if (pThis->m_DrawFocusTestResults && !pThis->m_ThreadShutdown)
					{
#if 0
						if (pThis->OpenJSON(file, "FocusTest"))
						{
							pThis->WriteString(file, "{", false);
							pThis->WriteAttrib(file, "CameraID", pThis->m_CameraID, true);
							pThis->WriteAttrib(file, "FrameNum", pThis->m_FrameNumber, true);
							pThis->WriteString(file, "\"MFT50\" : [", false);
							CString text;
							for (auto i = pThis->m_outputQuickMTF50.begin(); i != pThis->m_outputQuickMTF50.end(); i++)
							{
								CString value;
								value.Format("%.1f", i->mtf50);
								if (text.GetLength() > 0)
									text += ",";
								text += value;
							}
							file.Write(text.GetString(), text.GetLength());
							pThis->WriteString(file, "]", false);
							pThis->WriteString(file, "}", false);
						}
#endif
						pThis->m_bQuickMTF50Done = true;
						AfxGetMainWnd()->SetWindowTextA("Success from QuickMTF50");
					}
					else if (!pThis->m_ThreadShutdown)
					{
						AfxGetMainWnd()->SetWindowTextA("Error from QuickMTF50");
					}
					LOGMSG_DEBUG("Turning on m_bQuickMTF50Done");
				}
				else if (pThis->m_bRunTestFullChartMTF50)
				{
					pThis->m_bFullChartMTF50Done = false;
					LOGMSG_DEBUG("Running RunTestFullChartMTF50 - with average");
					AfxGetMainWnd()->SetWindowTextA("Running FullChartMTF50");
					pThis->m_DrawFullChartMTF50 = pThis->m_matlabTestCode.RunTestFullChartMTF50(pThis->m_image16DataTesting, pThis->m_imageInfo.width_, pThis->m_imageInfo.height_, pThis->registrationCoordinates, pThis->m_outputFullChartMTF50);
					LOGMSG_DEBUG("After RunTestFullChartMTF50 - " + std::to_string(pThis->m_DrawFullChartMTF50));
					if (pThis->m_DrawFullChartMTF50 && !pThis->m_ThreadShutdown)
					{
						if (pThis->OpenJSON(file, "FullChartMTF50"))
						{
							pThis->WriteString(file, "{", false);
							pThis->WriteAttrib(file, "CameraID", pThis->m_CameraID);
							pThis->WriteAttrib(file, "FrameNum", pThis->m_FrameNumber);
							pThis->WriteString(file, "\"Tests\" : [", false);
							int index = 1;
							int lastIndex = int(pThis->m_outputFullChartMTF50.size());
							for (auto i = pThis->m_outputFullChartMTF50.begin(); i != pThis->m_outputFullChartMTF50.end(); i++, index++)
							{
								pThis->WriteString(file, "{", false);
								pThis->WriteAttrib(file, "TestID", index, true, true);
								pThis->WriteAttrib(file, "Edge", int(i->edge), true, true);
								pThis->WriteAttrib(file, "X", int(i->x), true, true);
								pThis->WriteAttrib(file, "Y", int(i->y), true, true);
								pThis->WriteAttrib(file, "MFTF50", i->mtf50, true, false);
								pThis->WriteString(file, "}", index < lastIndex); // add comma if not last item
							}
							pThis->WriteString(file, "]", false);
							pThis->WriteString(file, "}", false);
							pThis->m_bRunTestFullChartMTF50 = false;
						}
						pThis->m_bFullChartMTF50Done = true;
						pThis->m_bRunTestFullChartMTF50 = false;
						AfxGetMainWnd()->SetWindowTextA("Success from FullChartMTF50");
						LOGMSG_DEBUG("Turning off m_bRunTestFullChartMTF50 and turning on m_bFullChartMTF50Done");
					}
					else if (!pThis->m_ThreadShutdown)
					{
						pThis->m_bFullChartMTF50Done = false;
						pThis->m_bRunTestFullChartMTF50 = true;
						AfxGetMainWnd()->SetWindowTextA("Failure from FullChartMTF50");
						LOGMSG_DEBUG("Error running RunTestFullChartMTF50 - will try to do again");
					}
				}
				else if (pThis->m_bRunTestFullChartSNR)
				{
					pThis->m_bFullChartSNRDone = false;
					LOGMSG_DEBUG("Running RunTestFullChartSNR");
					AfxGetMainWnd()->SetWindowTextA("Running FullChartSNR");
					pThis->m_DrawFullChartSNR = pThis->m_matlabTestCode.RunTestFullChartSNR(pThis->m_image16DataTesting, pThis->m_imageInfo.width_, pThis->m_imageInfo.height_, pThis->registrationCoordinates, pThis->m_outputFullChartSNR);
					LOGMSG_DEBUG("After RunTestFullChartSNR - " + std::to_string(pThis->m_DrawFullChartSNR));
					if (pThis->m_DrawFullChartSNR && !pThis->m_ThreadShutdown)
					{
						if (pThis->OpenJSON(file, "FullChartSNR"))
						{
							pThis->WriteString(file, "{", false);
							pThis->WriteAttrib(file, "CameraID", pThis->m_CameraID);
							pThis->WriteAttrib(file, "FrameNum", pThis->m_FrameNumber);
							pThis->WriteString(file, "\"Tests\" : [", false);
							int index = 1;
							int lastIndex = int(pThis->m_outputFullChartSNR.size());
							for (auto i = pThis->m_outputFullChartSNR.begin(); i != pThis->m_outputFullChartSNR.end(); i++, index++)
							{
								pThis->WriteString(file, "{", false); // no comma on start of struct item
								pThis->WriteAttrib(file, "TestID", index, true, true);
								pThis->WriteAttrib(file, "GrayBox", int(i->grayBox), true, true);
								pThis->WriteAttrib(file, "X", int(i->x), true, true);
								pThis->WriteAttrib(file, "Y", int(i->y), true, true);
								pThis->WriteAttrib(file, "meanIntensity", i->meanIntensity, true, true);
								pThis->WriteAttrib(file, "RMSNoise", i->RMSNoise, true, true);
								pThis->WriteAttrib(file, "SignalToNoise", i->SignalToNoise, true, false); // last item in struct, no comma
								pThis->WriteString(file, "}", index < lastIndex); // add comma if not last item
							}
							pThis->WriteString(file, "]", false);
							pThis->WriteString(file, "}", false);
						}
						pThis->m_bRunTestFullChartSNR = false;
						pThis->m_bFullChartSNRDone = true;
						AfxGetMainWnd()->SetWindowTextA("Success from FullChartSNR");
						LOGMSG_DEBUG("Turning off m_bRunTestFullChartSNR and turning on m_bRunTestFullChartSNR");
					}
					else if (!pThis->m_ThreadShutdown)
					{
						pThis->m_bRunTestFullChartSNR = true;
						pThis->m_bFullChartSNRDone = false;
						AfxGetMainWnd()->SetWindowTextA("Failure from FullChartSNR");
						LOGMSG_DEBUG("Error running RunTestFullChartSNR");
					}
				}
				else if (pThis->m_bRunDiffusionTest)
				{
					pThis->m_bDiffusionTestDone = true;
					LOGMSG_DEBUG("RunTestDiffusion");
				}
				else
				{
					LOGMSG_DEBUG("No test set to runrun");
				}
				pThis->m_ActiveTestRunning = false;
				::QueryPerformanceCounter(&timeEnd);
				LONGLONG diff = (timeEnd.QuadPart - timeStart.QuadPart) / (Frequency.QuadPart / 1000);
				LOGMSG_DEBUG("Time to run - " + std::to_string(diff) + "ms");
				if (diff < 100)
					Sleep(1);
			}
		}
		LOGMSG_DEBUG("Shutdown matlab code");
		pThis->m_matlabTestCode.Shutdown();
	}
	pThis->m_TestingThreadRunning = false;
	LOGMSG_DEBUG("Leave");
	return 0;
}

UINT __cdecl COperatorConsole3View::ThreadProc(LPVOID pParam)
{
	COperatorConsole3View* pThis = (COperatorConsole3View*)pParam;
	pThis->m_ProgramStateThreadRunning = true;
	LOGMSG_DEBUG("Program State thread running");
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	DWORD dwFrameTime = 1000 / 13; // 66 ms
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	LARGE_INTEGER timeStart, timeEnd;
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
				nextState = pThis->HandleWaitForCameraLock(pThis->m_stateChange);
				break;
			case eStateFocusingCamera:
				nextState = pThis->HandleFocusingCamera(pThis->m_stateChange);
				break;
			case eStateTesting1Camera:
				nextState = pThis->HandleTesting1Camera(pThis->m_stateChange);
				break;
			case eStateTesting2Camera:
				nextState = pThis->HandleTesting2Camera(pThis->m_stateChange);
				break;
			case eStateWaitForDiffusionFilter:
				nextState = pThis->HandleWaitForDiffusionFilter(pThis->m_stateChange);
				break;
			case eStateTestingDiffusionFilter:
				nextState = pThis->HandleTestWithDiffusionFilter(pThis->m_stateChange);
				break;
			case eStateWaitRemoveDiffusionFilter:
				nextState = pThis->HandleWaitForRemovalOfDiffusionFilter(pThis->m_stateChange);
				break;
			case eStateReportResults:
				nextState = pThis->HandleReportResults(pThis->m_stateChange);
				break;
			}
		}
		QueryPerformanceCounter(&timeEnd);
		LONGLONG diff = (timeEnd.QuadPart - timeStart.QuadPart) / (Frequency.QuadPart / 1000);
		int nSleepTime = dwFrameTime - int(diff);
		if ((nSleepTime < 0) || (nSleepTime > 1000))
			nSleepTime = 1;
		//char buffer[MAX_PATH];
		//sprintf_s(buffer, sizeof(buffer), "Frame took %lldms, sleeping for %dms", diff, nSleepTime);
		//LOGMSG_DEBUG(buffer);
		Sleep(nSleepTime);
	}
	CoUninitialize();
	pThis->m_ProgramStateThreadRunning = false;
	LOGMSG_DEBUG("Program State Thread shutting down");
	return 0;   // thread completed successfully
}

void COperatorConsole3View::SaveImage16ToFile(const wchar_t *pathname, const std::vector<uint16_t>& image)
{
	// Create factory
	wchar_t path[MAX_PATH];
	ExpandEnvironmentStringsW(pathname, path, MAX_PATH);
	IWICImagingFactoryPtr sp_factory { nullptr };
	HRESULT hr = sp_factory.CreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER);
	if (SUCCEEDED(hr))
	{
		// Create stream
		IWICStreamPtr sp_stream{ nullptr };
		hr = sp_factory->CreateStream(&sp_stream);
		if (SUCCEEDED(hr))
		{
			hr = sp_stream->InitializeFromFilename(path, GENERIC_WRITE);
			if (SUCCEEDED(hr))
			{
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
								hr = sp_frame->SetSize(m_imageInfo.width_, m_imageInfo.height_);
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
								UINT stride = (m_imageInfo.width_ * 16 + 7) / 8;
								UINT bufferSize = m_imageInfo.height_ * stride;
								hr = sp_frame->WritePixels(m_imageInfo.height_, stride, bufferSize, LPBYTE(&image[0]));
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
}

bool COperatorConsole3View::LoadImage8ToFile(const char* filename)
{
	bool success = false;
	if (m_pBitmapInfoSaved)
	{
		delete m_pBitmapInfoSaved;
		m_pBitmapInfoSaved = nullptr;
	}
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

void COperatorConsole3View::SaveImage8ToFile(const char* filename, std::vector<uint8_t>& image8)
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
				UINT cbSize = UINT(image8.size());
				file.Write((LPVOID)&image8[0], cbSize);
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
	m_bOverrideMagLock = m_OperatorConsoleLockEngaged;
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
	LOGMSG_DEBUG("Get threads to shut down");
	m_ThreadShutdown = true;
	m_ShutdownEvent.SetEvent();
	WaitForSingleObject(m_pTestingThread->m_hThread, 1000);
#if defined(USE_MATLAB)
	WaitForSingleObject(m_pProgramStateThread->m_hThread, 1000);
#endif
	LOGMSG_DEBUG("Threads shut down, continue shutting down...");
	m_loggerFactory.Close();
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
		m_PictureSavingFolder = FileDlg.GetFolderPath();
		CString filename = FileDlg.GetPathName();
		SaveImage8ToFile(filename, m_image8Data);
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
		CPoint ptImage = DisplayToImage(point);
		CPoint ptScroll = GetDeviceScrollPosition();
		ptImage += ptScroll;
		bool outsideImage = true;
		if ((ptImage.y >= 0) && (ptImage.y < m_imageInfo.height_))
		{
			if ((ptImage.x >= 0) && (ptImage.x < m_imageInfo.width_))
			{
				int offset = ptImage.y * m_imageInfo.width_ + ptImage.x;
				color = m_image8Data[offset];
				outsideImage = false;
			}
		}
		text.Format("Color %d at %d,%d - highest pixel value in white square = %d", color, ptImage.x, ptImage.y, m_maxPixelValueInSquare);
		SetStatusBarText(text);

		if (MK_LBUTTON & nFlags)
		{
			CPoint ptClient = point + ptScroll;
			// Left Button Down
			if (-1 == m_regPtMoving)
			{
				// We are, possibly, starting a move
				int n = 0;
				for (auto i = registrationCoordinates.begin(); i != registrationCoordinates.end(); i++, n++)
				{
					CRect rc(i->x - m_registrationPointsRadius, i->y - m_registrationPointsRadius, i->x + m_registrationPointsRadius, i->y + m_registrationPointsRadius);
					if (rc.PtInRect(ptImage))
					{
						m_regPtMoving = n;
						m_ptLastMovePt = ptImage;
						break;
					}
				}
			}
			else
			{
				// Ok, we ARE doing a registration point move...
				CPoint offset = ptImage - m_ptLastMovePt;
				registrationCoordinates[m_regPtMoving].Offset(offset);
				m_ptLastMovePt = ptImage;
			}
		}
		else
		{
			// Left Button Up
			if (-1 != m_regPtMoving)
			{
				for (size_t i = 0; i < registrationCoordinates.size(); i++)
				{
					prop.m_Fiducials[i] = registrationCoordinates[i];
				}
				prop.Save();
				m_regPtMoving = -1;
			}
		}
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
	sizeTotal.cx = m_imageInfo.width_ / 4;
	sizeTotal.cy = m_imageInfo.height_ / 4;
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
	sizeTotal.cx = m_imageInfo.width_/2;
	sizeTotal.cy = m_imageInfo.height_/2;
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
	sizeTotal.cx = m_imageInfo.width_;
	sizeTotal.cy = m_imageInfo.height_;
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
	sizeTotal.cx = m_imageInfo.width_*2;
	sizeTotal.cy = m_imageInfo.height_*2;
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
	sizeTotal.cx = m_imageInfo.width_*3;
	sizeTotal.cy = m_imageInfo.height_*3;
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
	sizeTotal.cx = m_imageInfo.width_*4;
	sizeTotal.cy = m_imageInfo.height_*4;
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
		m_PictureSavingFolder = FileDlg.GetFolderPath();
		CStringW filename = UTF8toUTF16(FileDlg.GetPathName());
		SaveImage16ToFile(filename, m_image16Data);
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
