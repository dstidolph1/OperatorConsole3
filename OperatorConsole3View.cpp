
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
//#include "MatlabMTFLib_1.h"

using namespace Gdiplus;

#pragma comment (lib,"Gdiplus.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
END_MESSAGE_MAP()

// COperatorConsole3View construction/destruction

COperatorConsole3View::COperatorConsole3View() noexcept
{
	// TODO: add construction code here
	m_pBitmapInfo = nullptr;
	m_sizeBitmapInfo = 0;
	m_OperatorConsoleLockEngaged = false;
	m_OperatorConsoleSwitchPressed = false;
	m_CameraRunning = false;
	m_ThreadRunning = false;
	m_ThreadShutdown = false;
	m_DrawingPicture = false;
	m_SaveEveryFrame = false;
	m_DrawRegistrationMarks = false;
	m_zoomDivision = 1;
	m_zoomMultiplier = 1;
	m_shrinkDisplay = false;
	m_magnifyDisplay = false;
	m_programState = eStateWaitForCameraLock;
	m_FrameNumber = 0;
	m_MaxSaveFrames = 10;
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
	::StretchDIBits(pDC->GetSafeHdc(), 0, 0, width, height, 0, 0, m_width, m_height, LPVOID(&m_imageData[0]), m_pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	if (m_DrawRegistrationMarks)
	{
		HGDIOBJ originalPen = pDC->SelectObject(GetStockObject(DC_PEN));
		HGDIOBJ originalBrush = pDC->SelectObject(GetStockObject(HOLLOW_BRUSH));
		pDC->SetDCPenColor(RGB(0, 255, 0));
		DrawRegistrationPoint(pDC, 776, 605);
		DrawRegistrationPoint(pDC, 1826,604);
		DrawRegistrationPoint(pDC, 776, 1396);
		DrawRegistrationPoint(pDC, 1827, 1397);
		pDC->SelectObject(originalPen);
		pDC->SelectObject(originalBrush);
	}
}

void COperatorConsole3View::InitPictureData()
{
	m_width = PICTURE_WIDTH;
	m_height = PICTURE_HEIGHT;
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
	m_pBitmapInfo->bmiHeader.biSizeImage = m_width * m_height;
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
	m_imageData.resize(m_width * m_height);
}

void COperatorConsole3View::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_vidCapture.InitCOM();
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
	m_programState = eStateWaitForCameraLock;
	m_FrameNumber = 0;
	m_MaxSaveFrames = 10;
	HRESULT hr = m_vidCapture.InitCOM();
	if (SUCCEEDED(hr))
	{
		OnCameraUseTop8Bits();
		AfxBeginThread(ThreadProc, this); // <<== START THE THREAD
	}
	else
	{
	}

}

OperatorConsoleState COperatorConsole3View::HandleWaitForCameraLock(bool newState)
{
	if (m_OperatorConsoleLockEngaged)
	{
		return eStateFocusingCamera;
	}
	else
		return eStateWaitForCameraLock;
}

OperatorConsoleState COperatorConsole3View::HandleFocusingCamera(bool newState)
{
	if (m_OperatorConsoleLockEngaged)
	{
		if (m_CameraRunning)
		{

			{
				WriteLock lock(m_pictureLock);
				m_vidCapture.GetCameraFrame(m_imageData, m_over254); // This will load the bitmap with the current frame
				m_FrameNumber++;
			}
			{
				ReadLock lock(m_pictureLock);
				CDC* pDC = GetDC();
				OnDraw(pDC);
				ReleaseDC(pDC);
			}
			if (m_SaveEveryFrame && (m_SaveFrameCount < m_MaxSaveFrames))
			{
				CString filename;
				filename.Format("%s\\%sFrame-%04d.bmp", m_PictureSavingFolder.GetBuffer(), m_PictureBaseName.GetBuffer(), m_FrameNumber);
				SaveImageToFile(filename);
				m_SaveFrameCount++;
				if (m_SaveFrameCount >= m_MaxSaveFrames)
				{
					m_SaveEveryFrame = false;
					CMenu* pMenu = GetParentMenu();
					if (pMenu)
						pMenu->CheckMenuItem(ID_CAMERA_SAVESEQUENCETODISK, MF_UNCHECKED | MF_BYCOMMAND);

				}
			}
			if (m_OperatorConsoleSwitchPressed)
				return eStateTestingCamera;
			return eStateFocusingCamera;
		}
		else
			return eStateWaitForCameraLock;
	}
	else
		return eStateWaitForCameraLock;
}

