// G_Prop.cpp: implementation of the CG_Prop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "G_Prop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CG_Prop, CG_Object );

CG_Prop::CG_Prop()
{

}

CG_Prop::~CG_Prop()
{

}

//////////////////////////////////////////////////////////////////////
// Implementation

BOOL CG_Prop::AttachObjectProperties()
{
   CG_Object::AttachObjectProperties();
   return TRUE;
}
