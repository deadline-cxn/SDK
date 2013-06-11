// G_Character.cpp: implementation of the CG_Character class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "G_Character.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CG_Character, CG_Item );

CG_Character::CG_Character()
{
   m_nNumAnims = 0;
   m_activeanim = -1;
   m_frameidx = 0;
   m_bTurningLeft = FALSE;
   m_surface_rev = NULL;
   for( int i=0; i<MAX_ANIMS; i++ ) m_anims[i].nFreq = 8;
}

CG_Character::~CG_Character()
{

}


//////////////////////////////////////////////////////////////////////
// Implementation

BOOL CG_Character::AttachObjectProperties()
{
   CG_Item::AttachObjectProperties();
   AddProp( _T("ANIMNAME"), VAR_STRING, &m_sAnimName );
   return TRUE;
}

long CG_Character::FindAnim(LPCTSTR Name)
{
   ASSERT(Name);
   CString sName(Name);
   sName.MakeUpper();
   BOOL bFound = FALSE;
   for( long i=0; i<m_nNumAnims; i++ ) {
      if( m_anims[i].sName == sName ) { bFound=TRUE; break; };
   };
   if( !bFound ) {
      // Error: not found.
      // Really returns index to first anim!!!
      CApp *app = (CApp *)AfxGetApp();
      app->m_debug.Error(_T("Anim %s not found."), Name);
      return 0; 
   };                     
   return i;
}




//////////////////////////////////////////////////////////////////////
// Master functions

BOOL CG_Character::Action( CController *pGame )
{
   if( m_state.IsReadyForNewState() ) 
      m_state.GetNewState( pGame,this );
   return m_state.ProcessState(pGame,this);
};

BOOL CG_Character::UpdateFrame(CController *pGame)
{
   ASSERT( pGame );
   ASSERT_VALID( pGame->m_current_room );
   ASSERT(m_activeanim>=0);
   if( m_RoomID != pGame->m_current_room->m_ID ) return TRUE; // not in current room
   TUpdateBlock blk;
   blk.obj = this;
   blk.pos = m_pos;
   blk.src_rc = m_anims[m_activeanim].Frames[m_frameidx];
   if( m_bTurningLeft ) {
      int left = blk.src_rc.left;
      int right = blk.src_rc.right;
      blk.src_rc.left  = 320-right;
      blk.src_rc.right = 320-left;
      blk.surface = m_surface_rev;
   }
   else
      blk.surface = m_surface;
   pGame->AddUpdateBlock(blk);
   return TRUE;
}

BOOL CG_Character::ManageAnims(CController *pGame, LPCTSTR szAnimName/*=NULL*/)
{
   // Make sure we're pointing at the right animation
   // or else reset animation pointer
   if( szAnimName!=NULL ) {
      if( m_sAnimName!=szAnimName ) {
         long idx = FindAnim(szAnimName);
         if( m_activeanim!=idx ) {
            m_activeanim = idx;
            m_frameidx = 0;
            m_sAnimName = m_anims[idx].sName;
            m_freq = 0;
         };
      };
   };
   // Make sure we loop frames
   if( m_freq<=0 ) {
      m_frameidx++;
      m_freq = (short)m_anims[m_activeanim].nFreq;
   };
   m_freq--;
   if( m_frameidx >= m_anims[m_activeanim].Frames.GetSize() ) {
      if( m_ai==ANIMSEQ ) {
         m_state.anim.wait = FALSE;
         m_state.anim.name.Empty();
      };
      m_frameidx = 0;
   };
   return TRUE;
}


BOOL CG_Character::SetPosition(CG_ID room, CPoint &pos)
{
   m_state.ResetStateAttributes();
   m_RoomID = room;
   m_pos = pos;
   m_ai = STAND;
   return TRUE;
}
