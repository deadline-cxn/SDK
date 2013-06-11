// G_Room.cpp: implementation of the CG_Room class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "G_Room.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CG_Room, CG_Object );

CG_Room::CG_Room()
{

}

CG_Room::~CG_Room()
{

}


//////////////////////////////////////////////////////////////////////
// Implementation

BOOL CG_Room::AttachObjectProperties()
{
   CG_Object::AttachObjectProperties();
   return TRUE;
}

BOOL CG_Room::Create(CDirectDraw *pDp, CController *pGame, CDirectSurface *pScene)
{
#define TOPOFFSET 8
   ASSERT_VALID( pDp );
   ASSERT_VALID( pGame );
   ASSERT_VALID( pScene );
   ASSERT_VALID( m_surface );
   if( m_surface==NULL ) return FALSE;
   CRect src_rc;
   CRect dst_rc;
   int i;
   // First erase all...
   pScene->Clear();
   // ...then blit the room-bitmap itself to the scene surface
   src_rc = CRect( 0,0, m_surface->GetWidth()-1,m_surface->GetHeight()-1 );
   dst_rc = CRect( CPoint(0,TOPOFFSET), src_rc.Size() );
   pScene->BltFrom(m_surface,dst_rc,src_rc);
   // Also blit the menu
   CG_Prop *prop = pGame->m_props.Find("Menu");
   if( prop!=NULL ) {
      src_rc = prop->m_frames[0];
      dst_rc = CRect( CPoint(0,pDp->GetHeight()-45), src_rc.Size() );
      pScene->BltFrom(prop->m_surface,dst_rc,src_rc);
   };
   // Then blit all the static props to the scene...
   for( i=0; i<m_staticobjects.GetSize(); i++ ) {
      CG_Prop *prop = (CG_Prop *)m_staticobjects[i].obj;
      ASSERT_VALID( prop );
      if( prop->m_frames.GetSize()==0 ) return FALSE;
      src_rc = prop->m_frames[0];
      dst_rc = CRect( m_staticobjects[i].pos, src_rc.Size() );
      pScene->BltFrom(prop->m_surface,dst_rc,src_rc);
   };
   return TRUE;
}

BOOL CG_Room::UpdateFrame(CDirectDraw *pDp, CController *pGame)
{
   // Go through all non-static items that should
   // be in this room (such as plants and stuff, which
   // must interact with the players) and add them
   // to the display list...
   CRect src_rc;
   CRect dst_rc;
   int i;
   for( i=0; i<m_objects.GetSize(); i++ ) {
      CG_Item *prop = (CG_Item *)m_objects[i].obj;
      ASSERT_VALID( prop );
      if( prop->m_frames.GetSize()==0 ) return ERROR;
      TUpdateBlock blk;
      blk.obj = prop;
      blk.surface = prop->m_surface;
      blk.src_rc = prop->m_frames[0];
      blk.pos = m_objects[i].pos;
      pGame->AddUpdateBlock(blk);
   };
   return TRUE;
}
