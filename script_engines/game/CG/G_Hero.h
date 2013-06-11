// G_Hero.h: interface for the CG_Hero class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G_HERO_H__7ABB2D97_48C5_11D2_87BD_0080AD509054__INCLUDED_)
#define AFX_G_HERO_H__7ABB2D97_48C5_11D2_87BD_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "G_Object.h"
#include "G_Character.h"

class CG_Hero : public CG_Character  
{
public:
	CG_Hero();
	virtual ~CG_Hero();
   DECLARE_DYNAMIC( CG_Hero );

// Implementation
public:
   virtual BOOL Action( CController *pGame );
   virtual BOOL UpdateFrame( CController *pGame );
protected:
	virtual BOOL AttachObjectProperties();

};

#endif // !defined(AFX_G_HERO_H__7ABB2D97_48C5_11D2_87BD_0080AD509054__INCLUDED_)
