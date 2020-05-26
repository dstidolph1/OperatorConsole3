// Properties.h: interface for the CProperties class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROPERTIES_H__FC2CE3B6_ADD5_45B9_B814_5630DDECE3D3__INCLUDED_)
#define AFX_PROPERTIES_H__FC2CE3B6_ADD5_45B9_B814_5630DDECE3D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable: 4786)
#pragma warning(disable: 4710)

#include <vector>

// forward declaration
class t_props;

class CProperties  
{
public:
    CString GetIniFilePath();
    CString GetProgramDir();
    bool Load();
	bool Save();
    bool Create();
	CProperties();
	virtual ~CProperties();

    std::vector<CPoint> m_Fiducials;
    int m_NumImagesAverage;
    std::vector<CPoint> m_QuickPoints;
    int m_FiducialRadius;
    int m_MinDiffusionValue;
    int m_MaxDiffusionValue;
    int m_CountDiffusionFilterValid;
    int m_NumDiffusionImagesToAverage;

private:
    bool GetPoint(CString text, CPoint &pt);
    std::vector<t_props> m_proplist;
};

class CSplitPath
{ 
// Construction
public: 
    CSplitPath( LPCTSTR lpszPath = NULL );

// Operations
    BOOL    Split(LPCTSTR lpszPath );
    CString GetPath( void ) { return path_buffer; }
    CString GetDrive( void ) { return drive; }
    CString GetDirectory( void ) { return dir; }
    CString GetFileName( void ) { return fname; }
    CString GetExtension( void ) { return ext; }

// Attributes
protected:
    TCHAR path_buffer[_MAX_PATH];
    TCHAR drive[_MAX_DRIVE];
    TCHAR dir[_MAX_DIR];
    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];
}; 

enum type{T_STRING, T_INT, T_FLOAT, T_POINT};
class t_props
{
public:
    t_props(CString key, CString valname, CString def, CString * str)
    {
        this->key = key;
        this->valname = valname;
        this->defStr = def;
        type = T_STRING;
        value = (void*)str;
    }

    t_props(CString key, CString valname, int def, int * val)
    {
        this->key = key;
        this->valname = valname;
        this->defInt = def;
        type = T_INT;
        value = (void*)val;
    }

    t_props(CString key, CString valname, double def, double * val)
    {
        this->key = key;
        this->valname = valname;
        this->defFloat = def;
        type = T_FLOAT;
        value = (void*)val;
    }

    t_props(CString key, CString valname, CPoint def, CPoint* val)
    {
        this->key = key;
        this->valname = valname;
        this->defPt = def;
        type = T_POINT;
        value = (void*)val;
    }

    CString key;
    CString valname;
    int     type;
    CString defStr;
    int     defInt;
    double  defFloat;
    CPoint  defPt;
    void *  value;
};


#endif // !defined(AFX_PROPERTIES_H__FC2CE3B6_ADD5_45B9_B814_5630DDECE3D3__INCLUDED_)
