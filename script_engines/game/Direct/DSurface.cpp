// DSurface.cpp: implementation of the CDirectSurface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSurface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectSurface::CDirectSurface()
{
   m_lpDDSurface = NULL;
    ::ZeroMemory(&m_ddsd, sizeof(m_ddsd));
}

CDirectSurface::~CDirectSurface()
{
   Close();
}

HRESULT CDirectSurface::Init()
{
   return 0;
}

HRESULT CDirectSurface::Close()
{
   if( m_lpDDSurface!=NULL ) m_lpDDSurface->Release();
   m_lpDDSurface = NULL;
   return 0;
}


//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

HRESULT CDirectSurface::Create(CDirectDraw *lpDd, 
                          DWORD width, DWORD height, 
                          DWORD caps/*=DDSCAPS_SYSTEMMEMORY*/)
{
   ASSERT_VALID(lpDd);
   // Remove previous surface
   if( m_lpDDSurface!=NULL ) {
      Close();
      Init();
   };  
   // Create a DirectDrawSurface for this buffer
   HRESULT ret;
   ::ZeroMemory(&m_ddsd, sizeof(m_ddsd));
   m_ddsd.dwSize = sizeof(m_ddsd);
   m_ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   m_ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | caps;
   m_ddsd.dwWidth = width;
   m_ddsd.dwHeight = height;   
   ret = lpDd->m_lpDD->CreateSurface(&m_ddsd, &m_lpDDSurface, NULL);
   // Creation failed!!!
   // Give second chance without the VIDEO MEMORY demand
   if( ret==DDERR_OUTOFVIDEOMEMORY ) {
      m_ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
      m_ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
      ret = lpDd->m_lpDD->CreateSurface(&m_ddsd, &m_lpDDSurface, NULL);
   };
   return ret;
}

HRESULT CDirectSurface::CreateReverse(CDirectDraw *lpDd, CDirectSurface *surf)
{
   ASSERT_VALID(lpDd);
   ASSERT_VALID(surf);
   if( surf==NULL ) return E_INVALIDARG;

    // make sure this surface is restored.
   surf->m_lpDDSurface->Restore();

    // Remove previous surface
   if( m_lpDDSurface!=NULL ) {
      Close();
      Init();
   };  
   // Create a DirectDrawSurface
   HRESULT ret;
   // Copy surface description structure
   m_ddsd = surf->m_ddsd;
   m_ddsd.dwSize = sizeof(m_ddsd);
   m_ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
   ret = lpDd->m_lpDD->CreateSurface(&m_ddsd, &m_lpDDSurface, NULL);
   // Creation failed!!!
   // Give second chance without the VIDEO MEMORY demand
   if( ret==DDERR_OUTOFVIDEOMEMORY ) {
      m_ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
      m_ddsd.ddsCaps.dwCaps |= ~DDSCAPS_SYSTEMMEMORY;
      ret = lpDd->m_lpDD->CreateSurface(&m_ddsd, &m_lpDDSurface, NULL);
   };

   // Get ready to copy bits
   ret = m_lpDDSurface->Lock(NULL, &m_ddsd, 0, NULL);
   if (ret != DD_OK) {
      m_lpDDSurface->Release();
      return ret;
   };
   ret = surf->m_lpDDSurface->Lock(NULL, &surf->m_ddsd, 0, NULL);
   if (ret != DD_OK) {
      surf->m_lpDDSurface->Release();
      return ret;
   };

   // Validate
   if( GetPitch() != surf->GetPitch() ) {
      TRACE0("CreateSurface error: pitch does not match\n");
   };

   // Copy pixels...
   long dwWidth = GetWidth();
   long dwHeight = GetHeight();
   long dwPitch = GetPitch();
   DWORD RGBcount = m_ddsd.ddpfPixelFormat.dwRGBBitCount;
   for (long j = 0; j < dwHeight; j++) {
       // Point to next row in surface
       LPBYTE lpPtr = (LPBYTE)m_ddsd.lpSurface;
       lpPtr += dwPitch * j;
       LPBYTE lpBuffer = (LPBYTE)surf->m_ddsd.lpSurface;
       lpBuffer += surf->m_ddsd.lPitch * j;
       lpBuffer += dwPitch;
       for (long i = 0; i < dwWidth; i++) {
          switch(RGBcount) {
          case 8:
             *lpPtr++ = *--lpBuffer; 
             break;
          case 16:
             *(LPWORD)lpPtr = *(LPWORD)lpBuffer; 
             lpPtr+=2;
             lpBuffer-=2;
             break;
          case 32:
             *(LPDWORD)lpPtr = *(LPDWORD)lpBuffer; 
             lpPtr+=4;
             lpBuffer-=4;
             break;
          };
       };
   };

   // Done
   m_lpDDSurface->Unlock(NULL);
   surf->m_lpDDSurface->Unlock(NULL);

   return ret;
}

