//
//
//

#if !defined(AFX_SCRIPT_H__836BD724_3F8A_11D2_87A7_0080AD509054__INCLUDED_)
#define AFX_SCRIPT_H__836BD724_3F8A_11D2_87A7_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScriptParser : public CObject  
{
public:
	CScriptParser();
	virtual ~CScriptParser();

// Attributes
public:
   CStdioFile *f_ptr;
   CString m_sFilename;
   char m_UngetChar;
   BOOL m_bErrors;
   //
   CController *m_pGame;
   CG_Object *m_pObj;
   CG_Room *m_room;
   CG_Hero *m_hero;
   CG_Character *m_player;
   CG_Item *m_item;
   CG_Prop *m_prop;
   //
   CPoint m_pos;
   CRect m_rect;
   CString m_sSurfaceName;
   CFrameArray m_frames;
   CG_Room::TRoomExit m_roomexit;
   CG_Room::TRoomObject m_roomobj;
   long m_framerepeat;


// Implementation
public:
   BOOL Init( CController *pGame );
   BOOL Close();
   BOOL Validate();
   BOOL ProcessFile( LPCTSTR Filename );
};


#endif // !defined(AFX_COMPILER_H__836BD724_3F8A_11D2_87A7_0080AD509054__INCLUDED_)
