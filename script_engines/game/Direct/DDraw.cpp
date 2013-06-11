// DDraw.cpp: implementation of the CDirectDraw class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DDraw.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectDraw::CDirectDraw()
{
   m_lpDD = NULL;
   m_lpDDSPrimary = NULL;
	m_lpDDPalette = NULL;
	m_lpDDClipper = NULL;
   //
   m_bFullScreen = FALSE;
   m_bActive = TRUE;
   //
   m_nWidth = 320;
   m_nHeight = 240;
   m_nDepth = 8;
	//
   m_Pitch=0;
	m_RedMask = m_GreenMask = m_BlueMask = m_AlphaMask = 0;
   //
   Font = NULL;
}

CDirectDraw::~CDirectDraw()
{
   Close();
}


//////////////////////////////////////////////////////////////////////
// Implementation

HRESULT CDirectDraw::Init( HWND hwnd )
{
   HRESULT ret;

   // Create Direct Draw object
   ret = DirectDrawCreate( NULL, &m_lpDD, NULL );
   if( ret != DD_OK ) {
      DirectError(_T("Unable to create DD object."));
      return ret;
   };

   // Get exclusive mode if requested
   ret = SetExclusive( hwnd, m_bFullScreen );
   if( ret != DD_OK ) return ret;
  
   if(m_bFullScreen) {
      // Set the video mode...
      ret = SetDisplayMode( m_nWidth, m_nHeight, m_nDepth );
      if( ret != DD_OK ) {
         DirectError(_T("Unable to set display mode"));
         return ret;
      };
   };

   DDSURFACEDESC ddsd;
   ddsd.dwSize = sizeof( ddsd );
   if( m_bFullScreen ) {
	   // Sets flippable surface for full screen
	   ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	   ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	   ddsd.dwBackBufferCount = 1;
   }
   else {	
	   ddsd.dwFlags = DDSD_CAPS;
	   ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
   }	

   // Create the primary surface
   ret = m_lpDD->CreateSurface( &ddsd, &m_lpDDSPrimary, NULL );
   if( ret != DD_OK ) {
	   // surface was not created
	   Close();
	   DirectError(_T("Unable to create primary surface."));
	   return ret;
   }

	// Ops, no flippable surface for windowed mode
	// Creates back buffer
	if( !m_bFullScreen ) {
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		ddsd.dwWidth=m_nWidth;
		ddsd.dwHeight=m_nHeight;

		ret = m_lpDD->CreateSurface( &ddsd, &m_lpDDSBack, NULL );
      if( ret != DD_OK ) {
			// Error: Surface was not created
			Close();
			DirectError(_T("Unable to create back buffer"));
			return ret;
		}
		m_lpDD->CreateClipper(0,&m_lpDDClipper,NULL);
		m_lpDDClipper->SetHWnd(0, hwnd);			
		m_lpDDSPrimary->SetClipper(m_lpDDClipper);
		m_lpDDClipper->Release();
	}
	else {
		// Gets BackBuffer for m_bFullScreen
   	DDSCAPS ddscaps;
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		ret = m_lpDDSPrimary->GetAttachedSurface( &ddscaps, &m_lpDDSBack );
      if( ret != DD_OK ) {
			Close();
			DirectError(_T("Unable to get the back buffer"));
			return ret;
		}
	}

	// Gets pixel format for RGB modes
   DDPIXELFORMAT ddpf;
   ddpf.dwSize = sizeof(ddpf);
   m_lpDDSPrimary->GetPixelFormat(&ddpf);

   if (ddpf.dwRGBBitCount != 8) {

      m_RedMask = ddpf.dwRBitMask;
		m_GreenMask = ddpf.dwGBitMask;
		m_BlueMask = ddpf.dwBBitMask;

		// ATTENTION: Here we do not return the Alpha Mask
		// From DDRAW, since what is interesting is the place left in the pixel
		// and ddraw don't return this as the alpha channel since it's not really usable 
		// as alpha values for him
		// Gets the number of alpha bits
		double tmp=pow(2,ddpf.dwRGBBitCount)-1;		// some trouble with integer arithmetic so use heavy tools
		m_AlphaMask = ~(m_RedMask|m_GreenMask|m_BlueMask);
		m_AlphaMask &=(unsigned)tmp;
	}	

	// Gets pitch
	ddsd.dwSize=sizeof(ddsd);		
	m_lpDDSBack->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR/*DDLOCK_NOSYSLOCK*/,NULL);
	m_lpDDSBack->Unlock(NULL);
	m_Pitch=ddsd.lPitch;
	
   // Erase the background 
   DDBLTFX ddbltfx; 
   ::ZeroMemory(&ddbltfx,sizeof(ddbltfx));
   ddbltfx.dwSize = sizeof(ddbltfx); 
   ddbltfx.dwFillColor = RGB(0,0,0);
   ret = m_lpDDSPrimary->Blt( NULL, NULL, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddbltfx );
   ret = m_lpDDSBack->Blt( NULL, NULL, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddbltfx );
        
   // Set black colorkey
   DDCOLORKEY ddck;
   ddck.dwColorSpaceLowValue  = RGB(0,0,0);
   ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
   m_lpDDSPrimary->SetColorKey(DDCKEY_SRCBLT, &ddck);
   m_lpDDSBack->SetColorKey(DDCKEY_SRCBLT, &ddck);

   // Shut up stupid mouse
   if(m_bFullScreen) while( ::ShowCursor(FALSE)>=0 ) ;

	// Finally sets up a font
   Font = ::CreateFont(m_nWidth < 640 ? 24/2 : 48/3, 0, 0, 0, 
                       FW_NORMAL, FALSE, FALSE, FALSE, 
                       ANSI_CHARSET, 
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, 
                       VARIABLE_PITCH,
                       _T("Comic Sans MS"));

	// Sets a default Palette
	if(m_nDepth==8) SetPaletteRaw(NULL);

   // Spew some state
   PrintInfo();

   return DD_OK;
}


