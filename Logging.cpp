/*
Copyright (c) 2011 Christoph Stoepel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "pch.h"
#include "Logging.h"
#include <shlwapi.h>
#include <fstream>

using namespace Logging;

CLogger CLoggerFactory::m_SingletonInstance;

/////////////////////////////////////////////////////////////////////////////
// Construction/ Deconstruction
/////////////////////////////////////////////////////////////////////////////

CLogger::CLogger()
{
	m_dwLogStart = ::GetTickCount();
}

CLogger::~CLogger()
{
	for (int i=0; i<m_pTargets.GetSize(); i++)
		delete m_pTargets[i];
	m_pTargets.RemoveAll(); 
}

/////////////////////////////////////////////////////////////////////////////
// Method-Implementation
/////////////////////////////////////////////////////////////////////////////

void CLogger::AddTarget(ILogTarget* pTarget)
{
	m_pTargets.Add(pTarget);
}

void CLogger::Log(LOG_LEVEL lvl, LPCTSTR szMsg, LPCTSTR szFile, LPCTSTR szFunction, int nLine)
{
	bool shouldLog = false;
	for (int i=0; i<m_pTargets.GetSize(); i++)
	{
		if (m_pTargets[i]->IsEnabled(lvl))
		{
			shouldLog = true;
			break; // for
		}
	}

	if (shouldLog)
	{
		CString msg;
		msg.Format(_T("%06d [%s] %s:%d - %s"), ::GetTickCount()-m_dwLogStart, szFunction, ::PathFindFileName(szFile), nLine, szMsg);
		
		for (int i=0; i<m_pTargets.GetSize(); i++)
			if (m_pTargets[i]->IsEnabled(lvl))
				m_pTargets[i]->Append(msg);
	}
}

void CLogTargetFile::Append(LPCTSTR szMsg)
{
	std::ofstream outfile;

	outfile.open(filename, std::ios_base::out | std::ios_base::app); // append instead of overwrite
	outfile << szMsg << std::endl;
}
