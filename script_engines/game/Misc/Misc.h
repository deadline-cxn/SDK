#if !defined(AFX_MISC_H__F3D3F542_98D7_11D1_93C1_444553540000__INCLUDED_)
#define AFX_MISC_H__F3D3F542_98D7_11D1_93C1_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// GLOBAL DEFINES

#define DEBUGLOG ((CApp *)AfxGetApp())->m_debug.Print
#define DEBUGERROR ((CApp *)AfxGetApp())->m_debug.Error


/////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS

CString KfxLocateFile( LPCTSTR FileTitle );
DWORD KfxGetTime();
DWORD KfxGetTimerFreq();

BOOL KfxCalcWalk( CPoint &src_pos, CPoint &dst_pos, long velocity, long &steps, float &deta_x, float &delta_y );
long KfxCalcDistance( CPoint &p1, CPoint &p2 );

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISC_H__F3D3F542_98D7_11D1_93C1_444553540000__INCLUDED_)
