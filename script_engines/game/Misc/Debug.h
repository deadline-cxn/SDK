// Debug.h: interface for the CDebug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUG_H__836BD726_3F8A_11D2_87A7_0080AD509054__INCLUDED_)
#define AFX_DEBUG_H__836BD726_3F8A_11D2_87A7_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDebug : public CObject  
{
public:
	CDebug();
	virtual ~CDebug();

// Attributes:
protected:
   CWnd m_hwnd;
   CEdit m_text;
   CEdit m_input;
   CFont m_Font;
   BOOL m_bConsoleCreated;

// Implementation
public:
	BOOL DestroyConsole();
	BOOL CreateConsole();
   BOOL IsCreated() const { return m_bConsoleCreated; };
	//
   BOOL Print( LPCTSTR szFormat, ... );
	BOOL Error( LPCTSTR szFormat, ... );
   BOOL PrintReplaceLine( LPCTSTR szFormat, ... );
protected:
   BOOL Log( CString &str );


};

#endif // !defined(AFX_DEBUG_H__836BD726_3F8A_11D2_87A7_0080AD509054__INCLUDED_)
