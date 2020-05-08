
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
#include "Logging.h"
#include "MatlabTestCode.h"
#include <memory>

typedef std::shared_mutex Lock;
typedef std::unique_lock< Lock >  WriteLock;
typedef std::shared_lock< Lock >  ReadLock;

#include "VideoCapture.h"

typedef enum
{
	eStateWaitForCameraLock,
	eStateFocusingCamera,
	eStateTesting1Camera,
	eStateTesting2Camera,
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
	bool MagLockEngaged();
	bool SwitchPressed();

protected:
	void CheckIfSaveFrames(std::vector<uint8_t> &image8, std::vector<uint16_t> &image16);
	bool OpenJSON(CFile& file, CString testName);
	void WriteString(CFile& file, CString text, bool addComma = true);
	void WriteAttrib(CFile& file, std::string name, std::string value, bool addComma = true);
	void WriteAttrib(CFile &file, std::string name, double value, bool inQuotes = true, bool addComma = true);
	void WriteAttrib(CFile& file, std::string name, int value, bool inQuotes = true, bool addComma = true);
	bool GetFrame(std::vector<uint8_t>& image8Data, std::vector<uint16_t>& image16Data);
	void DisplayFocusResult(CDC *pDC, int x, int y, double value);
	CStringW UTF8toUTF16(const CStringA& utf8);
	void DrawRegistrationPoint(CDC* pDC, int x, int y);
	void InitPictureData();
	bool LoadImage8ToFile(const char* filename);
	void SaveImage8ToFile(const char* filename, std::vector<uint8_t>& image8);
	void SaveImage16ToFile(const wchar_t* filename, const std::vector<uint16_t> &image);
	virtual void OnInitialUpdate(); // called first time after construct
	static UINT __cdecl ThreadProc(LPVOID pParam);
	static UINT __cdecl AnalyzeFrameThreadProc(LPVOID pParam);
	OperatorConsoleState HandleWaitForCameraLock(bool newState);
	OperatorConsoleState HandleFocusingCamera(bool newState);
	OperatorConsoleState HandleTesting1Camera(bool newState);
	OperatorConsoleState HandleTesting2Camera(bool newState);
	OperatorConsoleState HandleReportResults(bool newState);
	CMenu* GetParentMenu();
	void SetStatusBarText(CString text);
	OperatorConsoleState SetProgramState(OperatorConsoleState state);
	std::wstring GetImagePath(const wchar_t* testName);

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
	std::string m_CameraID;
	Logging::CLoggerFactory m_loggerFactor;
	CWinThread* m_pProgramStateThread;
	CWinThread* m_pTestingThread;
	bool m_OperatorConsoleLockEngaged;
	bool m_OperatorConsoleSwitchPressed;
	bool m_ProgramStateThreadRunning;
	bool m_TestingThreadRunning;
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
	bool m_DrawFullChartMTF50;
	bool m_DrawFullChartSNR;
	bool m_ActiveTestRunning;
	bool m_RunFocusTest;
	bool m_bMagStripeEngaged;
	bool m_bCX3ButonPressed;
	bool m_bRunTestQuickMTF50;
	bool m_bQuickMTF50Done;
	bool m_bRunTestFullChartMTF50;
	bool m_bFullChartMTF50Done;
	bool m_bRunTestFullChartSNR;
	bool m_bFullChartSNRDone;
	CEvent m_ShutdownEvent;
	CEvent m_MatlabImageTestReadyEvent;
	uint8_t m_maxPixelValueInSquare;
	CString m_PictureSavingFolder;
	CString m_PictureBaseName;
	std::vector<uint8_t> m_image8Data, m_image8DataTesting;
	std::vector<uint16_t> m_image16Data, m_image16DataTesting;
	std::vector<uint32_t> m_image32Average;
	OperatorConsoleState m_programState;
	bool m_stateChange;
	MatlabTestCode m_matlabTestCode;
	std::vector<CPoint> registrationCoordinates;
	CameraInfoParser m_CameraInfo;
	std::vector<CPoint> m_GreyBoxes;
	// Output variables
	std::vector<QuickMTF50Data> m_outputQuickMTF50;
	std::vector<FullChartSNRData> m_outputFullChartSNR;
	std::vector<FullChartMTF50Data> m_outputFullChartMTF50;

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

