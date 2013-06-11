// G_Object.h: interface for the CG_Object class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G_OBJECT_H__780689AB_3AB2_11D2_879D_0080AD509054__INCLUDED_)
#define AFX_G_OBJECT_H__780689AB_3AB2_11D2_879D_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef DWORD CG_TIME;
typedef DWORD CG_ID;
typedef CArray<CRect, CRect&> CFrameArray;

class CG_Object : public CObject  
{
public:
	CG_Object();
	virtual ~CG_Object();
   DECLARE_DYNAMIC( CG_Object );

// Types
public:
typedef struct {
   SYMBTYPE Type;
   long nOffset;
} TProperty;
typedef CMap< CString, LPCTSTR, TProperty, TProperty & > CPropertyMap;

// Attributes
public:
   CG_ID   m_ID;
   CString m_sName;
   CString m_sDescription;
   //
#define MAX_OPTIONS 5
   DWORD   m_option[MAX_OPTIONS];
   //
   CString m_sSurfaceName;
   CDirectSurface *m_surface;
   //
   static CPropertyMap *m_properties;

// Implementation
public:
   BOOL CreatePropertyList();
	BOOL GetProp( LPCTSTR Name, LPVOID &Ptr, SYMBTYPE *Type=NULL);
protected:
	virtual BOOL AttachObjectProperties();
	BOOL AddProp( LPCTSTR Name, SYMBTYPE Type, LPVOID Addr );
};

#endif // !defined(AFX_G_OBJECT_H__780689AB_3AB2_11D2_879D_0080AD509054__INCLUDED_)
