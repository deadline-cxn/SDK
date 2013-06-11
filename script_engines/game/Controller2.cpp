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
// Control functions

BOOL CController::ParseScripts()
{
   CApp *app = (CApp *)AfxGetApp();
   CString Filename;
   BOOL ok;

   app->m_debug.Print(_T("Parsing defintions...."));
   CScriptParser def_parser;
   def_parser.Init(this);
   Filename = KfxLocateFile( _T("scripts\\data.scr") );
   ok = def_parser.ProcessFile(Filename);
   if( ok ) 
      ok = def_parser.Validate();
   else
      app->m_debug.Print(_T("Error while parsing definitions!"));
   def_parser.Close();

   app->m_debug.Print(_T("Parsing states...."));
   CStateCompiler state_parser;
   state_parser.Init(this);
   Filename = KfxLocateFile( _T("scripts\\states.s") );
   ok = state_parser.ProcessFile(Filename);
   if( ok ) 
      ;
   else
      app->m_debug.Print(_T("Error while parsing states!"));
   state_parser.Close();

   return ok;
};

BOOL CController::ParseCode()
{
   CApp *app = (CApp *)AfxGetApp();
   CString Filename;
   BOOL ok;
   app->m_debug.Print(_T("Compiling code...."));
   CCompiler compiler;
   compiler.Init(this);
   Filename = KfxLocateFile( _T("scripts\\main.gc") );
   ok = compiler.ProcessFile(Filename);
   Filename = BfxGetAppPath() + _T("data\\logic.dat");
   app->m_debug.Print(_T("Saving logic code...."));
   compiler.Save(Filename);
   compiler.Close();
   if( ok ) {
      app->m_debug.Print(_T("Compile successfull."));
   }
   else {
      app->m_debug.Print(_T("Compile errors!!!"));
   };
   return ok;
};

BOOL CController::LoadLogic()
{
   CApp *app = (CApp *)AfxGetApp();
   CString Filename;
   Filename = KfxLocateFile(_T("data\\logic.dat"));
   if( Filename.IsEmpty() ) return FALSE;
   app->m_debug.Print(_T("Loading logic code...."));
   CLogicFile f;
   BOOL ok = f.Load(Filename,
                    m_eventmaps, m_eventmapsize,
                    m_code, m_codesize,
                    m_data, m_datasize);
   app->m_debug.PrintReplaceLine(_T("Logic code loaded."));
   return ok;
};


//////////////////////////////////////////////////////////////////////
// Init Control functions

BOOL CController::CreateScene()
{
   HRESULT ret;
   m_scene.Init();
   ret = m_scene.Create( m_dd, m_dd->GetWidth(), m_dd->GetHeight(), DDSCAPS_VIDEOMEMORY );
   if(ret!=DD_OK) {
      DEBUGERROR(_T("Error loading initial scene surface (%08X)."),ret);
      return FALSE;
   };
   return TRUE;
}

BOOL CController::CreateSplash(CDirectSurface &surface)
{
   DEBUGLOG(_T("Loading splash."));
   // We need to attach a proper palette (to system!!!)
   CString sFilename;
   sFilename = KfxLocateFile(_T("title1.lbm"));
   if( sFilename.IsEmpty() ) {
      DEBUGERROR(_T("Unable to locate title splash screen."));
      return FALSE;
   };
   m_dd->SetPaletteILBM( sFilename );
   HRESULT ret;
   ret = surface.LoadILBM( m_dd, sFilename );
   if( ret!=DD_OK ) return FALSE;
   surface.SetPalette( m_dd->GetPalette() );
   CRect rc( 0,0,surface.GetWidth()-1,surface.GetHeight()-1 );
   ret = surface.BltTo( m_dd->GetPrimarySurface(), 
                        rc, 
                        CRect(CPoint(0,35),rc.Size()), 
                        FALSE);
   if( ret!=DD_OK ) return FALSE;
   return TRUE;
}

