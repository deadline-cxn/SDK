// DSurface.h: interface for the CDirectSurface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSURFACE_H__997BC033_3ACC_11D2_879D_0080AD509054__INCLUDED_)
#define AFX_DSURFACE_H__997BC033_3ACC_11D2_879D_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDirectSurface : public CObject  
{
public:
	CDirectSurface();
	virtual ~CDirectSurface();

// Attributes
public:
   LPDIRECTDRAWSURFACE m_lpDDSurface; // DirectDraw surface
   DDSURFACEDESC       m_ddsd;        // Description of surface
protected:

// Implementation
public:
	BOOL CopyPixels( LPBYTE lpBuffer, LPBYTE lpPalette, long nWidth, long nHeight );
	HRESULT Clear();
	HRESULT Init();
	HRESULT Close();
	//
	HRESULT Create( CDirectDraw *pDd, DWORD width, DWORD height, DWORD caps=DDSCAPS_SYSTEMMEMORY );
	HRESULT CreateReverse( CDirectDraw *lpDd, CDirectSurface *surf );
	HRESULT LoadILBM( CDirectDraw *lpDd, LPCTSTR Filename );
	HRESULT LoadPPM( CDirectDraw *lpDd, LPCTSTR Filename );
	HRESULT LoadBMP( CDirectDraw *lpDd, LPCTSTR Filename, short dx=0, short dy=0 );
	BOOL    IsLoaded(void) const { return m_lpDDSurface!=NULL; };
   BOOL    IsPalettized(void) const { return m_ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8; };
   //
	HRESULT SetPalette( LPDIRECTDRAWPALETTE pal ) { ASSERT(pal); return m_lpDDSurface->SetPalette(pal); };
	HRESULT SetColorKey( COLORREF rgb );
	//   
	DWORD GetWidth() const { return m_ddsd.dwWidth; };
	DWORD GetHeight() const { return m_ddsd.dwHeight; };
	DWORD GetPitch() const { return m_ddsd.lPitch; };
	COLORREF GetPixelColor( long x, long y );
	DWORD GetPixelIndex( long x, long y );
	//
	HRESULT BltFrom( LPDIRECTDRAWSURFACE source, CRect &src_rc, CRect &dst_rc, BOOL bUseColorkey=TRUE );
	HRESULT BltFrom( CDirectSurface *source, CRect &src_rc, CRect &dst_rc, BOOL bUseColorkey=TRUE ) 
	{
	   return BltFrom( source->m_lpDDSurface, src_rc, dst_rc, bUseColorkey );
	};
	HRESULT BltTo( LPDIRECTDRAWSURFACE dst, CRect &src_rc, CRect &dst_rc, BOOL bUseColorkey=TRUE );
	HRESULT BltTo( CDirectSurface *dst, CRect &src_rc, CRect &dst_rc, BOOL bUseColorkey=TRUE ) 
	{
	   return BltTo( dst->m_lpDDSurface, src_rc, dst_rc, bUseColorkey );
	};
protected:
	DWORD ColorMatch( COLORREF rgb );
	HRESULT CopyBitmap( HBITMAP hbm, int x, int y, int dx=0, int dy=0 );

};

#endif // !defined(AFX_DSURFACE_H__997BC033_3ACC_11D2_879D_0080AD509054__INCLUDED_)
