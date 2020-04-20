
// OperatorConsole3View.h : interface of the COperatorConsole3View class
//

#pragma once

#include <vector>
#include <shared_mutex>

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
	void InitPictureData();
	void SaveImageToFile(const char* filename);
	virtual void OnInitialUpdate(); // called first time after construct
	static UINT __cdecl ThreadProc(LPVOID pParam);
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
	DWORD m_sizeBitmapInfo;
	bool m_OperatorConsoleLockEngaged;
	bool m_OperatorConsoleSwitchPressed;
	bool m_DrawingPicture;
	bool m_ThreadRunning;
	bool m_ThreadShutdown;
	bool m_CameraRunning;
	bool m_SaveEveryFrame;
	int m_SaveFrameCount;
	int m_MaxSaveFrames;
	int m_FrameNumber;
	int m_width;
	int m_height;
	bool m_DrawRegistrationMarks;
	CString m_PictureSavingFolder;
	CString m_PictureBaseName;
	std::vector<uint8_t> m_imageData;
	OperatorConsoleState m_programState;
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
};

#ifndef _DEBUG  // debug version in OperatorConsole3View.cpp
inline COperatorConsole3Doc* COperatorConsole3View::GetDocument() const
   { return reinterpret_cast<COperatorConsole3Doc*>(m_pDocument); }
#endif

