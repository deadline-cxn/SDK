// Debug.cpp: implementation of the CDebug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogFile.h"
#include "Debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebug::CDebug()
{
   m_bConsoleCreated = FALSE;
}

CDebug::~CDebug()
{
}


static long FAR PASCAL WindowProc( HWND hWnd, UINT message, 
                                   WPARAM wParam, LPARAM lParam )
{
    switch( message ) {      
    case WM_CREATE:
        break;

    case WM_DESTROY:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
} /* WindowProc */



BOOL CDebug::CreateConsole()
{
   const TCHAR szClassName[] = _T("DEBUGCLASS");
   WNDCLASS wc;
   wc.style = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc = WindowProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = AfxGetInstanceHandle();
   wc.hIcon = NULL;
   wc.hCursor = ::LoadCursor( NULL, IDC_ARROW );
   wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
   wc.lpszMenuName = NULL;
   wc.lpszClassName = szClassName;
   ::RegisterClass( &wc );

   CRect rc( 400,20,400+300,20+400);

   // Create a window
   m_hwnd.CreateEx( 
      WS_EX_TOOLWINDOW, 
      szClassName, 
      _T("Debug Console"), 
      WS_POPUP |  WS_BORDER | WS_CAPTION,
      rc,
      NULL,
      NULL );
   if( !::IsWindow(m_hwnd) ) {
      AfxMessageBox(_T("Unable to create debug window!"), MB_OK|MB_SYSTEMMODAL);
      return FALSE;
   }

   // Create the text box
   CRect childrc;
   DWORD flags;
   m_hwnd.GetClientRect(childrc);
   childrc.bottom -= 20;
   flags = WS_CHILD | WS_BORDER |  WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL;
   m_text.Create(flags,childrc,&m_hwnd,NULL);
   m_text.SetReadOnly();
   // Set debug text font
   LOGFONT logfont;
   ::ZeroMemory( &logfont, sizeof(logfont) );
   logfont.lfHeight = 12;
   logfont.lfWeight = FW_THIN;
   _tcscpy( logfont.lfFaceName, _T("Arial") );
   m_Font.CreateFontIndirect( &logfont );
   m_text.SetFont( &m_Font );
   // Create the text edit field
   m_hwnd.GetClientRect(childrc);
   childrc.top = childrc.bottom-20;
   flags = WS_CHILD | WS_BORDER |  WS_VISIBLE;
   m_input.Create(flags,childrc,&m_hwnd,NULL);

   // Display debug window right away...
   m_hwnd.ShowWindow( SW_SHOWNORMAL );
   m_hwnd.UpdateWindow();

   // Done
   m_bConsoleCreated = TRUE;
   return TRUE;
}

BOOL CDebug::DestroyConsole()
{
   if(!m_bConsoleCreated) return FALSE;
   m_hwnd.DestroyWindow();
   m_bConsoleCreated = FALSE;
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Methods

BOOL CDebug::Log(CString &str)
{
   if(!m_bConsoleCreated) {
#ifdef _DEBUG
      str += _T('\n');
      TRACE0(str);
#else
      CLogFile f(BfxGetAppPath()+_T("GAME.LOG"));
      f.AppendText(str);
#endif
      return FALSE; // anyway, this was not right
   };
   if(!::IsWindow(m_hwnd)) return FALSE;
   // Append line
   str += _T("\r\n");
   int charidx = m_text.GetWindowTextLength()-2;
   if( charidx>25000 ) m_text.SetWindowText(_T("..."));
   m_text.SetSel(charidx,charidx,TRUE);
   m_text.ReplaceSel(str,FALSE);
   return TRUE;
};

BOOL CDebug::Print(LPCTSTR szFormat, ... )
{
   ASSERT(szFormat);
   CApp *app = (CApp *)AfxGetApp();
   if( ((CApp*)AfxGetApp)->config.bDebug==FALSE ) return TRUE; // do not want debug msg
   CString str;
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);
   va_end(args);
   return Log(str);
}

BOOL CDebug::Error(LPCTSTR szFormat, ... )
{
   ASSERT(szFormat);
   ::MessageBeep(-1);
   CString str;
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);   
   va_end(args);
   CString tmp(_T("Error:"));
   Log(tmp);
   Log(str);
   return FALSE; // always return FALSE
}

BOOL CDebug::PrintReplaceLine(LPCTSTR szFormat, ... )
{
   ASSERT( szFormat );
   CString str;
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);
   va_end(args);
   if(!((CApp*)AfxGetApp)->config.bDebug) return TRUE; // do not want debug msg
   if(!m_bConsoleCreated) return FALSE;
   if(!::IsWindow(m_hwnd)) return FALSE;
   // Find last line (assumes that an \r\n has been appended)
   // and replace that line
   str += _T("\r\n");
   int curline = m_text.GetLineCount();
   int charidx = m_text.LineIndex(curline-3);
   int linelen = m_text.LineLength(charidx);
   m_text.SetSel(charidx,charidx+linelen+2,TRUE);
   m_text.ReplaceSel(str,FALSE);
   return TRUE;
}

