// DSound.cpp: implementation of the CDirectSound class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSound.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectSound::CDirectSound()
{

}

CDirectSound::~CDirectSound()
{

}

long CDirectSound::Play(DWORD dwFlags)
{
   if( !m_bLoaded ) return 1;

   DWORD dwStatus;
   if ( m_lpSound->GetStatus(&dwStatus)!=0 ) {
      TRACE0( "Error while getting sound status\n" );
      return 2;
   };
   if ((dwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING) {
      // Play the sound
      if (m_lpSound->Play(0, 0, dwFlags)!=0) {
         TRACE0( "Error while starting sound\n" );
         return 3;
      };
   }
   return TRUE;
}

long CDirectSound::LoadFromWaveFile(LPCTSTR Filename)
{
   return 0;
}
