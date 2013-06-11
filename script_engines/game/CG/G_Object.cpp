// G_Object.cpp: implementation of the CG_Object class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "G_Object.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CG_Object, CObject );

CG_Object::CPropertyMap *CG_Object::m_properties = NULL;

CG_Object::CG_Object()
{
   static int unqiue_id = 1;
   m_ID = unqiue_id++;
   for( int i=0; i<MAX_OPTIONS; i++ ) m_option[i]=0;
}

CG_Object::~CG_Object()
{
   if( m_properties!=NULL ) delete m_properties;
   m_properties = NULL;
}


//////////////////////////////////////////////////////////////////////
// Properties

BOOL CG_Object::AddProp(LPCTSTR Name, SYMBTYPE Type, LPVOID Addr)
{
   TProperty prop;
   CString tmp(Name);
   tmp.MakeUpper();
   prop.Type = Type;
   prop.nOffset = (LPBYTE)Addr - (LPBYTE)this;
   m_properties->SetAt(tmp,prop);
   return TRUE;
}

BOOL CG_Object::GetProp(LPCTSTR Name, LPVOID &Ptr, SYMBTYPE *Type/*=NULL*/)
{
   ASSERT(AfxIsValidString(Name));
   ASSERT(m_properties);
   TProperty prop;
   CString tmp(Name);
   tmp.MakeUpper();
   if( !m_properties->Lookup( tmp, prop ) ) return FALSE;
   Ptr = (LPBYTE)this + prop.nOffset;
   if( Type!=NULL ) *Type = prop.Type;
   return TRUE;
}

BOOL CG_Object::CreatePropertyList()
{
   if( m_properties==NULL ) {
      m_properties = new CPropertyMap;
   };
   return AttachObjectProperties();
}

BOOL CG_Object::AttachObjectProperties()
{
   AddProp( _T("ID"), VAR_LONG, &m_ID );
   AddProp( _T("NAME"), VAR_STRING, &m_sName );
   AddProp( _T("DESCRIPTION"), VAR_STRING, &m_sDescription );
   CString s;
   for( int i=0; i<MAX_OPTIONS; i++ ) {
      s.Format(_T("OPTION%d"),i+1);
      AddProp( s, VAR_LONG, &m_option[i] );
   };
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// 
