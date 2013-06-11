#if !defined(AFX_CACHE_H__7ABB2D98_6EAB_CE67_66D0_0080AD509054__INCLUDED_)
#define AFX_CACHE_H__7ABB2D98_6EAB_CE67_66D0_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template< class TYPE >
class CG_ObjArray : public CTypedPtrArray< CObArray, TYPE >
{
public:
   TYPE Find( LPCTSTR Name )
   {
      ASSERT(AfxIsValidString(Name));
      CString tmp( Name );   
      tmp.MakeUpper();
      for( int i=0; i<GetSize(); i++ )
         if( GetAt(i)->m_sName.CompareNoCase(tmp)==0 ) return GetAt(i);
      return NULL;
   };
   TYPE Find( CG_ID ID )
   {
      for( int i=0; i<GetSize(); i++ )
         if( GetAt(i)->m_ID==ID ) return GetAt(i);
      return NULL;
   };
   void RemoveAll()
   {
      for( int i=0; i<GetSize(); i++ ) delete GetAt(i);
      CTypedPtrArray< CObArray, TYPE >::RemoveAll();
   };
};

template< class TYPE >
class CG_ObjMap : public CTypedPtrMap< CMapStringToOb, CString, TYPE >
{
public:
   TYPE Find( LPCTSTR Name )
   {
      ASSERT(AfxIsValidString(Name));
      TYPE item;
      CString tmp( Name );
      tmp.MakeUpper();
      if( !Lookup( tmp, item ) ) return NULL; else return item;
   };
   TYPE Find( CG_ID ID )
   {
      TYPE obj;
      POSITION pos = GetStartPosition();
      while( pos!=NULL ) {
         CString key;
         GetNextAssoc(pos,key,obj);
         if( obj->m_ID == ID ) return obj;
      };
      return NULL;
   }
   //
   CG_Room * FindRoom( LPCTSTR Name ) { return (CG_Room *)Find(Name); };
   CG_Room * FindRoom( CG_ID ID ) { return (CG_Room *)Find(ID); };
   CG_Prop * FindProp( LPCTSTR Name ) { return (CG_Prop *)Find(Name); };
   CG_Prop * FindProp( CG_ID ID ) { return (CG_Prop *)Find(ID); };
   CG_Item * FindItem( LPCTSTR Name ) { return (CG_Item *)Find(Name); };
   CG_Item * FindItem( CG_ID ID ) { return (CG_Item *)Find(ID); };
   CG_Character * FindPlayer( LPCTSTR Name ) { return (CG_Character *)Find(Name); };
   CG_Character * FindPlayer( CG_ID ID ) { return (CG_Character *)Find(ID); };
};



#endif // !defined(AFX_CACHE_H__7ABB2D98_6EAB_CE67_66D0_0080AD509054__INCLUDED_)

