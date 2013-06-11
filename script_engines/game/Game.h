// Game.h : main header file for the GAME application
//

#if !defined(AFX_GAME_H__F7C577E7_39DF_11D2_879C_0080AD509054__INCLUDED_)
#define AFX_GAME_H__F7C577E7_39DF_11D2_879C_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CApp:
// See Game.cpp for the implementation of this class
//


class CApp : public CWinApp
{
public:
	CApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Types
public:
   struct TConfig {
      CString sIncludePaths;
      BOOL    bDebug;
      BOOL    bFullScreen;
   } config;

// Implementation
public:
   void UpdateFrame();
   UINT GetMouseState() const { return m_mousestate; };
   void SetMouseState( UINT state ) { m_mousestate = state; };

// Attributes
public:
	CDirectDraw m_dd;
   CController game;
   CWnd m_hwnd;
   CDebug m_debug;
protected:
	void SetEnv();
private:
   UINT m_mousestate;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAME_H__F7C577E7_39DF_11D2_879C_0080AD509054__INCLUDED_)