BOOL CController::PrepareWorld()
{
   CApp *app = (CApp *)AfxGetApp();
   BOOL ok;
   int i;

   DEBUGLOG(_T("Preparing world."));

   // Look up hero!!!
   if( m_current_hero==NULL ) {
      CG_Character *hero;
      hero = m_objects.FindPlayer("HERO");
      if( hero==NULL ) return FALSE;
      m_current_hero = (CG_Hero *)hero;
   };

   //
   // Prepare the surfaces
   //
   // First surface we look at is the 'scene' which
   // is the most important surface next to the primary/back
   // drawing surface
   DEBUGLOG(_T("Loading surfaces..."));
   // We need to attach a proper palette (to system!!!)
   // and we need to do it now!
   CString sFilename;
   sFilename = KfxLocateFile(_T("room01.lbm")); // this is the default palette file
   if( sFilename.IsEmpty() ) return DEBUGERROR(_T("Error locating default palette file!"));
   m_dd->SetPaletteILBM( sFilename );
   m_scene.Clear();
   m_scene.SetPalette(m_dd->GetPalette());
   m_scene.SetColorKey(COLORKEY);
   // then we add all other surfaces...
   if( !AttachSurface( m_current_hero ) ) return FALSE;
   POSITION pos = m_objects.GetStartPosition();
   while( pos!=NULL ) {
      CString key;
      CG_Object *obj;
      m_objects.GetNextAssoc( pos, key, obj );
      // Attach surdface to object
      if( !AttachSurface( obj ) ) return FALSE;
   };
   app->m_debug.PrintReplaceLine(_T("Surfaces loaded."));

   //
   // Bind all state maps to objects' statemachine
   //
   for( i=0; i<m_liveobjects.GetSize(); i++ ) {
      CString name( m_liveobjects[i]->m_sLinkStateCode );
      if( name.IsEmpty() ) continue;
      name.MakeUpper();
      BOOL bFound = FALSE;
      for(int j=0; j<m_statemaps.GetSize(); j++ ) {
         if( m_statemaps[j].Name == name ) { bFound=TRUE; break; }
      }
      if( !bFound ) continue;
      ok = m_liveobjects[i]->m_state.Bind(&m_statemaps[j]);
      if( !ok ) {
         DEBUGERROR(_T("Error while binding state code."));
         return FALSE;
      };
   };

   //
   // Bind all event maps to objects' VM
   //
   for( i=0; i<m_liveobjects.GetSize(); i++ ) {
      CString eventmap( m_liveobjects[i]->m_sLinkEventCode );
      if( eventmap.IsEmpty() ) continue;
      ok = m_liveobjects[i]->m_vm.Bind(this,
                                       m_liveobjects[i],                              
                                       eventmap,
                                       m_eventmaps, m_eventmapsize,
                                       m_code,m_codesize,
                                       m_data,m_datasize);
      if( !ok ) {
         DEBUGERROR(_T("Error while binding logic code."));
         return FALSE;
      };
   };

   //
   // Send EVENT_INIT to all objects
   //
   app->m_debug.Print(_T("Initializing objects..."));
   PostMessageToAll(EVENT_INIT);
   PostMessageToAll(EVENT_SPAWN);
   // Process the init code for all objects
   // We're not really sure this will run ALL the code
   // but it's a good attempt.
   // This also allows some object interaction at startup.
   for( i=0; i<m_liveobjects.GetSize(); i++ ) m_liveobjects[i]->m_vm.Schedule(300);
   for( i=0; i<m_liveobjects.GetSize(); i++ ) m_liveobjects[i]->m_vm.Schedule(300);
   for( i=0; i<m_liveobjects.GetSize(); i++ ) m_liveobjects[i]->m_vm.Schedule(100);

   //
   // Create initial room
   //
   if( !CreateRoom(m_current_hero->m_sStartRoom) ) return FALSE;
   
   m_current_mouse = m_objects.FindItem(_T("MOUSE"));

   return TRUE;
}

