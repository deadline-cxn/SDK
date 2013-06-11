// Compiler.h: interface for the CCompiler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPILER_H__836BD724_3F8A_11D2_87A7_0080AD509054__INCLUDED_)
#define AFX_COMPILER_H__836BD724_3F8A_11D2_87A7_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable : 4200 )


class CFileParser : public CObject
// This class is used as a base class for both the
// .GC code compiler and the STATE script file compiler.
// It still includes a lot of nasty types and declares from
// both classes, so it cannot be reused much.
{
public:
	CFileParser();
	virtual ~CFileParser();

// Types
public:
   // Tokens (must match 'tokentypes' in cpp file
   typedef enum { 
      T_UNKNOWN,T_STRING_CONST,T_NUMERIC_CONST,T_NAME,T_TYPE,T_OBJECT,T_DIRECTIVE,
      T_WHILE,T_IF,T_ELSE,T_DO,T_FOR,
      T_BEGIN,T_END,T_LPAREN,T_RPAREN,T_LBRACK,T_RBRACK,
      T_SEPARATOR,T_PERIOD,T_COMMA,
      T_PLUS,T_MIN,T_MUL,T_DIV,T_MOD,T_POW,
      T_BOOL_AND,T_BOOL_OR, T_AND, T_OR,
      T_ASSIGN,T_COLON,
      T_LINECOMMENT,T_COMMENT,
      T_EQUAL,T_GREATER,T_LESS,T_LESS_EQUAL,T_GREATER_EQUAL,T_NOT_EQUAL,
      T_PLUSPLUS,T_MINMIN,
      T_CONST,T_THIS,T_RETURN,T_DATASEG,
      T_EXIT,T_ASM,T_INCLUDE,
      T_EOF,T_EVENT,T_PROCEDURE,T_ENDL,
      T_LASTTOKEN 
   } TOKENTYPE;
#define token_num CFileParser::T_LASTTOKEN
   // Function definitions
   typedef struct {
      LPCTSTR Name;
      SYMBTYPE ReturnType;
      SYMBTYPE Arguments[5];
   } TFuncDef;
   // Code structure
   typedef struct {
      CString Filename;
      DWORD   LineNr;
      DWORD   TotalLines;
      //
      CStdioFile file; // script file we're processing
      CString orgline; // original line as read from file
      CString line;    // parsed line
      BOOL bEOL;       // flag set if next token is EOL!
      union {
         SYMBTYPE symbol;
         EVENTTYPE event;
         int funcidx;
         CG_Object *obj;
      };
   } TCode;

// Attributes
protected:
   CController *m_game;
   BOOL         m_bErrors;
   CString      m_sClassIdea;

// Implementation
public:
   virtual BOOL ProcessFile( LPCTSTR Filename );
protected:
	virtual BOOL Init( CController *game );
	virtual BOOL Close();
   virtual BOOL ProcessScript( TCode *script ) { return FALSE; };
   //
   BOOL SCRIPTMESSAGE( BOOL bError, TCode *script, LPCTSTR szFormat, ... );
	BOOL SCRIPTERROR( TCode *script, LPCTSTR Error, ... );
	BOOL SCRIPTERROR( TCode *script, short ErrCode );
	//
	BOOL GetLine( TCode *scr, CString &s );
	CString GetToken( TCode *script, TOKENTYPE &type );
   CString MatchToken( TCode *script, TOKENTYPE type, TOKENTYPE &curtype );
	CString GetTokenName( TCode *script, CString &tok, TOKENTYPE &type);
	long GetTokenNumber( TCode *script, CString &tok, TOKENTYPE &type );
};


class CLogicFile;


