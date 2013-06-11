// G_Item.h: interface for the CG_Item class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G_ITEM_H__780689AC_3AB2_11D2_879D_0080AD509054__INCLUDED_)
#define AFX_G_ITEM_H__780689AC_3AB2_11D2_879D_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "G_Object.h"

#define OBJ_NOTVISIBLE -1
#define OBJ_INVENTORY -2
#define OBJ_WORN -3


class CG_Item : public CG_Prop
{
public:
	CG_Item();
	virtual ~CG_Item();
   DECLARE_DYNAMIC( CG_Item );

// Attributes
public:
   CG_ID m_RoomID;
   CPoint m_pos;
   CSize  m_size;
   //
   BOOL m_bPickable;
   BOOL m_bWearable;
   BOOL m_bTurningLeft;
   long m_nVelocity;
   //
   AITYPE m_ai;
   CStateMachine m_state;
   CVirtualMachine m_vm;
   //
   // A few string used for initialization
   // (short be zero'ed out at startup)
   CString m_sLinkEventCode;
   CString m_sLinkStateCode;
   CString m_sStartRoom;

// Implementation
public:
   virtual BOOL AttachObjectProperties();
   virtual BOOL Action( CController *pGame );
   virtual BOOL UpdateFrame( CController *pGame );
   virtual BOOL ManageAnims(CController *pGame, LPCTSTR szAnimName=NULL);
};

#endif // !defined(AFX_G_ITEM_H__780689AC_3AB2_11D2_879D_0080AD509054__INCLUDED_)
