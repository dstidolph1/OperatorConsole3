/**
 * CProperties.cpp
 *
 * This class is used to make it easy to store application settings in an INI file.
 * The ini file is automatically called the same as the application name with .ini
 * as extention. 
 * 
 * Use:
 *  To use this class you have to add the next lines to stdafx.h:
 *      #include "Properties.h"
 *      extern CProperties prop;
 *
 *  Then add the next line to the main application CPP file (myapp.cpp for example) under 
 *  the line "CMyApp theApp;"
 *      CProperties prop;
 * 
 *  The properties class reads the ini file when it's object is created. When no ini file
 *  yet exists, it creates it. When the object is destroyed, the destructor makes sure that
 *  the data is stored in the ini file.
 *
 *  Adding data to the ini file is easy. Just add a public member of the type CString, int or
 *  double to the header of CProperties and then add a line like the next 
 *  in the constructor of CProperties.
 *  m_proplist.push_back( t_props("Connection",     "ServerIp",      "127.0.0.1",    &m_serverIp)        );
 *                                    ^                  ^                  ^                 ^
 *                        key name ___|     value name __|    default val __|   data member __|
 *
 * To compile the properties class, add properties.(cpp/h) and inifile.(cpp/h) files to your project.
 * 
 * To access a property from your source code you can type: prop.m_serverIp for example.
 *
 * This class is tested with the Visual C++ 6.0 compiler.
 * 
 * Dependencies: CIniFile class
 *
 * Author:  Dennis Kuppens (www.KuppensAutomation.nl)
 * Date:    27 Nov 2006
 * 
 * Changelog:
 *          1.0 | 27 Nov 2006 | Initial version.
 *
 *
 * Credits:
 *          Thanks to Shane Hill for his IniFile class.
 *
 */

#include "pch.h"
#include "Properties.h"
#include "IniFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProperties::CProperties()
{
    m_Fiducials.resize(4);
    m_proplist.push_back(t_props("Fiducials", "PT1", CPoint(768, 575), &m_Fiducials[0]));
    m_proplist.push_back(t_props("Fiducials", "PT2", CPoint(1818, 589), &m_Fiducials[1]));
    m_proplist.push_back(t_props("Fiducials", "PT3", CPoint(1810, 1375), &m_Fiducials[2]));
    m_proplist.push_back(t_props("Fiducials", "PT4", CPoint(763, 1370), &m_Fiducials[3]));
    m_proplist.push_back(t_props("Settings", "NumAverage", 20, &m_NumImagesAverage));
    m_QuickPoints.resize(5);
    m_proplist.push_back(t_props("Focus", "PT1", CPoint(1201, 515), &m_QuickPoints[0]));
    m_proplist.push_back(t_props("Focus", "PT2", CPoint(815, 880), &m_QuickPoints[1]));
    m_proplist.push_back(t_props("Focus", "PT3", CPoint(1205, 1145), &m_QuickPoints[2]));
    m_proplist.push_back(t_props("Focus", "PT4", CPoint(1700, 950), &m_QuickPoints[3]));
    m_proplist.push_back(t_props("Focus", "PT5", CPoint(1180, 1450), &m_QuickPoints[4]));
    if (!Load())
    {
        Create();
        Load();
    }
}  

CProperties::~CProperties()
{
    Save();
}

