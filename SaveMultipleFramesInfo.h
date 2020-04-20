#pragma once


// SaveMultipleFramesInfo dialog

class SaveMultipleFramesInfo : public CDialogEx
{
	DECLARE_DYNAMIC(SaveMultipleFramesInfo)

public:
	SaveMultipleFramesInfo(CWnd* pParent = nullptr);   // standard constructor
	virtual ~SaveMultipleFramesInfo();
	virtual BOOL OnInitDialog();
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INPUT_SAVE_FRAME_DATA };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString MaxFrameCount;
	CString FilenameLeadingText;
	CString FolderPath;
	CMFCEditBrowseCtrl PathControl;
	CSpinButtonCtrl MaxFramesSpin;
};
