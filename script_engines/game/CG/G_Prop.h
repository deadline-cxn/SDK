// G_Prop.h: interface for the CG_Prop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G_PROP_H__9FCFD026_4554_11D2_87B4_0080AD509054__INCLUDED_)
#define AFX_G_PROP_H__9FCFD026_4554_11D2_87B4_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "G_Object.h"

class CG_Prop : public CG_Object  
{
public:
	CG_Prop();
	virtual ~CG_Prop();
   DECLARE_DYNAMIC( CG_Prop );

// Attributes
public:
   CFrameArray m_frames;

// Implementation
public:
protected:
	virtual BOOL AttachObjectProperties();

};

#endif // !defined(AFX_G_PROP_H__9FCFD026_4554_11D2_87B4_0080AD509054__INCLUDED_)