bool CProperties::Load()
{
    CIniFile ini;
    ini.SetPath((char*)(LPCTSTR)GetIniFilePath());

    if (ini.ReadFile())
    {
        std::vector<t_props>::iterator it;
        for(it = m_proplist.begin(); it!= m_proplist.end(); it++)
        {
            switch(it->type)
            {
            case T_STRING:
                *((CString*)it->value) = ini.GetValue((LPCTSTR)it->key, (LPCTSTR)it->valname, (LPCTSTR)it->defStr).c_str();
                break;
            case T_INT:
                *((int*)it->value) = ini.GetValueI((LPCTSTR)it->key, (LPCTSTR)it->valname, it->defInt);
                break;
            case T_FLOAT:
                *((double*)it->value) = ini.GetValueF((LPCTSTR)it->key, (LPCTSTR)it->valname, it->defFloat);
                break;
            case T_POINT:
                *((CPoint*)it->value) = ini.GetValuePT((LPCTSTR)it->key, (LPCTSTR)it->valname, it->defPt);
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool CProperties::Create()
{
    CIniFile ini;
    ini.SetPath((char*)(LPCTSTR)GetIniFilePath());
    std::vector<t_props>::iterator it;
    for(it = m_proplist.begin(); it!= m_proplist.end(); it++)
    {
        switch(it->type)
        {
        case T_STRING:
            ini.SetValue((LPCTSTR)it->key, (LPCTSTR)it->valname, (LPCTSTR)it->defStr);
            break;
        case T_INT:
            ini.SetValueI((LPCTSTR)it->key, (LPCTSTR)it->valname, it->defInt);
            break;
        case T_FLOAT:
            ini.SetValueF((LPCTSTR)it->key, (LPCTSTR)it->valname, it->defFloat);
            break;
        }
    }
    return ini.WriteFile();
}


bool CProperties::Save()
{
    bool retval=false;
    CIniFile ini;
    ini.SetPath((char*)(LPCTSTR)GetIniFilePath());

    if (ini.ReadFile())
    {
        std::vector<t_props>::iterator it;
        for(it = m_proplist.begin(); it!= m_proplist.end(); it++)
        {
            switch(it->type)
            {
            case T_STRING:
                ini.SetValue((LPCTSTR)it->key, (LPCTSTR)it->valname, (LPCTSTR)*(CString*)(it->value));
                break;
            case T_INT:
                ini.SetValueI((LPCTSTR)it->key, (LPCTSTR)it->valname, *(int*)(it->value));
                break;
            case T_FLOAT:
                ini.SetValueF((LPCTSTR)it->key, (LPCTSTR)it->valname, *(double*)(it->value));
                break;
            }
        }

        retval = ini.WriteFile();
    }
    
    return retval;
}

/**
 * Returns the program directory
 * The result does NOT contains the trailing '\' slash.
 */
CString CProperties::GetProgramDir()
{
    CString programPath;
	TCHAR szEXEPathname[_MAX_PATH];
	GetModuleFileName(NULL, szEXEPathname, _MAX_PATH);
	CSplitPath sp;
	sp.Split(szEXEPathname);

    programPath=sp.GetDrive();
    programPath+=sp.GetDirectory();

    programPath.TrimRight();

    if(programPath[programPath.GetLength()-1]=='\\')
    {
        programPath.Delete(programPath.GetLength()-1);
    }

    return programPath;
}

/**
 * Returns the full path to the ini file for this application.
 */
CString CProperties::GetIniFilePath()
{
	TCHAR szEXEPathname[_MAX_PATH];
	GetModuleFileName(NULL, szEXEPathname, _MAX_PATH);
	CSplitPath sp;
	sp.Split(szEXEPathname);

	CString iniFileName = GetProgramDir();
    iniFileName+="\\";
	iniFileName+=sp.GetFileName();
    iniFileName+=".ini";

    return iniFileName;
}


CSplitPath::CSplitPath( LPCTSTR lpszPath )
{
    // Initialize our objects
    memset( path_buffer, 0, sizeof( path_buffer ) );
    memset( drive, 0, sizeof( drive ) );
    memset( dir, 0, sizeof( dir ) );
    memset( fname, 0, sizeof( fname ) );
    memset( ext, 0, sizeof( ext ) );

    // If we were given a path, split it
    if ( lpszPath ) {
        Split( lpszPath );
    }
}

BOOL CSplitPath::Split( LPCTSTR lpszPath )
{
    // If we weren't given a path, fail
    if ( lpszPath == NULL ) {
        // Return failure
        return FALSE;
    }

    // Copy the path
    _tcsncpy_s( path_buffer, lpszPath, sizeof( path_buffer ) - 1 );
    // Split the given path
    _tsplitpath_s( path_buffer, drive, dir, fname, ext );

    return TRUE;
}

bool CProperties::GetPoint(CString text, CPoint& pt)
{
    bool success = false;
    int commaPos = text.Find(',');
    if (commaPos > 0)
    {
        CString xString = text.Left(commaPos);
        CString yString = text.Right(text.GetLength() - commaPos);
        pt.x = atoi(xString.GetString());
        pt.y = atoi(yString.GetString());
        success = true;
    }
    return success;
}
