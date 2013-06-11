// G_Controller.cpp: implementation of the CController class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Controller.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CController::CController()
{
   m_dd = NULL;
   m_code = NULL;
   m_data = NULL;
   m_eventmaps = NULL;
   m_current_room = NULL;
   m_current_hero = NULL;
   nFlip = 0;
   m_nMenuCmd = -1;
}

CController::~CController()
{
   Close();
}


//////////////////////////////////////////////////////////////////////
// Implementation

BOOL CController::Init()
{
   CApp *app = (CApp *)AfxGetApp();
   CDirectSurface splash;
  
   // Initialize Direct Draw
   DEBUGLOG(_T("Starting gfx engine..."));
   app->m_dd.SetFullScreenMode( app->config.bFullScreen );
   app->m_dd.Init( app->m_hwnd );
   m_dd = &app->m_dd; // set local attribute

   CreateScene();
   CreateSplash(splash);

   // Parse definitions
   BOOL ok;
   ok = ParseScripts();
   if( !ok ) return FALSE;
   if( m_current_hero==NULL ) return FALSE;
   if( m_chars.GetSize()==0 ) return FALSE;
   if( m_rooms.GetSize()==0 ) return FALSE;
   if( m_items.GetSize()==0 ) return FALSE;
   if( m_props.GetSize()==0 ) return FALSE;
   // Initialize objects
   m_current_hero->CreatePropertyList();
   m_rooms[0]->CreatePropertyList();
   m_chars[0]->CreatePropertyList();
   m_items[0]->CreatePropertyList();
   m_props[0]->CreatePropertyList();
   // Parse event code
   ok = ParseCode();
   if( !ok ) return FALSE;
   ok = LoadLogic();
   if( !ok ) return FALSE;
   
   /*// Switch to fullscreen
   if( app->config.bFullScreen ) {
      m_dd->SetExclusive(app->m_hwnd,TRUE);
   };*/

   ok = PrepareWorld();
   if( !ok ) return FALSE;

   DEBUGLOG(_T("System ready."));

   return TRUE;
};

BOOL CController::Close()
{
   //
   m_rooms.RemoveAll();
   m_props.RemoveAll();
   m_items.RemoveAll();
   m_chars.RemoveAll();
   //
   m_scene.Close();
   //
   POSITION pos;   
   pos = m_surfaces.GetStartPosition();
   while( pos!=NULL ) {
      CString key;
      CDirectSurface *surf;
      m_surfaces.GetNextAssoc(pos,key,surf);
      delete surf;
   };
   m_surfaces.RemoveAll();
   //
   m_patches[0].RemoveAll();  // friendly cleanup
   m_patches[1].RemoveAll();
   //
   if( m_eventmaps!=NULL) delete [] m_eventmaps;
   m_eventmaps = NULL;
   if( m_code!=NULL) delete [] m_code;
   m_code = NULL;
   if( m_data!=NULL) delete [] m_data;
   m_data = NULL;

   return TRUE;
};


//////////////////////////////////////////////////////////////////////
// Control functions

BOOL CController::CreateRoom( LPCTSTR Name )
{
   ASSERT_VALID(m_dd);
   ASSERT(AfxIsValidString(Name));
   CApp *app = (CApp *)AfxGetApp();
   
   CG_Room *room;
   room = m_objects.FindRoom(Name);
   if( room==NULL ) return FALSE;
   
   PALETTEENTRY OrgPalette[256];
   PALETTEENTRY BlackPalette[256];
   m_dd->GetPaletteDirect( OrgPalette );
   m_dd->GetBlackPalette( BlackPalette );
   m_dd->FadePalette(OrgPalette,BlackPalette,8);

   BOOL ok = room->Create( m_dd, this, &m_scene );
   if( ok ) {
      // This is our room...
      m_current_room = room;
      m_nMenuCmd = -1;
      // Remove all previous drawing requests...
      m_patches[0].RemoveAll();
      m_patches[1].RemoveAll();
      m_updateblocks[0].RemoveAll();
      m_updateblocks[1].RemoveAll();
      // Now paint the large scene
      CRect rc( 0,0, m_scene.GetWidth()-1,m_scene.GetHeight()-1 );
      m_patches[0].Add(rc);
      m_patches[1].Add(rc);
      GraphicsUpdateFrame();
      GraphicsUpdateFrame();
      // Here we send event messages to all objects in the room
      PostMessageToAllInRoom( EVENT_ENTERROOM,m_current_room->m_ID,m_current_hero );
   }
   else
      app->m_debug.Error(_T("Unable to create room '%s'."),(LPCTSTR)Name);

   m_dd->FadePalette(BlackPalette,OrgPalette,8);
   return ok;
};

