// G_Item.cpp: implementation of the CG_Item class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "G_Item.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CG_Item, CG_Prop );

CG_Item::CG_Item()
{
   m_ai = STAND;
   m_RoomID = OBJ_NOTVISIBLE;
   m_pos = CPoint(0,0);
   m_nVelocity = 100;
   m_bPickable = FALSE;
   m_bWearable = FALSE;
}

CG_Item::~CG_Item()
{

}


//////////////////////////////////////////////////////////////////////
// Implementation

BOOL CG_Item::AttachObjectProperties()
{
   CG_Prop::AttachObjectProperties();
   AddProp( _T("ROOM"), VAR_LONG, &m_RoomID );
   AddProp( _T("XPOS"), VAR_LONG, &m_pos.x );
   AddProp( _T("YPOS"), VAR_LONG, &m_pos.y );
   AddProp( _T("VELOCITY"), VAR_LONG, &m_nVelocity );
   AddProp( _T("PICKABLE"), VAR_LONG, &m_bPickable );
   AddProp( _T("WEARABLE"), VAR_LONG, &m_bWearable );
   AddProp( _T("ISTURNINGLEFT"), VAR_LONG, &m_bTurningLeft );
   AddProp( _T("STATE"), VAR_LONG, &m_state.m_nState );
   return TRUE;
}

BOOL CG_Item::Action(CController *pGame)
{
   return TRUE;
}

BOOL CG_Item::UpdateFrame(CController *pGame)
{
   ASSERT_VALID(pGame);
   ASSERT_VALID(pGame->m_current_room);
   
   if( m_RoomID != pGame->m_current_room->m_ID ) return TRUE; // not in current room
   if( m_frames.GetSize()==0 ) return FALSE;
   TUpdateBlock blk;
   blk.obj = this;
   blk.surface = m_surface;
   blk.pos = m_pos;
   blk.src_rc = m_frames[0];
   pGame->AddUpdateBlock(blk);
 
   return TRUE;
}

BOOL CG_Item::ManageAnims(CController *pGame, LPCTSTR szAnimName/*=NULL*/)
{
   return TRUE;
};