OperatorConsoleState COperatorConsole3View::HandleTestingCamera(bool newState)
{
	if (m_OperatorConsoleLockEngaged)
	{
		// do test
		return eStateTestingCamera;
	}
	else
		return eStateWaitForCameraLock;
}

OperatorConsoleState COperatorConsole3View::HandleReportResults(bool newState)
{
	if (m_OperatorConsoleLockEngaged)
	{
		return eStateReportResults;
	}
	else
		return eStateWaitForCameraLock;
}

UINT __cdecl COperatorConsole3View::ThreadProc(LPVOID pParam)
{
	COperatorConsole3View* pThis = (COperatorConsole3View*)pParam;
	pThis->m_ThreadRunning = true;
	DWORD dwFrameTime = 1000 / 13; // 66 ms
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	LARGE_INTEGER timeStart, timeEnd;
	bool stateChange = true;
	OperatorConsoleState nextState = pThis->m_programState;
	while (!pThis->m_ThreadShutdown)
	{
		::QueryPerformanceCounter(&timeStart);
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

void COperatorConsole3View::SaveImageToFile(const char* filename)
{
	BITMAPFILEHEADER bmfh = { 0 };
	bmfh.bfType = 0x4d42;  // 'BM'
	int nSizeHdr = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
	bmfh.bfSize = 0;
	//     bmfh.bfSize = sizeof(BITMAPFILEHEADER) + nSizeHdr + m_dwSizeImage;
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
				file.Write((LPVOID)&m_imageData[0], m_imageData.size());
}
			file.Close();
		}
	}
	catch (CException* pe) {
		pe->Delete();
	}

}



// COperatorConsole3View diagnostics

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
	CMenu* pMenu = GetParentMenu();
	if (pMenu)
	{
		if (m_OperatorConsoleLockEngaged)
		{
			pMenu->CheckMenuItem(ID_CAMERA_OPERATORCONSOLELOCK, MF_CHECKED | MF_BYCOMMAND);
			HRESULT hr = m_vidCapture.StartVideoCapture();
			if (SUCCEEDED(hr))
			{
				m_CameraRunning = true;
				m_DrawingPicture = true;
			}
			else
			{
				MessageBox("Error from trying to start video capture", "Error from Camera", MB_OK);
			}
		}
		else
		{
			pMenu->CheckMenuItem(ID_CAMERA_OPERATORCONSOLELOCK, MF_UNCHECKED | MF_BYCOMMAND);
			m_vidCapture.vcStopCaptureVideo();
		}
	}
}

void COperatorConsole3View::OnDestroy()
{
	// TODO: Add your message handler code here and/or call default
	if (m_CameraRunning)
		m_vidCapture.vcStopCaptureVideo();
	m_CameraRunning = false;
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
		m_PictureSavingFolder = FileDlg.GetFolderPath();
		CString filename = FileDlg.GetPathName();
		SaveImageToFile(filename);
	}
}


void COperatorConsole3View::OnCameraSaveSequenceToDisk()
{
	if (!m_SaveEveryFrame)
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
			m_SaveEveryFrame = true;
			CMenu* pMenu = GetParentMenu();
			pMenu->CheckMenuItem(ID_CAMERA_SAVESEQUENCETODISK, MF_CHECKED | MF_BYCOMMAND);
		}
	}
	else
	{
		m_SaveEveryFrame = false;
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
				color = m_imageData[offset];
			}
		}
		text.Format("Color %d at %d,%d - over 254 count = %d", color, point.x, point.y, m_over254);
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