BOOL CController::IsValidSpot(CPoint &pos)
// A complex function that will actually
// test the pixel in the scene if it's available
// (a valid destination)
{
   return TRUE;
#define LAYER_COLOR 224
   DWORD p;
   p = m_scene.GetPixelIndex(pos.x-5,pos.y);
   if( p<LAYER_COLOR ) return FALSE;
   p = m_scene.GetPixelIndex(pos.x+5,pos.y);
   if( p<LAYER_COLOR ) return FALSE;
   return TRUE;
}

BOOL CController::AddUpdateBlock(TUpdateBlock &block)
{
   ASSERT(block.surface);
   m_updateblocks[nFlip].Add(block);
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Real-Time functions

static int _cdecl compare_updateblock( const void *elem1, const void *elem2 )
{
   int h1 = ((TUpdateBlock *)elem1)->pos.y + ((TUpdateBlock *)elem1)->src_rc.Height();
   int h2 = ((TUpdateBlock *)elem2)->pos.y + ((TUpdateBlock *)elem2)->src_rc.Height();
   if( h1==h2 ) return 0; 
   if( h1<h2 ) return -1; 
   return 1;
};

BOOL CController::UpdateFrame()
{
   ASSERT_VALID( m_current_room );
   ASSERT_VALID( m_current_hero );
   CApp *app = (CApp *)AfxGetApp();
   int i,cnt;

   //
   // Flip surfaces
   //
   m_dd->Flip(app->m_hwnd);
   nFlip = (nFlip+1) & 1;

   // Get and initialize various runtime objects
   GetMouseInfo();

   //
   // Restore previous screen
   //
   cnt = m_patches[nFlip].GetSize();
   for( i=0; i<cnt; i++ ) {
      m_scene.BltTo(m_dd->m_lpDDSBack, m_patches[nFlip][i], m_patches[nFlip][i], FALSE );
   };
   m_patches[nFlip].SetSize(0,50);

   // Also add room props (non-static things)
   m_current_room->UpdateFrame( m_dd, this );

   //
   // Move players and items
   //
   for( i=0; i<m_liveobjects.GetSize(); i++ ) m_liveobjects[i]->Action(this);
   for( i=0; i<m_liveobjects.GetSize(); i++ ) m_liveobjects[i]->UpdateFrame(this);

   // Sort update patches according to pos+height (lowest vertical position)
   // This will draw all object from top to bottom...
   qsort( &m_updateblocks[nFlip][0], 
          m_updateblocks[nFlip].GetSize(), 
          sizeof(TUpdateBlock), 
          compare_updateblock);

   // Add inventory items to top-display
   InventoryUpdateFrame();
   // Do some menu/mouse stuff
   // ( Note: This should be the almost last thing, since we use
   //   the display list to find objects )
   MenuUpdateFrame();
   // Add mouse
   MouseUpdateFrame();

   //
   // Update screen with new graphics
   //
   GraphicsUpdateFrame();

   //
   // Run scripting code
   //
   RunCode();

   //
   // Check various other states
   //
   CheckExits();

   return TRUE;
};

BOOL CController::RunCode()
{
   const int timeslice = 50;
   int cnt;
   BOOL ok;
   cnt = m_chars.GetSize();
   for( int i=0; i<cnt; i++ ) {
      ok = m_chars[i]->m_vm.Schedule(timeslice);
   };
   return ok;
};


//////////////////////////////////////////////////////////////////////
// Real-Time helper functions

BOOL CController::GetMouseInfo()
{
   if( m_current_mouse==NULL ) return FALSE;
   // Get mouse position
   CApp *app = (CApp *)AfxGetApp();
   CPoint pos;
   ::GetCursorPos( &pos );
   ::ScreenToClient(app->m_hwnd,&pos);
   // Make sure point is placed on screen
   pos.x = min( max(0,pos.x), m_dd->GetWidth()-m_current_mouse->m_size.cx );
   pos.y = min( max(0,pos.y), m_dd->GetHeight()-m_current_mouse->m_size.cy );
   // get position
   m_mousepos = pos;
   m_mousestate = app->GetMouseState();
   app->SetMouseState(0); // we ready to receive another mouse event
                          // note that they are not buffered!
   return TRUE;
};

BOOL CController::MouseUpdateFrame()
{
   if( m_current_mouse==NULL ) return FALSE;
   if( m_current_mouse->m_frames.GetSize()==0 ) return FALSE;
   // Add mouse to update list
   TUpdateBlock blk;
   blk.obj = m_current_mouse;
   blk.surface = m_current_mouse->m_surface;
   blk.pos = m_mousepos;
   blk.src_rc = m_current_mouse->m_frames[0];
   m_updateblocks[nFlip].Add(blk);
   return TRUE;
}

BOOL CController::InventoryUpdateFrame()
{
#define XSPACE 10
   CPoint pos( 10,10 ); // start pos for inventory display
   for( int i=0; i<m_items.GetSize(); i++ )
      if( m_items[i]->m_RoomID == OBJ_INVENTORY ) {
         // Add inventory item
         TUpdateBlock blk;
         blk.obj = m_items[i];
         blk.surface = m_items[i]->m_surface;
         blk.pos = pos;
         blk.src_rc = m_items[i]->m_frames[0];
         m_updateblocks[nFlip].Add(blk);
         // Get next position...
         pos.x += blk.src_rc.Width() + XSPACE;
      };
   return TRUE;
}

BOOL CController::CheckExits()
{
   ASSERT_VALID(m_current_room);
   ASSERT_VALID(m_current_hero);
   // Run hero's position through all exit areas and
   // check if he has entered one
   BOOL bFound = FALSE;
   for( int i=0; i<m_current_room->m_exits.GetSize(); i++ ) {
      if( m_current_room->m_exits[i].exitarea.PtInRect( m_current_hero->m_pos ) ) { bFound=TRUE; break; };
   };
   if( !bFound ) return FALSE; // not in exit area!
   // Hero is in exit area! We must create a new room...
   CPoint newpos( m_current_room->m_exits[i].startpos );
   BOOL ok;
   // But first we will send an event to everyone that hero left the room
   PostMessageToAllInRoom( EVENT_EXITROOM,m_current_room->m_ID,m_current_hero );
   // then create the new room
   ok = CreateRoom( m_current_room->m_exits[i].room );
   if( ok ) {
      // We suceeded in creating a new room
      // 'm_current_room' now points to this! We must
      // also update a few hero attributes.
      m_current_hero->SetPosition(m_current_room->m_ID,newpos);
   };
   return ok;
}


BOOL CController::Print(LPCTSTR Alphabet, LPCTSTR Text, CPoint &pos)
{
   const TCHAR szLetters[] = _T(" ABCDEFGHIJKLMNOPQRSTUVWXYZ!?,.-ÆØÅ");
   // First we need to figure out
   // the size of things...
   CFixedString<64> szBuffer( Text );
   CG_Prop *bitmap = m_objects.FindProp(Alphabet);
   if( bitmap==NULL ) return FALSE;
   CPoint letter_pos( bitmap->m_frames[0].left, bitmap->m_frames[0].top );
   CSize letter_size( 8,bitmap->m_frames[0].Height() );
   //
   CPoint startpos( pos );
   szBuffer.MakeUpper(); // make sure letters are uppercased
   // Parse all letters and blit them one
   // by one to the scene surface
   for( int i=0; i<szBuffer.GetLength(); pos.x += 8, i++ ) {
      LPCTSTR p = _tcschr( szLetters, szBuffer[i] );
      if( p==NULL ) break; // unsupported char
      int charpos = (p-szLetters)/sizeof(TCHAR);
      CPoint xy( letter_pos.x+(charpos*8), letter_pos.y );
      // Add letter
      CRect src_rc;
      CRect dst_rc;
      src_rc = CRect( xy, letter_size );
      dst_rc = CRect( pos, src_rc.Size() );
      m_scene.BltFrom(bitmap->m_surface,dst_rc,src_rc,FALSE);
   };
   // Then make sure that the scene surface area is
   // copied to the primary and back surface in
   // the next frames...
   TUpdateBlock blk;
   blk.obj = bitmap;
   blk.surface = bitmap->m_surface;
   blk.pos = startpos;
   blk.src_rc = CRect( startpos, CSize(m_dd->GetWidth()-1,8) );
   m_updateblocks[0].Add(blk);
   m_updateblocks[1].Add(blk);
   return TRUE;
}

BOOL CController::MenuUpdateFrame()
{
   static const TCHAR menus[][16] = 
   {
      _T("UNDERSØG"), // 0
      _T("TAL TIL"), // 1
      _T("ÅBEN"), // 2
      _T("TAG"), // 3
      _T("GIV"), // 4
      _T("LÆG"), // 5
      _T("TRYK PÅ"), // 6
      _T("?"), // 7
      _T("HJÆLP"), // 8
      _T("INDSTILLINGER"), // 9
      _T(""), // 10
      NULL
   };
   static short LastCmd = 10;
   static CG_Object *LastItem = NULL;
   // First get a potential new menu command
   short MenuCmd = m_mousepos.x/32;
   if( MenuCmd<0 ) MenuCmd=0;
   if( MenuCmd>9 ) MenuCmd=9;
   // Is mouse on menu or above
   if( m_mousepos.y < m_dd->GetHeight()-50 ) {
      MenuCmd = 10;
      // If a menu command is active we may want to
      // display menu item...
      m_objMenuItem = NULL;
      if( m_nMenuCmd>-1 ) {
         int cnt = m_updateblocks[nFlip].GetSize();
         for( int i=0; i<cnt; i++ ) {
            TUpdateBlock blk( m_updateblocks[nFlip][i] );
            CRect rc( blk.pos, blk.src_rc.Size() );
            if( rc.PtInRect( m_mousepos ) ) { m_objMenuItem = blk.obj; break; };
         };
      };
   }
   else {
      if( m_mousestate==WM_LBUTTONDOWN ) m_nMenuCmd = MenuCmd;
      m_objMenuItem = NULL;
   };
   // If there is current a menu selected, then use it
   if( (MenuCmd==10) && (m_nMenuCmd!=-1) ) MenuCmd = m_nMenuCmd;
   // Already dispayed this item
   if( (LastCmd==MenuCmd) && (LastItem==m_objMenuItem) ) return TRUE;
   // Prepare string to be printed
   CFixedString<64> str(menus[MenuCmd]);
   if( m_objMenuItem!=NULL ) {
      ASSERT_VALID(m_objMenuItem);
      str += _T(" ");
      str += m_objMenuItem->m_sName;
   };
   // Print string
   int fill = 40 - str.GetLength();
   for( int i=0; i<fill; i++ ) str += _T(" ");
   Print( _T("SMALLLETTERS"), str, CPoint(0,m_dd->GetHeight()-70) );
   LastCmd = MenuCmd;
   LastItem = m_objMenuItem;
   return TRUE;
}

BOOL CController::GraphicsUpdateFrame()
{
   // Traverse all items and blit area from the object's surfaces
   // to the back display surface...
   int cnt = m_updateblocks[nFlip].GetSize();
   for( int i=0; i<cnt; i++ ) {
      TUpdateBlock blk( m_updateblocks[nFlip][i] );
      ASSERT(blk.surface);
      CRect dst_rc( blk.pos, blk.src_rc.Size() );
      blk.surface->BltTo(m_dd->GetBackSurface(), blk.src_rc, dst_rc );
      m_patches[nFlip].Add( dst_rc );
   };
   // Remove all blocks from list and re-grow
   m_updateblocks[nFlip].SetSize(0,50);
   return TRUE;
}
