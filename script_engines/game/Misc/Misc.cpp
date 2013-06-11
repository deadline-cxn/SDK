// Misc.cpp: implementation of global support functions.
// All functions must be prefixed "Kfx" !!!
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

CString KfxLocateFile( LPCTSTR FileTitle )
{
   CApp *app = (CApp *)AfxGetApp();
   CString Path;
   // If full path, then test that...
   Path = FileTitle;
   if( Path.Find("\\")>=0 ) {
      if( BfxFileExists(Path) ) return Path;
   };
   // Attempt to locate file in application path
   Path = BfxGetAppPath() + FileTitle;
   if( BfxFileExists(Path) ) return Path;
   Path = BfxGetAppPath() + _T("Images\\") + FileTitle;
   if( BfxFileExists(Path) ) return Path;
   // Try the INCLUDEPATH folders
   CString sInc = app->config.sIncludePaths;
   while( !sInc.IsEmpty() ) {
      CString tok;
      tok = BfxRemoveToken(sInc,_T(";"));
      if( !tok.IsEmpty() ) {
         ADDBACKSLASH(tok);
         Path = tok + FileTitle;
         if( BfxFileExists(Path) ) return Path;
         Path = tok + _T("images\\") + FileTitle;
         if( BfxFileExists(Path) ) return Path;
      };
   };
   // File not found
   return CString();
};

DWORD KfxGetTime()
{
   LARGE_INTEGER t;
   ::QueryPerformanceCounter(&t);
   return t.LowPart;
};

DWORD KfxGetTimerFreq()
{
   LARGE_INTEGER t;
   ::QueryPerformanceFrequency(&t);
   return t.LowPart;
};

BOOL KfxCalcWalk(CPoint &src_pos, CPoint &dst_pos,
                 long velocity,
                 long &steps,
                 float &delta_x, float &delta_y)
{
   if( velocity<=0 ) velocity=100;
   if (abs(src_pos.x-dst_pos.x) > abs(src_pos.y-dst_pos.y))
		steps = abs(src_pos.x-dst_pos.x);
	else
		steps = abs(src_pos.y-dst_pos.y);
   steps = (long)((float)steps * (100.0/(float)velocity));
   delta_x = (dst_pos.x - src_pos.x) / (float)steps;
   delta_y = (dst_pos.y - src_pos.y) / (float)steps;
   return TRUE;
}

long KfxCalcDistance( CPoint &p1, CPoint &p2 )
{
   long r1 = p1.x-p2.x;
   long r2 = p1.y-p2.y;
   return (r1*r1) + (r2*r2);
};