HRESULT CDirectSurface::LoadPPM(CDirectDraw *lpDd, LPCTSTR Filename)
{
   return 0;
}

HRESULT CDirectSurface::LoadILBM(CDirectDraw *lpDd, LPCTSTR Filename)
{
   ASSERT_VALID(lpDd);
   ASSERT(AfxIsValidString(Filename));
   CXILBM ilbm;
   DWORD ret;
   HRESULT res;
   Close();
   ret = ilbm.Load(Filename);
   if( ret!=0 ) return E_FAIL;

   // Create surface
   if( m_lpDDSurface==NULL ) {
      res = Create( lpDd, ilbm.GetWidth(), ilbm.GetHeight() );
      if( res!=0 ) return res;
   };

   // Get ready to copy bits
   m_ddsd.dwSize = sizeof(m_ddsd);
   res = m_lpDDSurface->Lock(NULL, &m_ddsd, 0, NULL);
   if (res != DD_OK) {
      return res;
   };

   // Prepare pixels copy...
   LPBYTE lpBuffer = ilbm.GetBitmapPtr();
   LPBYTE lpPal = ilbm.GetPalettePtr();
   if( lpBuffer==NULL ) return E_FAIL;
   long dwWidth = ilbm.GetWidth();
   long dwHeight = ilbm.GetHeight();
   if( !CopyPixels(lpBuffer, lpPal, dwWidth, dwHeight ) ) {
      m_lpDDSurface->Unlock(NULL);
      return E_FAIL;
   };

   // Done
   m_lpDDSurface->Unlock(NULL);
   return DD_OK;
}

HRESULT CDirectSurface::LoadBMP(CDirectDraw *lpDd, LPCTSTR Filename, 
                           short dx, short dy)
{
   ASSERT_VALID(lpDd);
   ASSERT(AfxIsValidString(Filename));
   HBITMAP hbm;
   BITMAP  bm;
   HRESULT ret;

   //  Try to load the bitmap as a resource, if that fails, try it as a file
   hbm = (HBITMAP)::LoadImage(::GetModuleHandle(NULL), Filename, IMAGE_BITMAP, dx, dy, LR_CREATEDIBSECTION);
   if (hbm == NULL) hbm = (HBITMAP)::LoadImage(NULL, Filename, IMAGE_BITMAP, dx, dy, LR_LOADFROMFILE|LR_CREATEDIBSECTION);
   if (hbm == NULL) return ::GetLastError();

   // get size of the bitmap
   ::GetObject(hbm, sizeof(bm), &bm);      // get size of bitmap

   // Create surface
   if( m_lpDDSurface=NULL ) {
      ret = Create( lpDd, bm.bmWidth, bm.bmHeight );
      if( ret!=0 ) return ret;
   };
   // Recreate bitmap as DIB and copy...
   CopyBitmap(hbm, 0, 0, 0, 0);
   // Done
   ::DeleteObject(hbm);
   return DD_OK;
}



//////////////////////////////////////////////////////////////////////
// Palette and pixel helpers

HRESULT CDirectSurface::SetColorKey(COLORREF rgb)
// Set the colourkey for the surface
// Use CLR_INVALID to set the (0,0) pixel color
{
   ASSERT(m_lpDDSurface); 
   DDCOLORKEY ddck;
   ddck.dwColorSpaceLowValue  = ColorMatch(rgb);
   ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
   return m_lpDDSurface->SetColorKey(DDCKEY_SRCBLT, &ddck);
}