BOOL CController::AttachSurface( CG_Object *obj )
// Attaches a surface to a game object
// If the surface does not already exist, it is created
// and populated with the ILBM picture.
// If the game object is a character, another surface
// is created and flipped (creating the left/right mirror of
// all animations)
{
   ASSERT_VALID( obj );
   // See if surface has not already been created
   CDirectSurface *surf = NULL;
   CDirectSurface *rightsurf = NULL;

   BOOL bFound = m_surfaces.Lookup( obj->m_sSurfaceName, surf );
   m_surfaces.Lookup( obj->m_sSurfaceName + _T("_REV"), rightsurf );
   
   if( bFound==FALSE ) {
      // It has not, we need to create it and load
      // the picture into it...
      CApp *app = (CApp *)AfxGetApp();
      app->m_debug.PrintReplaceLine(_T("Loading surface '%s'."),obj->m_sSurfaceName);
      surf = new CDirectSurface;
      if( surf==NULL ) {
         app->m_debug.Error(_T("Out of memory."));
         return FALSE;
      };
      surf->Init();
      CString sFilename = KfxLocateFile(obj->m_sSurfaceName);
      if( sFilename.IsEmpty() ) {
         app->m_debug.Error(_T("Unable to locate surface '%s'."),sFilename);
         return FALSE;
      };
      HRESULT ok;
      ok = surf->LoadILBM(m_dd, sFilename);
      if( ok==DD_OK ) {
         surf->SetPalette(m_dd->GetPalette());
         surf->SetColorKey(COLORKEY);
         // Add it to list of surfaces
         m_surfaces.SetAt( obj->m_sSurfaceName, surf );

         // Do we need to create a mirror surface
         // If so, we must attach it to the object too
         if( obj->IsKindOf(RUNTIME_CLASS(CG_Character))) {
            // For character like classes we must create
            // the "reverse" surface (for left/right images)
            rightsurf = new CDirectSurface;
            if( rightsurf==NULL ) {
               app->m_debug.Error(_T("Out of memory."));
               return FALSE;
            };
            rightsurf->Init();
            ok = rightsurf->CreateReverse( m_dd, surf );
            if( ok!=DD_OK ) {
               app->m_debug.Error(_T("Unable to create surface."));
               return FALSE;
            };        
            rightsurf->SetPalette(m_dd->GetPalette());
            rightsurf->SetColorKey(COLORKEY);
            // Add it to list of surfaces
            CString sName;
            sName = obj->m_sSurfaceName + _T("_REV");
            m_surfaces.SetAt( sName, rightsurf );
         };
      }
      else {
         app->m_debug.Error(_T("Error loading surface '%s'."),sFilename);
         return FALSE;
      };
   };
   obj->m_surface = surf;
   if( obj->IsKindOf(RUNTIME_CLASS(CG_Character)))
      ((CG_Character *)obj)->m_surface_rev = rightsurf;
   return TRUE;
};

//////////////////////////////////////////////////////////////////////
// Misc

BOOL CController::PostMessageToAllInRoom(EVENTTYPE event, CG_ID room, CG_Item *source/*=NULL*/, long param1/*=0*/, long param2/*=0*/)
{
   if( room<=0 ) return FALSE;
   for( int i=0; i<m_liveobjects.GetSize(); i++ )
      if( m_liveobjects[i]->m_RoomID == room )
         if( m_liveobjects[i] != source )
            m_liveobjects[i]->m_vm.PostMessage(event,source,param1,param2);
   return TRUE;
}

BOOL CController::PostMessageToAll(EVENTTYPE event, CG_Item *source/*=NULL*/, long param1/*=0*/, long param2/*=0*/)
{
   for( int i=0; i<m_liveobjects.GetSize(); i++ )
      m_liveobjects[i]->m_vm.PostMessage(event,source,param1,param2);
   return TRUE;
}

