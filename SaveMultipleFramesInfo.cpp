// SaveMultipleFramesInfo.cpp : implementation file
//

#include "pch.h"
#include "OperatorConsole3.h"
#include "SaveMultipleFramesInfo.h"
#include "afxdialogex.h"
#include "resource.h"

// SaveMultipleFramesInfo dialog

IMPLEMENT_DYNAMIC(SaveMultipleFramesInfo, CDialogEx)

SaveMultipleFramesInfo::SaveMultipleFramesInfo(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_INPUT_SAVE_FRAME_DATA, pParent)
	, MaxFrameCount(_T(""))
	, FilenameLeadingText(_T(""))
	, FolderPath(_T(""))
{

}

SaveMultipleFramesInfo::~SaveMultipleFramesInfo()
{
}

BOOL SaveMultipleFramesInfo::OnInitDialog()
{
	BOOL retVal = CDialogEx::OnInitDialog();
	PathControl.EnableBrowseButton(TRUE);
	PathControl.EnableFolderBrowseButton("Select/Create folder to write images to", BIF_NEWDIALOGSTYLE);
	MaxFramesSpin.SetRange(0, 9999);
	MaxFramesSpin.SetBase(10);

	return retVal;
}

void SaveMultipleFramesInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_MAX_FRAME_COUNT, MaxFrameCount);
	DDX_Text(pDX, IDC_EDIT3, FilenameLeadingText);
	DDX_Text(pDX, IDC_MFCEDITBROWSE1, FolderPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, PathControl);
	DDX_Control(pDX, IDC_SPIN1, MaxFramesSpin);
}


BEGIN_MESSAGE_MAP(SaveMultipleFramesInfo, CDialogEx)
END_MESSAGE_MAP()


// SaveMultipleFramesInfo message handlers
