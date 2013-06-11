// ScriptParser.h: interface for the CScriptParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTPARSER_H__B0D12414_3C57_11D2_87A3_0080AD509054__INCLUDED_)
#define AFX_SCRIPTPARSER_H__B0D12414_3C57_11D2_87A3_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScriptParser : public CObject  
{
public:
	CScriptParser();
	virtual ~CScriptParser();

// Types
public:
   typedef struct {
      CString Filename;
      DWORD   LineNr;
      //
      CStdioFile file; // script file we're processing
      CString orgline; // original line as read from file
      CString line;    // parsed line
      BOOL bEOFLine;   // flag set if next token is EOL!
   } TScript;
   typedef enum { TT_EOF, TT_EOL, TT_CONSTANT, TT_STRING, TT_VALUE, TT_KEYWORD, TT_SEPERATOR } TOKENTYPE;

// Attributes
protected:
   CG_Controller *m_game;

// Methods
public:
	BOOL Validate(void);
	BOOL ProcessFile( LPCTSTR Filename );
	BOOL Init( CG_Controller *game );
	BOOL Close();

private:
	BOOL SCRIPTERROR( TScript *script, LPCTSTR Error, ... );
	BOOL SCRIPTERROR( TScript *script, short ErrCode );
   BOOL SCRIPTMESSAGE( BOOL bError, TScript *script, LPCTSTR szFormat, ... );
	//
	CString GetToken( TScript *script, TOKENTYPE &type );
	CString GetToken( TScript *script, TOKENTYPE &type, TOKENTYPE match );
	CPoint GetTokenPos( TScript *script );
	CRect GetTokenRect( TScript *script );
	BOOL ProcessScript( TScript *script );
	BOOL GetLine( TScript *scr, CString &s );
   //
   BOOL ParseInclude( TScript *script );
	BOOL ParseProps( TScript *script );
	BOOL ParseRoom( TScript *script );
	BOOL ParseCharacter( TScript *script );
	//
   BOOL ParseRoomObject( TScript *script, CG_Room *room, short type );
	BOOL ParseRoomExit( TScript *script, CG_Room *room );
};

#endif // !defined(AFX_SCRIPTPARSER_H__B0D12414_3C57_11D2_87A3_0080AD509054__INCLUDED_)
