
// OperatorConsole3.h : main header file for the OperatorConsole3 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// COperatorConsole3App:
// See OperatorConsole3.cpp for the implementation of this class
//

class COperatorConsole3App : public CWinAppEx
{
public:
	COperatorConsole3App() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern COperatorConsole3App theApp;