HRESULT CDirectDraw::Close()
{
   // Get a normal display first
   if( m_lpDD!=NULL ) if(m_bFullScreen) m_lpDD->RestoreDisplayMode();
   // Say goodbye to primary surface
   if( m_lpDDSPrimary != NULL ) {
      m_lpDDSPrimary->Release();
      m_lpDDSPrimary = NULL;
   };
	// Remove backbuffer
   if(!m_bFullScreen) if(m_lpDDSBack!=NULL) m_lpDDSBack->Release();		
	m_lpDDSBack=NULL;		
	// Bye bye, palette...
   if(m_lpDDPalette!=NULL) {
		m_lpDDPalette->Release();
		m_lpDDPalette=NULL;
	}
	// Remove DirectDraw object
   if( m_lpDD!=NULL ) {
      m_lpDD->Release();
      m_lpDD = NULL;
   };
   // and that nasty font...
   if( Font!=NULL ) ::DeleteObject(Font);
   Font = NULL;
   return DD_OK;
}


void CDirectDraw::PrintInfo(void)
{
   /*LPDIRECTDRAW2 lpDD2;
   DWORD         dwTotal; 
   DWORD         dwFree;  
   ret = m_lpDD->QueryInterface(IID_IDirectDraw2, &lpDD2); 
   if (FAILED(ret)) {
      DirectError(_T("DirectDraw2 not installed!"));
      return;
   };
   DDSCAPS ddsCaps;
   ddsCaps.dwCaps = DDSCAPS_LOCALVIDMEM;
   lpDD2->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree);*/

   DDCAPS driver;
   DDCAPS hel;
   HRESULT ret;
   ::ZeroMemory(&driver,sizeof(driver));
   ::ZeroMemory(&hel,sizeof(hel));
   driver.dwSize = sizeof(driver);
   hel.dwSize = sizeof(hel);
   ret = m_lpDD->GetCaps(&driver,NULL);
   if( ret!=DD_OK ) {
      DirectError(_T("Unable to query DirectDraw capabilities?"));
      return;
   };
   DirectMsg(_T("   Total Video Memory: %d"), driver.dwVidMemTotal );
   DirectMsg(_T("   Screen: %d x %d x %d"), m_nWidth, m_nHeight, m_nDepth );
   DirectMsg(_T("   Fullscreen: %s"), m_bFullScreen ? _T("Yes") : _T("No"));
   DirectMsg(_T("   Using 3D card: %s"), (driver.dwCaps & DDCAPS_3D) ? _T("Yes") : _T("No"));
};


