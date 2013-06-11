// G_Room.h: interface for the CG_Room class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G_ROOM_H__FDE67476_3C55_11D2_87A3_0080AD509054__INCLUDED_)
#define AFX_G_ROOM_H__FDE67476_3C55_11D2_87A3_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CG\G_Object.h"

class CController;


class CG_Room : public CG_Object  
{
public:
	CG_Room();
	virtual ~CG_Room();
   DECLARE_DYNAMIC( CG_Room );

// Types
public:
   typedef struct {
      CG_Object *obj;
      CPoint pos;
   } TRoomObject;
   typedef CArray<TRoomObject,TRoomObject&> CRoomObjectArray;
   typedef struct {
      CString room;
      CRect exitarea;
      CPoint startpos;
   } TRoomExit;
   typedef CArray<TRoomExit,TRoomExit&> CRoomExitArray;

// Attributes
public:
   CRoomObjectArray m_objects;
   CRoomObjectArray m_staticobjects;
   CRoomExitArray   m_exits;

// Implementation
public:
	virtual BOOL UpdateFrame( CDirectDraw *pDp, CController *pGame );
	virtual BOOL Create( CDirectDraw *pDp, CController *pGame, CDirectSurface *pScene );
protected:
	virtual BOOL AttachObjectProperties();

};

#endif // !defined(AFX_G_ROOM_H__FDE67476_3C55_11D2_87A3_0080AD509054__INCLUDED_)
