// DSound.h: interface for the CDirectSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSOUND_H__F7C577F7_39DF_11D2_879C_0080AD509054__INCLUDED_)
#define AFX_DSOUND_H__F7C577F7_39DF_11D2_879C_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment(lib, "dsound.lib")
#include <dsound.h>


class CDirectSound : public CObject  
{
public:
	CDirectSound();
	virtual ~CDirectSound();

// Types
protected:
#pragma pack(1)
   struct WaveHeader
   {
       BYTE        RIFF[4];          // "RIFF"
       DWORD       dwSize;           // Size of data to follow
       BYTE        WAVE[4];          // "WAVE"
       BYTE        fmt_[4];          // "fmt "
       DWORD       dw16;             // 16
       WORD        wOne_0;           // 1
       WORD        wChnls;           // Number of Channels
       DWORD       dwSRate;          // Sample Rate
       DWORD       BytesPerSec;      // Sample Rate
       WORD        wBlkAlign;        // 1
       WORD        BitsPerSample;    // Sample size
       BYTE        DATA[4];          // "DATA"
       DWORD       dwDSize;          // Number of Samples
   };
#pragma pack()

// Implementation
public:
	long Play( DWORD dwFlags );
	long LoadFromWaveFile( LPCTSTR Filename );
   BOOL IsLoaded(void) const { return m_bLoaded; };

// Attributes
protected:
   LPDIRECTSOUNDBUFFER m_lpSound;
   BOOL m_bLoaded;

};

#endif // !defined(AFX_DSOUND_H__F7C577F7_39DF_11D2_879C_0080AD509054__INCLUDED_)