class CCompiler : public CFileParser
{
friend CLogicFile;
public:
	CCompiler();
	virtual ~CCompiler();

// Types
protected:
   // Symbol definition
   typedef struct {
      CString Name;
      SYMBTYPE Type;
      union {
         short NumElements;
         short NumArguments;
      };
      long Size;
      long DataPtr;
   } TSymbol;
   typedef CMap< CString, LPCTSTR, TSymbol, TSymbol& > CSymbolMap;
   // Label definition
   typedef struct {
      CString Name;
      CODEPTR ptr;
      CODEPTR refs[30];
      short nrrefs;
   } TLabel;
   typedef CMap< CString, LPCTSTR, TLabel, TLabel& > CLabelMap;
   // Internal array of function descriptions
   typedef CArray< TEventMap, TEventMap& > CEventMapArray;

// Attributes
protected:
   CEventMapArray m_eventmaps;
   CDWordArray m_code;
   CDWordArray m_data;
   CSymbolMap m_symbols;
   CLabelMap m_labels;

// Implementation
public:
	BOOL Init( CController *game );
	BOOL Close();
   BOOL Save( LPCTSTR Filename );
protected:
   BOOL ProcessScript( TCode *script );
   BOOL ParseCode( TCode *script, CString &tok, TOKENTYPE &type );
	BOOL ParseDeclares( TCode *script, CString &tok, TOKENTYPE &type );
   //
	DATAPTR save_string_const( LPCTSTR str );
	CString newlabel( TCode *script );
	BOOL addlabelref( TCode *script, LPCTSTR Label );
	BOOL labellink();
	//
   BOOL emitcode( TCode *script, EMITCODE opcode, long arg1=0, LPCTSTR arg2=NULL );
   //
   BOOL iscompareop(TOKENTYPE x) const;
   BOOL isaddop(TOKENTYPE x) const;
   BOOL isboolop(TOKENTYPE x) const;
   BOOL ismulop(TOKENTYPE x) const;
   //
   void subtract(TCode *script, CString &tok, TOKENTYPE &type);
   void add(TCode *script, CString &tok, TOKENTYPE &type);
   void divide(TCode *script, CString &tok, TOKENTYPE &type);
   void multiply(TCode *script, CString &tok, TOKENTYPE &type);
   void bool_and(TCode *script, CString &tok, TOKENTYPE &type);
   void bool_or(TCode *script, CString &tok, TOKENTYPE &type);
	//
   SYMBTYPE findsymbol( TCode *script, LPCTSTR name, TSymbol &symbol );
	BOOL addsymbol( TCode *script, LPCTSTR name, SYMBTYPE type, long arg1, long arg2 );
	//
   SYMBTYPE procedure_code(TCode *script, CString &tok, TOKENTYPE &type);
	BOOL assignment( TCode *script, CString &tok, TOKENTYPE &type );
	//
   SYMBTYPE expression(TCode *script, CString &tok, TOKENTYPE &type);
	BOOL blockstatement( TCode *script, CString &tok, TOKENTYPE &type );
   BOOL statement(TCode *script, CString &tok, TOKENTYPE &type);
	SYMBTYPE factor( TCode *script, CString &tok, TOKENTYPE &type );
   SYMBTYPE property(TCode *script, CG_Object *obj, CString &tok, TOKENTYPE &type);
	SYMBTYPE value( TCode *script, CString &tok, TOKENTYPE &type );
   SYMBTYPE term(TCode *script, CString &tok, TOKENTYPE &type);
	BOOL do_while_loop(TCode *script, CString &tok, TOKENTYPE &type);
	BOOL if_then_else( TCode *script, CString &tok, TOKENTYPE &type );
   BOOL while_loop(TCode *script, CString &tok, TOKENTYPE &type);
   BOOL c_for_loop(TCode *script, CString &tok, TOKENTYPE &type);
   //
   CODEPTR GetCodePtr() const { return m_code.GetSize(); };
   DATAPTR GetDataPtr() const { return m_data.GetSize(); };
};


class CStateCompiler : public CFileParser
{
public:
	CStateCompiler();
	virtual ~CStateCompiler();

// Types
protected:

// Attributes
protected:
   CController *m_pGame;

// Implementation
public:
	BOOL Init( CController *game );
	BOOL Close();
protected:
   BOOL ProcessScript( TCode *script );
   CString ParseState( TCode *script, CString &tok, TOKENTYPE &type );
};



class CLogicFile : public CObject
// This is a small class which deals with
// writing/reading the compiled code on disk
{
public:
	CLogicFile();
	virtual ~CLogicFile();

// Types
public:
#define CODEFILE_ID 0x66667777   // the file header ID
#define CODEFILE_VER 1           // a file version marker
   typedef struct {
      DWORD ID;
      short Version;
      //
      long NumEventMaps;
      long EventMapSize;
      long CodeSize;
      long DataSize;
   } TLogicFileHeader;

// Implementation
public:
   BOOL Load(LPCTSTR Filename, TEventMap *&eventmaps, DWORD &eventsize, CODEPTR *&code, DWORD &codesize, DATAPTR *&data, DWORD &datasize );
   BOOL Save(LPCTSTR Filename, CCompiler::CEventMapArray &eventmaps, CDWordArray &code, CDWordArray &data );
};


#endif // !defined(AFX_COMPILER_H__836BD724_3F8A_11D2_87A7_0080AD509054__INCLUDED_)

