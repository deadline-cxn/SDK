// Game.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Game.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CApp

/////////////////////////////////////////////////////////////////////////////
// CApp construction

CApp::CApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CApp object

CApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CApp initialization

static long FAR PASCAL WindowProc( HWND hWnd, UINT message, 
                                   WPARAM wParam, LPARAM lParam )
{
   CApp *app = (CApp *)AfxGetApp();

   switch( message ) {
   case WM_ACTIVATEAPP:
      app->m_dd.SetActive(wParam);
      break;

   case WM_LBUTTONDOWN:
   case WM_RBUTTONDOWN:
      if( app->GetMouseState()==0 ) app->SetMouseState( message );
      break;

   case WM_SETCURSOR:
      ::SetCursor(NULL);
      return TRUE;

   case WM_PALETTECHANGED:
      if ((HWND)wParam == hWnd)
         break;
      // fall through to WM_QUERYNEWPALETTE
   case WM_QUERYNEWPALETTE:
      // install our palette here
      app->m_dd.RestorePalette();
      //DDReLoadBitmap(lpDDSOne, szBitmap);
      break;

   case WM_CREATE:
      break;

   case WM_KEYDOWN:
      switch( wParam ) {
      case VK_ESCAPE:
      case VK_F12:
         ::PostMessage(hWnd,WM_CLOSE,0,0);
         break;
      }
      break;

   case WM_DESTROY:
      ::PostQuitMessage( 0 );
      break;
   }
   return DefWindowProc(hWnd, message, wParam, lParam);
} /* WindowProc */


BOOL CApp::InitInstance()
{
   const TCHAR szClassName[] = _T("DXGAMECLASS");
   const TCHAR szTitle[] = _T("DierctX Game");
   BOOL ok;
   // Standard initialization
   SetRegistryKey(_T("Local AppWizard-Generated Applications"));
   SetEnv();
   
#ifdef _DEBUG
   if( config.bDebug ) m_debug.CreateConsole();
#endif
   DEBUGLOG(_T("Game v0.0"));
   DEBUGLOG(_T(""));
   DEBUGLOG(_T("Initializing..."));

   // set up and register window class
   m_debug.Print(_T("Creating display..."));
   WNDCLASS wc;
   wc.style = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc = WindowProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = AfxGetInstanceHandle();
   wc.hIcon = ::LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME) );
   wc.hCursor = ::LoadCursor( NULL, IDC_ARROW );
   wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   wc.lpszMenuName = NULL; //MAKEINTRESOURCE(IDR_MAINFRAME);
   wc.lpszClassName = szClassName;
   ::RegisterClass( &wc );

   // Create a window
   m_hwnd.CreateEx( 
      WS_EX_TOOLWINDOW, 
      szClassName, 
      szTitle, 
      WS_POPUP |  WS_BORDER | WS_CAPTION,
      30,
      30,
      m_dd.GetWidth(),
      m_dd.GetHeight(),
      NULL,
      NULL );
   if( !::IsWindow(m_hwnd) ) {
      m_debug.Error(_T("Main window creation failed."));
      return FALSE;
   }
   m_hwnd.ShowWindow( SW_SHOWNORMAL );
   m_hwnd.UpdateWindow();
   m_hwnd.SetFocus();

   // Intialize game...
   ok = game.Init();

   // Message loop
   MSG msg;
   while( TRUE ) {
      if( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
         if( !::GetMessage( &msg, NULL, 0, 0 ) ) return msg.wParam;
         ::TranslateMessage(&msg); 
         ::DispatchMessage(&msg);
      }
      else if( !m_dd.IsFullScreen() || m_dd.IsActive() ) {
         if(ok) UpdateFrame();
      }
      else {
         ::WaitMessage();
      };
   };

   return TRUE;
}


int CApp::ExitInstance() 
{
   DEBUGLOG(_T("Closing..."));
   // Close game stuff   
   game.Close();
   m_debug.DestroyConsole();
   // Done with Direct Draw for now...
   m_dd.Close();
	
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CApp message handlers

void CApp::UpdateFrame()
{
   game.UpdateFrame();
};




/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CApp message handlers

void CApp::SetEnv()
{
   static TCHAR szSection[] = _T("GENERAL");
   CIni ini;
   CString sFilename;
   // First find settings file (in app path)
   sFilename = KfxLocateFile(_T("env.cfg"));
   if( !BfxFileExists(sFilename) ) return;
   // Read settings
   ini.SetIniFilename(sFilename);
   config.bFullScreen = ini.GetBoolean(szSection,_T("FULLSCREEN"),TRUE);
   config.bDebug = ini.GetBoolean(szSection,_T("DEBUG"),FALSE);
   config.sIncludePaths = ini.GetString(szSection,_T("INCLUDE"));
}

