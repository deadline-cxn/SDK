// G_Hero.cpp: implementation of the CG_Hero class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "G_Hero.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CG_Hero, CG_Character );

CG_Hero::CG_Hero()
{
   m_nNumAnims = 0;
   m_activeanim = -1;
   m_frameidx = 0;
   m_ai = STAND;
}

CG_Hero::~CG_Hero()
{

}


//////////////////////////////////////////////////////////////////////
// Implementation

BOOL CG_Hero::AttachObjectProperties()
{
   CG_Character::AttachObjectProperties();
   return TRUE;
}

BOOL CG_Hero::UpdateFrame(CController *pGame)
{
   CG_Character::UpdateFrame(pGame);
   return TRUE;
}

BOOL CG_Hero::Action( CController *pGame )
{
   ASSERT_VALID(pGame);
   // First handle mouse-click
   switch( pGame->m_mousestate ) {
   case WM_LBUTTONDOWN:
      {
         CPoint newpos( pGame->m_mousepos );
         if( newpos.y > 160 ) break; // mouse click lower than room
         if( !pGame->IsValidSpot(newpos) ) {
            m_state.walk.steps = 0;
            break;
         };
         // Ajust mouse position to be right under hero
         DEBUGLOG("Click: %d %d", newpos.x,newpos.y);
         newpos.x -= m_size.cx/2;
         newpos.y -= (m_size.cy-10);
         // Set new state
         m_state.ResetStateAttributes();
         m_state.walk.x = (float)m_pos.x;
         m_state.walk.y = (float)m_pos.y;
         BOOL ok = KfxCalcWalk(m_pos, 
                               newpos,
                               m_nVelocity,
                               m_state.walk.steps,
                               m_state.walk.delta_x, m_state.walk.delta_y);
         if( !ok ) return FALSE; // this is bad...
         m_ai = WALKING; // ok, we're walking!!!
      }
      break;
   };
   
   return m_state.ProcessState(pGame,this);
};