DWORD CDirectSurface::ColorMatch(COLORREF rgb)
{
   COLORREF rgbT;
   HDC hdc;
   DWORD dw = CLR_INVALID;
   HRESULT hres;

   // Use GDI SetPixel to color match for us
   if (rgb != CLR_INVALID && m_lpDDSurface->GetDC(&hdc) == DD_OK) {
      rgbT = ::GetPixel(hdc, 0, 0);             // save current pixel value
      ::SetPixel(hdc, 0, 0, rgb);               // set our value
      m_lpDDSurface->ReleaseDC(hdc);
   }

   // Now lock the surface so we can read back the converted color
   m_ddsd.dwSize = sizeof(m_ddsd);
   while ((hres = m_lpDDSurface->Lock(NULL, &m_ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
     ;
   if (hres == DD_OK) {
      dw  = *(DWORD *)m_ddsd.lpSurface;                     // get DWORD
      dw &= (1 << m_ddsd.ddpfPixelFormat.dwRGBBitCount)-1;  // mask it to bpp
      // Unlock it again
      m_lpDDSurface->Unlock(NULL);
   };

   // Now put the color that was there back.
   if (rgb != CLR_INVALID && m_lpDDSurface->GetDC(&hdc) == DD_OK) {
      ::SetPixel(hdc, 0, 0, rgbT);
      m_lpDDSurface->ReleaseDC(hdc);
   };

   return dw;
}

COLORREF CDirectSurface::GetPixelColor(long x, long y)
{
   HDC hdc;
   COLORREF rgb;
   //  use GDI SetPixel to color match for us
   if (m_lpDDSurface->GetDC(&hdc) == DD_OK) {
      rgb = ::GetPixel(hdc, x, y);             // save current pixel value
      m_lpDDSurface->ReleaseDC(hdc);
   }
   return rgb;
}

DWORD CDirectSurface::GetPixelIndex(long x, long y)
{
   DWORD dw = CLR_INVALID;
   HRESULT hres;

   // now lock the surface so we can read back the converted color
   m_ddsd.dwSize = sizeof(m_ddsd);
   while ((hres = m_lpDDSurface->Lock(NULL, &m_ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
     ;
   if (hres == DD_OK) {
      if( !IsPalettized() ) {
         LPBYTE p = (LPBYTE)m_ddsd.lpSurface;
         p += (m_ddsd.lPitch * y) + x;
         dw  = *(DWORD *)p;                                    // get DWORD
         dw &= (1 << m_ddsd.ddpfPixelFormat.dwRGBBitCount)-1;  // mask it to bpp
      }
      else {
         LPBYTE p = (LPBYTE)m_ddsd.lpSurface;
         p += (m_ddsd.lPitch * y) + x;
         dw = (DWORD)*p;
      };
      m_lpDDSurface->Unlock(NULL);
   };
   return dw;
}


//////////////////////////////////////////////////////////////////////
// Blit routines

HRESULT CDirectSurface::BltFrom(LPDIRECTDRAWSURFACE source, 
                                CRect &src_rc, CRect &dst_rc, 
                                BOOL bUseColorkey/*=TRUE*/ )
{
   ASSERT(source);
   ASSERT(m_lpDDSurface);
   
   CApp *app = (CApp *)AfxGetApp();
   DWORD dwTrans = 0;
   HRESULT ret;
TryAndTryAgain:
   if( app->m_dd.IsFullScreen() ) {
      if( bUseColorkey ) dwTrans |= DDBLTFAST_SRCCOLORKEY;
      ret = m_lpDDSurface->BltFast( src_rc.left, src_rc.top, 
                                    source,
                                    &dst_rc, 
                                    dwTrans | DDBLTFAST_WAIT );
   }
   else {
      if( bUseColorkey ) dwTrans |= DDBLT_KEYSRC;
      ret = m_lpDDSurface->Blt( &src_rc, 
                                source, 
                                &dst_rc, 
                                dwTrans | DDBLT_WAIT, NULL );
   };
   if( ret==DDERR_WASSTILLDRAWING ) 
      goto TryAndTryAgain;
   if( ret!=DD_OK ) 
      { DEBUGERROR("Blit failed %08X\n", ret ); };   
   return ret;
}

HRESULT CDirectSurface::BltTo(LPDIRECTDRAWSURFACE dst, 
                              CRect &src_rc, CRect &dst_rc, 
                              BOOL bUseColorkey/*=TRUE*/ )
{
   ASSERT(dst);
   ASSERT(m_lpDDSurface);

   CApp *app = (CApp *)AfxGetApp();
   DWORD dwTrans = 0;
   HRESULT ret;
TryAndTryAgain:
   if( app->m_dd.IsFullScreen() ) {
      if( bUseColorkey ) dwTrans |= DDBLTFAST_SRCCOLORKEY;
      ret = dst->BltFast( dst_rc.left, dst_rc.top, 
                          m_lpDDSurface,
                          &src_rc, 
                          dwTrans );
   }
   else {
      if( bUseColorkey ) dwTrans |= DDBLT_KEYSRC;
      ret = dst->Blt( &dst_rc, 
                      m_lpDDSurface, 
                      &src_rc, 
                      dwTrans, NULL );
   };
   if( ret==DDERR_WASSTILLDRAWING ) 
      goto TryAndTryAgain;
   if( ret!=DD_OK ) 
      { DEBUGERROR("Blit failed %08X\n", ret ); };   
   return ret;
}

HRESULT CDirectSurface::Clear()
// Clears the screen
// Effectively setting all pixels to RGB(0,0,0)
{
   DDBLTFX ddbltfx; 
   ::ZeroMemory(&ddbltfx,sizeof(ddbltfx));
   ddbltfx.dwSize = sizeof(ddbltfx);
   ddbltfx.dwFillColor = RGB(0,0,0);
   return m_lpDDSurface->Blt( NULL, NULL, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddbltfx );
}



//////////////////////////////////////////////////////////////////////
// Various bitmap copying and duplication

HRESULT CDirectSurface::CopyBitmap(HBITMAP hbm, int x, int y, int dx, int dy)
// A helper function that copied a HBITMAP
// converts it into a DIP and renders it to the surface
{
    ASSERT( hbm );
    ASSERT( m_lpDDSurface );
    HDC                 hdcImage;
    HDC                 hdc;
    BITMAP              bm;
    DDSURFACEDESC       ddsd;
    HRESULT             hr;

    if (hbm == NULL || m_lpDDSurface == NULL) return E_INVALIDARG ;

    // make sure this surface is restored.
    m_lpDDSurface->Restore();

    //  select bitmap into a memoryDC so we can use it.
    hdcImage = ::CreateCompatibleDC(NULL);
    if (!hdcImage) TRACE0("createcompatible dc failed\n");
    ::SelectObject(hdcImage, hbm);

    // get size of the bitmap
    ::GetObject(hbm, sizeof(bm), &bm);  // get size of bitmap
    dx = dx == 0 ? bm.bmWidth  : dx;    // use the passed size, unless zero
    dy = dy == 0 ? bm.bmHeight : dy;

    // get size of surface.
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    m_lpDDSurface->GetSurfaceDesc(&ddsd);

    if ((hr = m_lpDDSurface->GetDC(&hdc)) == DD_OK) {
       ::StretchBlt(hdc, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcImage, x, y, dx, dy, SRCCOPY);
        m_lpDDSurface->ReleaseDC(hdc);
    };
    ::DeleteDC(hdcImage);

    return hr;
}

BOOL CDirectSurface::CopyPixels(LPBYTE lpBuffer, LPBYTE lpPalette, 
                                long nWidth, long nHeight)
// Internal function that copied a 8-bit bitmap
// to the Direct Surface memory
// This is needed because the pixelformat of the surface
// can vary.
{
   ASSERT(nWidth>0);
   ASSERT(nHeight>0);
   ASSERT(AfxIsValidAddress(lpBuffer,nWidth*nHeight,FALSE));
   ASSERT(AfxIsValidAddress(lpPalette,256*3,FALSE));
   
   // Prepare copy
   unsigned long m;
   int s;
   int red_shift, red_scale;
   int green_shift, green_scale;
   int blue_shift, blue_scale;

   // Determine the red, green and blue masks' shift and scale.
   if( !IsPalettized() ) {
      for (s = 0, m = m_ddsd.ddpfPixelFormat.dwRBitMask; (m & 1)==0; s++, m >>= 1)
         ;
      red_shift = s;
      red_scale = 255 / (m_ddsd.ddpfPixelFormat.dwRBitMask >> s);
      for (s = 0, m = m_ddsd.ddpfPixelFormat.dwGBitMask; (m & 1)==0; s++, m >>= 1)
         ;
      green_shift = s;
      green_scale = 255 / (m_ddsd.ddpfPixelFormat.dwGBitMask >> s);
      for (s = 0, m = m_ddsd.ddpfPixelFormat.dwBBitMask; (m & 1)==0; s++, m >>= 1)
         ;
      blue_shift = s;
      blue_scale = 255 / (m_ddsd.ddpfPixelFormat.dwBBitMask >> s);

      // Copy pixels
      DWORD RGBcount = m_ddsd.ddpfPixelFormat.dwRGBBitCount;
      for (long j = 0; j < nHeight; j++) {
		   // Point to next row in surface
		   LPBYTE lpPtr = (LPBYTE)m_ddsd.lpSurface;
		   lpPtr += m_ddsd.lPitch * j;
		   for (short i = 0; i < nWidth; i++) {
		      BYTE c;
		      int r, g, b;
		      c = *lpBuffer++;
		      r = lpPalette[(c*3)+0] / red_scale;
		      g = lpPalette[(c*3)+1] / green_scale;
		      b = lpPalette[(c*3)+2] / blue_scale;
            switch(RGBcount) {
            case 8:
               *lpPtr++ = (r << red_shift) | (g << green_shift) | (b << blue_shift);
               break;
            case 16:
               *(LPWORD)lpPtr = (r << red_shift) | (g << green_shift) | (b << blue_shift);
               lpPtr += 2;
               break;
            case 32:
               *(LPDWORD)lpPtr = (r << red_shift) | (g << green_shift) | (b << blue_shift);
               lpPtr += 4;
               break;
            };
         };
      };
   }
   else {
      // Copy pixels
      for (long j = 0; j < nHeight; j++) {
		   // Point to next row in surface
		   LPBYTE lpPtr = (LPBYTE)m_ddsd.lpSurface;
		   lpPtr += m_ddsd.lPitch * j;
		   for (short i = 0; i < nWidth; i++) {
		      *lpPtr++ = *lpBuffer++;
         };
      };
   };
     
   return TRUE;
}
