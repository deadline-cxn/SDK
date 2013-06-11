// G_Character.h: interface for the CG_Character class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G_CHARACTER_H__FDE67477_3C55_11D2_87A3_0080AD509054__INCLUDED_)
#define AFX_G_CHARACTER_H__FDE67477_3C55_11D2_87A3_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "G_Object.h"



class CVirtualMachine;

class CG_Character : public CG_Item
{
public:
	CG_Character();
	virtual ~CG_Character();
   DECLARE_DYNAMIC( CG_Character );

// Types
public:
   typedef struct {
      CString sName;
      long nFreq;
      CFrameArray Frames;
   } TAnim;

// Attributes
public:
#define MAX_ANIMS 15
   //
   CString m_sAnimName;    // name of the currently active animation strip
   long   m_activeanim;    // currently active animation (index to m_anims)
   TAnim  m_anims[MAX_ANIMS];
   short  m_nNumAnims;     // number of available anims in m_anims
   long   m_frameidx;      // current frame pointer to anim strip
   short  m_freq;
   //
   CDirectSurface *m_surface_rev;

// Implementation
public:
	virtual BOOL ManageAnims( CController *pGame, LPCTSTR szAnimName=NULL );
	BOOL SetPosition( CG_ID room, CPoint &pos );
	long FindAnim( LPCTSTR Name );
   //
   virtual BOOL UpdateFrame( CController *pGame );
   virtual BOOL Action( CController *pGame );

protected:
	virtual BOOL AttachObjectProperties();
};

#endif // !defined(AFX_G_CHARACTER_H__FDE67477_3C55_11D2_87A3_0080AD509054__INCLUDED_)