//////////////////////////////////////////////////////////////////////
// Direct Draw functions

HRESULT CDirectDraw::SetDisplayMode(short width, short height, short depth)
{
   ASSERT( m_lpDD );
   return m_lpDD->SetDisplayMode( width, height, depth );
}

HRESULT CDirectDraw::SetExclusive(HWND hwnd, BOOL bFullScreen)
{
	HRESULT ret;
   if(bFullScreen) {
		if( ret = m_lpDD->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_ALLOWMODEX ) != DD_OK ) {
			// not successful
			// however, the application can still run				
         DirectError(_T("Unable to set exclusinve mode. Setting NORMAL!"));
			ret = m_lpDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
		}
	}
	else 
      ret = m_lpDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
   if( ret==DD_OK ) m_bFullScreen = bFullScreen;
   return ret;
}

long CDirectDraw::Flip( HWND hwnd )
{
   ASSERT(::IsWindow(hwnd));
   ASSERT(m_lpDDSPrimary);
   ASSERT(m_lpDDSBack);
   HRESULT ret;
   if( m_lpDDSPrimary==NULL ) return E_FAIL;

   if( m_lpDDSPrimary->IsLost() == DDERR_SURFACELOST)
      m_lpDDSPrimary->Restore();

   if( m_bFullScreen ) {
		// Fullscreen
      //
      // First we wait for a vblank to start
      m_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
      // Then it's time to blit...
      while( TRUE ) {
         ret = m_lpDDSPrimary->Flip( NULL, DDFLIP_WAIT );
         if( ret == DD_OK ) break;
         if( ret == DDERR_SURFACELOST ) {
	         ret = m_lpDDSPrimary->Restore();
	         if( ret != DD_OK ) break;					
         }
   	}
	}
	else {
		// Non fullscreen
      //
      // First we wait for a vblank to start
      m_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
      // Then it's time to blit...
      RECT srcr,dstr;
		POINT pt;
      ::GetClientRect(hwnd,&srcr);
		pt.x=pt.y=0;
      ::ClientToScreen(hwnd,&pt);
		dstr=srcr;
		dstr.left+=pt.x;
		dstr.right+=pt.x;
		dstr.top+=pt.y;
		dstr.bottom+=pt.y;
		ret = m_lpDDSPrimary->Blt(&dstr,m_lpDDSBack,&srcr,DDBLT_WAIT,0);
	};
   return ret;
}

BYTE * CDirectDraw::Lock()
{
	ASSERT( m_lpDDSBack );
	DDSURFACEDESC ddsd;
	HRESULT hr;

	if(m_lpDDSBack==NULL) return NULL;
	
	ddsd.dwSize=sizeof(ddsd);	
	hr=m_lpDDSBack->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR/*|DDLOCK_WRITEONLY|DDLOCK_NOSYSLOCK*/,NULL);
	if(hr!=DD_OK) ddsd.lpSurface=NULL;
	return (BYTE *)ddsd.lpSurface;
}

HRESULT CDirectDraw::Unlock()
{
	ASSERT( m_lpDDSBack );
   if(m_lpDDSBack==NULL) return E_FAIL;
	m_lpDDSBack->Unlock(NULL);
   return DD_OK;
}


//////////////////////////////////////////////////////////////////////
// Direct Palette functions

