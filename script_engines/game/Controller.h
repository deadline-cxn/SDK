// G_Controller.h: interface for the CController class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G_CONTROLLER_H__FDE67473_3C55_11D2_87A3_0080AD509054__INCLUDED_)
#define AFX_G_CONTROLLER_H__FDE67473_3C55_11D2_87A3_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScriptParser;

#include "Misc\Cache.h"

typedef CTypedPtrMap<CMapStringToOb, CString, CDirectSurface*> CDirectSurfaceMap;
typedef CG_ObjMap<CG_Object*> CG_ObjectMap;
typedef CG_ObjArray<CG_Room*> CG_RoomArray;
typedef CG_ObjArray<CG_Prop*> CG_PropArray;
typedef CG_ObjArray<CG_Item*> CG_ItemArray;
typedef CG_ObjArray<CG_Character*> CG_CharacterArray;
typedef CMap<CString, LPCTSTR, DWORD, DWORD&> CConstantMap;
typedef CArray< TStateMap, TStateMap& > CStateMapArray;

typedef struct {
   CG_Object *obj;
   CDirectSurface *surface;
   CRect src_rc;
   CPoint pos;
} TUpdateBlock;
typedef CArray<TUpdateBlock, TUpdateBlock&> CUpdateArray;

class CCompiler;
class CStateCompiler;

class CController : public CObject  
{
   friend CCompiler;
   friend CStateCompiler;
public:
	CController();
	virtual ~CController();

// Types
public:
#define COLORKEY CLR_INVALID
   typedef CArray<CRect, CRect&> CPatchArray;

// Attributes
public:
   CDirectDraw *m_dd;                  // ptr to global DirectDraw object
   //
   CDirectSurfaceMap m_surfaces;
   //
   CG_Hero *m_current_hero;
   CG_Room *m_current_room;
   CG_Item *m_current_mouse;
   //
   CG_ObjectMap m_objects;      // Contains ALL objects  
   CG_ItemArray m_liveobjects;  // Contains both Items and Players!!!  
   //
   CG_PropArray m_props;
   CG_RoomArray m_rooms;
   CG_CharacterArray m_chars;
   CG_ItemArray m_items;
   //
   CPoint m_mousepos;
   UINT   m_mousestate;
   //
protected:
   CDirectSurface m_scene;             // this is the backdrop surface
   //
   CPatchArray m_patches[2];       // these are CRect area to copy back to scene
   CUpdateArray m_updateblocks[2]; // these are new blocks to update on scene
   //
   CConstantMap m_constants;
   //
   short nFlip;                    // Surface number (primary or back)
   //
   TEventMap *m_eventmaps;
   DWORD m_eventmapsize;
   CODEPTR *m_code;
   DWORD m_codesize;
   DATAPTR *m_data;
   DWORD m_datasize;
   //
   CStateMapArray m_statemaps;

   //
   short          m_nMenuCmd;
   CG_Object     *m_objMenuItem;


// Methods
public:
   BOOL Init();
   BOOL Close();
	//
   BOOL Print( LPCTSTR Alphabet, LPCTSTR Text, CPoint &pos );
	BOOL PostMessageToAll( EVENTTYPE event, CG_Item *source=NULL, long param1=0, long param2=0 );
	BOOL PostMessageToAllInRoom( EVENTTYPE event, CG_ID room, CG_Item *source=NULL, long param1=0, long param2=0 );
   //
   BOOL GetConstant( LPCTSTR name, DWORD &value ) { return m_constants.Lookup(name,value); };
	BOOL AddConstant( LPCTSTR Name, DWORD val )
   {
      CString tmp(Name);
      tmp.MakeUpper();
      m_constants.SetAt(tmp,val);
      return TRUE;
   };
   //
	BOOL IsValidSpot( CPoint &pos );
   BOOL AddUpdateBlock( TUpdateBlock &block );
   //
   BOOL UpdateFrame();
   BOOL RunCode();

private:
   BOOL ParseScripts();
   BOOL ParseCode();
   BOOL LoadLogic();
   BOOL AttachSurface( CG_Object *obj );
   BOOL CreateRoom( LPCTSTR Name );
	BOOL CreateSplash( CDirectSurface &surface );
	BOOL CreateScene();
	BOOL GraphicsUpdateFrame();
	BOOL MenuUpdateFrame();
	BOOL CheckExits();
	BOOL InventoryUpdateFrame();
   BOOL GetMouseInfo();
	BOOL MouseUpdateFrame();
	BOOL PrepareWorld();
};

#endif // !defined(AFX_G_CONTROLLER_H__FDE67473_3C55_11D2_87A3_0080AD509054__INCLUDED_)
