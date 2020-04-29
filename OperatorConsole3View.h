
// OperatorConsole3View.h : interface of the COperatorConsole3View class
//

#pragma once

#include <vector>
#include <shared_mutex>
#include <Windows.h>
#include <comdef.h>
#include <comip.h>
#include <comutil.h>
#include <wincodec.h>
#include "MatlabTestCode.h"

typedef std::shared_mutex Lock;
typedef std::unique_lock< Lock >  WriteLock;
typedef std::shared_lock< Lock >  ReadLock;

#include "VideoCapture.h"

typedef enum
{
	eStateWaitForCameraLock,
	eStateFocusingCamera,
	eStateTestingCamera,
	eStateReportResults
} OperatorConsoleState;

class COperatorConsole3View : public CScrollView
{
protected: // create from serialization only
	COperatorConsole3View() noexcept;
	DECLARE_DYNCREATE(COperatorConsole3View)

// Attributes
public:
	COperatorConsole3Doc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	void DisplayFocusResult(CDC *pDC, int x, int y, int value);
	CStringW UTF8toUTF16(const CStringA& utf8);
	void DrawRegistrationPoint(CDC* pDC, int x, int y);
	void InitPictureData();
	bool LoadImage8ToFile(const char* filename);
	void SaveImage8ToFile(const char* filename);
	void SaveImage16ToFile(const wchar_t* filename);
	virtual void OnInitialUpdate(); // called first time after construct
	static UINT __cdecl ThreadProc(LPVOID pParam);
	static UINT __cdecl AnalyzeFrameThreadProc(LPVOID pParam);
	OperatorConsoleState HandleWaitForCameraLock(bool newState);
	OperatorConsoleState HandleFocusingCamera(bool newState);
	OperatorConsoleState HandleTestingCamera(bool newState);
	OperatorConsoleState HandleReportResults(bool newState);
	CMenu* GetParentMenu();
	void SetStatusBarText(CString text);

// Implementation
public:
	virtual ~COperatorConsole3View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	Lock m_pictureLock;
	VideoCapture m_vidCapture;
	BITMAPINFO* m_pBitmapInfo;
	BITMAPINFO* m_pBitmapInfoSaved;
	std::vector<uint8_t> m_savedImageData;
	DWORD m_sizeBitmapInfo;
	bool m_OperatorConsoleLockEngaged;
	bool m_OperatorConsoleSwitchPressed;
	bool m_DrawingPicture;
	bool m_ThreadRunning;
	bool m_ThreadShutdown;
	bool m_CameraRunning;
	bool m_SaveEveryFrame8;
	bool m_SaveEveryFrame16;
	int m_SaveFrameCount;
	int m_MaxSaveFrames;
	int m_FrameNumber;
	int m_width;
	int m_height;
	int m_zoomDivision;
	int m_zoomMultiplier;
	bool m_shrinkDisplay;
	bool m_magnifyDisplay;
	bool m_DrawRegistrationMarks;
	bool m_DrawFocusTestResults;
	bool m_ActiveTestRunning;
	bool m_RunFocusTest;
	CEvent m_ShutdownEvent;
	CEvent m_FocusImageReadyEvent;
	uint8_t m_maxPixelValueInSquare;
	CString m_PictureSavingFolder;
	CString m_PictureBaseName;
	std::vector<uint8_t> m_image8Data, m_image8DataTesting;
	std::vector<uint16_t> m_image16Data;
	OperatorConsoleState m_programState;
	MatlabTestCode m_matlabTestCode;
	std::vector<CPoint> registrationCoordinates;
	std::vector<int> m_focusTestingResults;
	CameraInfoParser m_CameraInfo;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCameraOperatorConsoleLock();
	afx_msg void OnCameraPressOperatorConsoleButton();
	afx_msg void OnCameraUseTop8Bits();
	afx_msg void OnCameraUseMiddle8Bits();
	afx_msg void OnCameraUseBottom8Bits();
	afx_msg void OnCameraSaveSingleImageToDisk();
	afx_msg void OnCameraSaveSequenceToDisk();
	afx_msg void OnMouseMove(UINT nFlags,CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnViewDrawRegistrationMarks();
	afx_msg void OnViewZoom14();
	afx_msg void OnViewZoom12();
	afx_msg void OnViewZoom11();
	afx_msg void OnViewZoom21();
	afx_msg void OnViewZoom31();
	afx_msg void OnViewZoom41();
	afx_msg void OnCameraSaveSingle10BitImageToTiffFile();
	afx_msg void OnCameraSaveSequence10BitImagesToTiffFiles();
	afx_msg void OnCameraRunfocustesting();
};

#ifndef _DEBUG  // debug version in OperatorConsole3View.cpp
inline COperatorConsole3Doc* COperatorConsole3View::GetDocument() const
   { return reinterpret_cast<COperatorConsole3Doc*>(m_pDocument); }
#endif