HRESULT CDirectDraw::SetPaletteRaw( BYTE palette[256*3] )
{
	unsigned i;
	PALETTEENTRY Pal[256];
	HRESULT ret;
	
   ASSERT(m_lpDD);

   if(m_lpDD==NULL) return E_FAIL;

	if(m_lpDDPalette!=NULL) {
		m_lpDDPalette->Release();
		m_lpDDPalette=NULL;
	};
	
	if( palette!=NULL ) {
		if(m_bFullScreen) {
         // Full screen palette
         for( i=0; i<256; i++ ) {
				Pal[i].peRed=palette[(i*3)+0];
				Pal[i].peGreen=palette[(i*3)+1];
				Pal[i].peBlue=palette[(i*3)+2];
				Pal[i].peFlags=PC_NOCOLLAPSE;
			}
		}
		else {
         // Non fullscreen palette has system colors
         for( i=0; i<10; i++ ) {
				Pal[i].peFlags=PC_EXPLICIT;
			}
			// and some custom ones
         for( ; i<246; i++ ) {
				Pal[i].peFlags=PC_NOCOLLAPSE;
				Pal[i].peRed=palette[i*3];
				Pal[i].peGreen=palette[i*3+1];
				Pal[i].peBlue=palette[i*3+2];
			}
			for( ; i<256; i++ ) {
				Pal[i].peFlags=PC_EXPLICIT;
			}
		}
	}
   else {
      // Build a 332 palette as the default.
      for (i=0;i<256;i++) {
         Pal[i].peRed   = (BYTE)(((i >> 5) & 0x07) * 255 / 7);
         Pal[i].peGreen = (BYTE)(((i >> 2) & 0x07) * 255 / 7);
         Pal[i].peBlue  = (BYTE)(((i >> 0) & 0x03) * 255 / 3);
         Pal[i].peFlags = PC_NOCOLLAPSE;;
      }
   };

   // Time to create the palette...
   ret = m_lpDD->CreatePalette( DDPCAPS_8BIT|DDPCAPS_ALLOW256, Pal, &m_lpDDPalette, NULL ); 
   if( ret!=DD_OK ) {
      DEBUGERROR(_T("Palette creation failed!"));
      return ret;
   };
	// And attach it to the surfaces
   if(m_lpDDSBack!=NULL) ret = m_lpDDSBack->SetPalette(m_lpDDPalette);
   if(m_lpDDSPrimary!=NULL) ret = m_lpDDSPrimary->SetPalette(m_lpDDPalette);
   return ret;
}

HRESULT CDirectDraw::SetPaletteDirect(LPDIRECTDRAWPALETTE palette)
{
   if( palette==NULL) return E_INVALIDARG;
   if(m_lpDDPalette!=NULL) {
      m_lpDDPalette->Release();
      m_lpDDPalette=NULL;
   };
   m_lpDDPalette=palette;

   if(m_lpDDSBack!=NULL) m_lpDDSBack->SetPalette(m_lpDDPalette);
   if(m_lpDDSPrimary!=NULL) m_lpDDSPrimary->SetPalette(m_lpDDPalette);
   return DD_OK;
}

HRESULT CDirectDraw::GetPaletteDirect( PALETTEENTRY palette[256] )
// Returns the RGB codes of the palette
{
   ASSERT(m_lpDDPalette);
   ::ZeroMemory(palette,sizeof(palette));
   return m_lpDDPalette->GetEntries(0, 0,256, palette );
};

HRESULT CDirectDraw::SetPaletteILBM(LPCTSTR Filename)
// Loads a complete ILBM picture file and
// extracts the first palette entries
// The palette for the primary and back surface is then set.
{
   CXILBM ilbm;
   DWORD ret;
   BYTE palette[256*3];
   LPBYTE pal;
   ret = ilbm.Load(Filename);
   if( ret!=0 ) return 1;
   pal = ilbm.GetPalettePtr();
   int idx=0;
   for( int i=0; i<256; i++ ) {
      palette[idx+0] = pal[(i*3)+0];
      palette[idx+1] = pal[(i*3)+1];
      palette[idx+2] = pal[(i*3)+2];
      idx += 3;
   };
   return SetPaletteRaw(palette);
};

