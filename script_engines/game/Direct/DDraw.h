// DDraw.h: interface for the CDirectDraw class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DDRAW_H__F7C577F3_39DF_11D2_879C_0080AD509054__INCLUDED_)
#define AFX_DDRAW_H__F7C577F3_39DF_11D2_879C_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define DIRECTDRAW_VERSION 0x300
#pragma comment(lib, "ddraw.lib")

#include <mmsystem.h>
#include <ddraw.h>


class CDirectDraw : public CObject  
{
public:
	CDirectDraw();
	virtual ~CDirectDraw();

// Attributes
public:
   LPDIRECTDRAW        m_lpDD;         // DirectDraw object
   LPDIRECTDRAWSURFACE m_lpDDSPrimary; // DirectDraw primary surface
   LPDIRECTDRAWSURFACE m_lpDDSBack;    // DirectDraw backbuffer surface
protected:
   LPDIRECTDRAWCLIPPER m_lpDDClipper;  // DirectDraw Clip object
   LPDIRECTDRAWPALETTE m_lpDDPalette; // DirectDraw palette
   //
   BOOL m_bFullScreen;
   BOOL m_bActive;
   //
   short m_nWidth, m_nHeight, m_nDepth;
	unsigned long m_RedMask,m_GreenMask,m_BlueMask,m_AlphaMask;	
	unsigned long m_Pitch;
   HFONT Font;

// Implementation
public:
	HRESULT GetBlackPalette( PALETTEENTRY Palette[256] );
	HRESULT Init( HWND hwnd );
	HRESULT Close();
	void    PrintInfo();
	//
   void    SetFullScreenMode(BOOL state) { m_bFullScreen = state; };
   void    SetActive(BOOL state) { m_bActive = state; };
   HRESULT SetExclusive( HWND hwnd, BOOL bFullScreen );
	HRESULT SetDisplayMode( short width, short height, short depth );
	//
   HRESULT SetPaletteRaw( BYTE palette[256*3] );
   HRESULT GetPaletteDirect( PALETTEENTRY palette[256] );
   HRESULT SetPaletteDirect( LPDIRECTDRAWPALETTE palette );
   HRESULT SetPaletteILBM(LPCTSTR Filename);
	HRESULT RestorePalette();
	HRESULT FadePalette( const PALETTEENTRY OldPalette[256], PALETTEENTRY NewPalette[256], short Duration );
	//
   LPDIRECTDRAWSURFACE GetPrimarySurface() const { return m_lpDDSPrimary; };
   LPDIRECTDRAWSURFACE GetBackSurface() const { return m_lpDDSBack; };
   LPDIRECTDRAWPALETTE GetPalette() const { return m_lpDDPalette; };
   //
   short GetWidth() const { return m_nWidth; };
   short GetHeight() const { return m_nHeight; };
   short GetDepth() const { return m_nDepth; };
   BOOL  IsFullScreen() const { return m_bFullScreen; };
   BOOL  IsActive() const { return m_bActive; };
   //
	BYTE * Lock();
   HRESULT Unlock();
	HRESULT Flip(HWND hwnd);
	//
   void DirectMsg( LPCTSTR szFormat, ... );
   void DirectError( LPCTSTR szFormat, ... );
	void Print( short x, short y, LPTSTR Text, BYTE r=0, BYTE g=0, BYTE b=0 );

};

#endif // !defined(AFX_DDRAW_H__F7C577F3_39DF_11D2_879C_0080AD509054__INCLUDED_)