HRESULT CDirectDraw::GetBlackPalette(PALETTEENTRY Palette[])
// A little helper function to return a black palette
// Since the palette might have several system colors that we
// do not wish to touch, you can use this to get a (partially)
// black palette back - with all system colors intact
{
   HRESULT ret;
   ret = GetPaletteDirect( Palette );
   if( ret!=RET_OK ) return ret;
   for( int i=0; i<256; i++) {
      if( Palette[i].peFlags!=PC_EXPLICIT ) 
         Palette[i].peRed = 
         Palette[i].peGreen = 
         Palette[i].peBlue = 0;
   };
   return DD_OK;
}

HRESULT CDirectDraw::RestorePalette()
{
   return SetPaletteDirect(m_lpDDPalette);
}


//////////////////////////////////////////////////////////////////////
// Misc

HRESULT CDirectDraw::FadePalette( const PALETTEENTRY OldPalette[256], PALETTEENTRY NewPalette[256], short Duration )
{
   ASSERT(m_lpDD);
   ASSERT(m_lpDDPalette);
   HRESULT ret;
   int i, j;
   PALETTEENTRY TempPalette[256];
   float palette[256*3], delta[256*3];
   
   // Calculate scale and delta values
   float step = (float)Duration;
   for( i=0, j=0; i<256; i++ ) {
      // r
      palette[j] = (float)OldPalette[i].peRed;
      delta[j] = (float)(NewPalette[i].peRed - OldPalette[i].peRed) / step;
      j++;
      // g
      palette[j] = (float)OldPalette[i].peGreen;
      delta[j] = (float)(NewPalette[i].peGreen - OldPalette[i].peGreen) / step;
      j++;
      // b
      palette[j] = (float)OldPalette[i].peBlue;
      delta[j] = (float)(NewPalette[i].peBlue - OldPalette[i].peBlue) / step;
      j++;
   };
   // Ok, loop and change the palette
   // once each vertical blank period
   BYTE c;
   while( Duration>=0 ) {
      Duration--;
      for( i=0, j=0; i<256; i++ ) {
         // r
         c = (BYTE)palette[j];
         palette[j] += delta[j];
         TempPalette[i].peRed = c;
         j++;
         // g
         c = (BYTE)palette[j];
         palette[j] += delta[j];
         TempPalette[i].peGreen = c;
         j++;
         // b
         c = (BYTE)palette[j];
         palette[j] += delta[j];
         TempPalette[i].peBlue = c;
         j++;
         // flags
         TempPalette[i].peFlags = OldPalette[i].peFlags;
      };
      // First we wait for a vblank to start
      m_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
      // then we set the palette
      ret = m_lpDDPalette->SetEntries(0, 0,256, TempPalette );
      if( ret!=DD_OK ) return ret;
   };

   // Time to set (force) the new palette
   // This way we can be sure that the colors are really
   // what we wanted them to be...
   ret = m_lpDDPalette->SetEntries(0, 0,256, NewPalette );
   return ret;
}

void CDirectDraw::Print(short x, short y, LPTSTR Text, BYTE r, BYTE g, BYTE b)
// Clumsy on-screen print function
{
	ASSERT( m_lpDDSBack );
   HDC hdc;
	if( m_lpDDSBack->GetDC(&hdc)==DD_OK ) {
      ::SelectObject( hdc,Font );
      ::SetTextColor( hdc, RGB(r,g,b) );
      ::SetBkMode( hdc, TRANSPARENT );
      ::TextOut( hdc, x, y, Text, _tcslen(Text) );
		m_lpDDSBack->ReleaseDC(hdc);
	}
}

void CDirectDraw::DirectMsg(LPCTSTR szFormat, ... )
// Sends debug messages to the debug console
{
   ASSERT(szFormat);
   CString str;
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);
   va_end(args);
   DEBUGLOG(str);
   return;
}

void CDirectDraw::DirectError(LPCTSTR szFormat, ... )
// Sends debug messages to the debug console
{
   ASSERT(szFormat);
   CString str;
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);
   va_end(args);
   DEBUGERROR(str);
   return;
}

