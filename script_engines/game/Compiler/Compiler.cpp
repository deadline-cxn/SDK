// Compiler.cpp: implementation of the CCompiler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Compiler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////
// Types

static LPCTSTR tokennames[] = {
	"","","","","","","",
	"WHILE","IF","ELSE","DO","FOR", //program flow tokens
	"{","}","(",")","[","]",        //compound tokens
	";",".",",",                    //separators
	"+","-","*","/","%","^",        //operators
	"&&","||","&","|",
	"=",":",
	"//","/*",                      //comment
	"==",">","<","<=",">=","!=",    //logical
	"++","--",
	"CONST","THIS","RETURN","DATASEG",
	"END","","INCLUDE",
	"","","","ENDL",
   NULL };

static LPCTSTR eventnames[] = {
   _T("INIT"),
   _T("DESTROY"),
   _T("SPAWN"),
   _T("DIE"),
   //
   _T("TICK"),
   _T("TOUCH"),
   _T("NEAR"),
   _T("ENTERROOM"),
   _T("EXITROOM"),
   //
   _T("EXAMINE"),
   _T("TAKE"),
   _T("GIVE"),
   _T("DROP"),
   _T("HIT"),
   _T("USE"),
   _T("TALK"),
   NULL
};

static LPCTSTR varnames[] = {
   _T(""),
   _T("VOID"),
   _T("INT"),
   _T("STRING"),
   _T("FLOAT"),
   NULL
};

static const CFileParser::TFuncDef funcs[] = {
   { _T("PLAYSOUND"), VAR_UNKNOWN, {VAR_STRING,VAR_LAST} }, //0
   { _T("INT2STR"), VAR_STRING, {VAR_LONG,VAR_LAST} }, //1
   { _T("STR2INT"), VAR_LONG, {VAR_STRING,VAR_LAST} }, //2
   { _T("LOG"), VAR_UNKNOWN, {VAR_STRING,VAR_LAST} }, //3
   { _T("POSTMESSAGE"), VAR_UNKNOWN, {VAR_LONG,VAR_LONG,VAR_LONG,VAR_LAST} }, //4
   { _T("SLEEP"), VAR_UNKNOWN, {VAR_LONG,VAR_LAST} }, //5
   { _T("UCASE"), VAR_STRING, {VAR_STRING,VAR_LAST} }, //6
   { _T("LCASE"), VAR_STRING, {VAR_STRING,VAR_LAST} }, //7
   { _T("LEFT"), VAR_STRING, {VAR_STRING,VAR_LAST} }, //8
   { _T("MID"), VAR_STRING, {VAR_STRING,VAR_LONG,VAR_LAST} }, //9
   { _T("RIGHT"), VAR_STRING, {VAR_STRING,VAR_LAST} }, //10
   { _T("SETTIMER"), VAR_UNKNOWN, {VAR_LONG,VAR_LAST} }, //11
   { _T("SWITCHSTATE"), VAR_UNKNOWN, {VAR_LONG,VAR_LAST} }, //12
   { _T("SENDER"), VAR_OBJECT, {VAR_LAST} }, //13
   { NULL, VAR_UNKNOWN, {VAR_UNKNOWN} }
};


//////////////////////////////////////////////////////////////////////
//
// Implementation of the CFileParser class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileParser::CFileParser()
{

}

CFileParser::~CFileParser()
{
   Close();
}

BOOL CFileParser::Init(CController *game)
{
   ASSERT_VALID( game );
   if( game==NULL ) return FALSE;
   m_game = game;
   m_bErrors = FALSE;
   return TRUE;
}

BOOL CFileParser::Close()
{
   return TRUE;
}

BOOL CFileParser::SCRIPTMESSAGE(BOOL bError, TCode *script, LPCTSTR szFormat, ...)
{
   ASSERT( script );
   CString str;
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);
   va_end(args);
   if( bError ) 
      return DEBUGERROR(str);
   else
      return DEBUGLOG(str);
}

BOOL CFileParser::SCRIPTERROR(TCode *script, LPCTSTR szFormat, ...)
{
   ASSERT( script );
   CString msg, str;
   msg.Format("File %s line %d: ", script->Filename,script->LineNr );
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);
   va_end(args);
   msg += str;
   SCRIPTMESSAGE(TRUE,script,msg);
   SCRIPTMESSAGE(TRUE,script,_T("Descr: \"%s\""),(LPCTSTR)script->orgline);
   ASSERT(FALSE);
   m_bErrors = TRUE;
   return FALSE;
}

BOOL CFileParser::SCRIPTERROR(TCode *script, short ErrCode )
{
   static LPCTSTR ErrMsgs[] = {
      _T("Syntax Error"), //0
      _T("Constant expected"), //1
      _T("Unexpected keyword in block"), //2
      _T("Unexpected end of file"), //3
      _T("Unexpected string in block"), //4
      _T("Error loading picture"), //5
      _T("Out of memory"), //6
      _T("Unexpected type of argument"), //7
      _T("Unable to open script file"), //8
      _T("Include failed"), //9
      _T("Unknown keyword in block"), //10
      _T("Object not found"), //11
      _T("Type mismatch"), //12
      _T("Unexpected token"), //13
      NULL };
   LPCTSTR *p;
   ASSERT( script );
   p = ErrMsgs;
   while( *p!=NULL ) {
      if( ErrCode<=0 ) break;
      ErrCode--;
      p++;
   };
   if( *p==NULL ) p=ErrMsgs; // error: msg not found - use default...
   return SCRIPTERROR(script,*p);
}


//////////////////////////////////////////////////////////////////////
// Process controllers

BOOL CFileParser::ProcessFile(LPCTSTR Filename)
// Top level functions which opens the script file
// and calls the process function...
{
   CApp *app = (CApp *)AfxGetApp();
   app->m_debug.PrintReplaceLine(_T("Compiling %s"), Filename);

   TCode script;
   TRY
	{	
      script.Filename = Filename;
      script.LineNr = 0;
      script.TotalLines = 0;
      script.bEOL = TRUE;    // ready to read 1st line
      //
      if( !script.file.Open( Filename, CFile::modeRead|CFile::typeText ) ) return SCRIPTERROR(&script,8);
      ProcessScript( &script );
      script.file.Abort();
	}
	CATCH( CFileException, e )
	{
		TCHAR err[128];
      e->GetErrorMessage(err,sizeof(err));
      SCRIPTERROR(&script,err);
      script.file.Abort();
      return FALSE;
	}
	END_CATCH;
   app->m_debug.PrintReplaceLine(_T("%s compiled."),m_sClassIdea);
   return (m_bErrors==FALSE);
}


//////////////////////////////////////////////////////////////////////
// Token controllers

BOOL CFileParser::GetLine(TCode *script, CString &s)
// A script file helper function which reads one
// line from the text file...
// The function will automatically trim and
// remove comments from the line read.
{
   TRY
	{	
      if( !script->file.ReadString(s) ) {
         script->file.Abort();
         s.Empty();
         return FALSE;
      };
      script->orgline = s;  // save the original line
      script->LineNr++;     // count of current lines
      script->TotalLines++; // count of total compiled lines
	}
	CATCH( CFileException, e )
	{
		// error: file error
#ifdef _DEBUG
      e->ReportError();
#endif
      script->file.Abort();
      return FALSE;
	}
	END_CATCH;
   // Trim string
   s.TrimLeft();
   // Look for any standard comment and remove
   long pos = s.Find(_T("//"));
   if( pos>=0 ) s = s.Left(pos);
   // More trim...
   s.TrimRight();
   script->line = s;
   return TRUE;
}

CString CFileParser::GetToken(TCode *script, TOKENTYPE &type)
// LEX function which parses a line read from the
// script file and examines the next sequence of characters,
// looking for well-defined tokens.
// The function recognizes numbers, literals (strings),
// compiler keywords, defined constants, defined
// function names and compiler event maps.
{
   ASSERT(script);
grab_token:;
   type = T_UNKNOWN;
   if( script->bEOL ) {
      if( !GetLine( script, script->line ) ) {
         // EOF met.
         // We must make sure to keep sending this!!!
         type = T_EOF;
         return CString();
      };
   };
   // Empty lines are skipped...
   script->line.TrimLeft();
   if( script->line.IsEmpty() ) {
      script->bEOL = TRUE;
      goto grab_token;
   };
   script->bEOL = FALSE; // This line is not empty anymore!

   CString newtoken;
   int pos = 0;
   int len = script->line.GetLength();
   DWORD val;
   TCHAR c = script->line[0];

   if( _istdigit(c) ) {
      while( len>0 ) {
         c = script->line[pos];
         if( !_istdigit(c) ) break;
         newtoken = newtoken + c;
         pos++;
         len--;
      };
      type = T_NUMERIC_CONST;
      goto got_token;
   };
   
   if( c==_T('"') ) {
      pos++;
      len--;
      while( len>0 ) {
         c = script->line[pos];
         if( c==_T('"') ) break;
         newtoken = newtoken + c;
         pos++;
         len--;
      };
      pos++;
      len--;
      type = T_STRING_CONST;
      goto got_token;
   };
   
   // At last get string (might be a keyword or something else though)
   type = T_NAME;
   c = script->line[pos];
   if( _istalnum(c) ) {
      while( len>0 ) {
         c = script->line[pos];
         if( !_istalnum(c) ) break;
         newtoken = newtoken + c;
         pos++;
         len--;
      };
   }
   else {
      while( len>0 ) {
         c = script->line[pos];
         if( _istalnum(c) ) break;
         if( !_istgraph(c) ) break;
         if( !newtoken.IsEmpty() ) {
            switch(c) {
            case _T('{'):
            case _T('}'):
            case _T('('):
            case _T(')'):
            case _T(';'):
            case _T('"'):
               goto break_token;
            };
         };
         newtoken = newtoken + c;
         pos++;
         len--;
      };
break_token: ;
   };

   {
      LPCTSTR *p; 
      int i;
      CString tmp( newtoken );
      tmp.MakeUpper();

      // Here we check for all known keywords
      // We make the seach case-insensitive (by making a temporary string
      // which is uppercased)
      p = tokennames;
      i=0;
      while( *p!=NULL ) {
         if( tmp==*p ) {
            type = (TOKENTYPE)i;
            goto got_token;
         };
         p++;
         i++;
      };

      // Here we check for all known symbol types
      p = varnames;
      i=0;
      while( *p!=NULL ) {
         if( tmp==*p ) {
            type = T_TYPE;
            script->symbol = (SYMBTYPE)i;
            goto got_token;
         };
         p++;
         i++;
      };
      
      // Here we check for all known functions
      const TFuncDef *pFunc;
      pFunc = funcs;
      i=0;
      while( pFunc->Name!=NULL ) {
         if( tmp==pFunc->Name ) {
            script->funcidx = i;
            type = T_PROCEDURE;
            goto got_token;
         };
         pFunc++;
         i++;
      };

      // Check for defined constants
      if( m_game->GetConstant(tmp,val) ) {
         newtoken.Format(_T("%d"),val);
         type = T_NUMERIC_CONST;
         goto got_token;
      };

      // Check for known objects
      if( m_game->m_objects.Lookup(tmp,script->obj)==TRUE) {
         type = T_OBJECT;
         goto got_token;
      };
      
      // Here we check for all known events
      p = eventnames;
      i=0;
      while( *p!=NULL ) {
         if( tmp==*p ) {
            type = T_EVENT;
            script->event = (EVENTTYPE)i;
            goto got_token;
         };
         p++;
         i++;
      };

      if( newtoken==_T("#") ) type = T_DIRECTIVE;

   };

got_token:
   script->line = script->line.Mid( pos );
   return newtoken;
}

CString CFileParser::GetTokenName(TCode *script, CString &tok, TOKENTYPE &type)
// Special version of the GetToken() funciton which tests
// that the new token read is of a string type.
{
   ASSERT( script );
   CString str( tok );
   if (type != T_NAME)
      return SCRIPTERROR(script,"Expected Name");
   tok = GetToken(script,type);
   str.MakeUpper();
   return str;
}

long CFileParser::GetTokenNumber(TCode *script, CString &tok, TOKENTYPE &type)
// Special version of the GetToken() funciton which tests
// that the new token read is a number.
{
  long i;
  i = _ttol(tok);
  tok = GetToken(script,type);
  return i;
}

CString CFileParser::MatchToken(TCode *script, TOKENTYPE type, TOKENTYPE &curtype)
// Function which tests that the current token is
// of a specific type.
{
   ASSERT(type>=0 && type<token_num);
   ASSERT(curtype>=0 && curtype<token_num);
   if( type!=curtype ) {
      if( type<token_num )
         SCRIPTERROR(script,_T("Unexpected token. Token '%s' expected"),tokennames[type] );
      else
         SCRIPTERROR(script,13);
      return CString();
   };
   CString str = GetToken(script,curtype);
   return str;
};




//////////////////////////////////////////////////////////////////////
//
// CCompiler.cpp: implementation of the CCompiler class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompiler::CCompiler()
{
   m_sClassIdea = _T("Code");
}

CCompiler::~CCompiler()
{
   Close();
}


//////////////////////////////////////////////////////////////////////
// Process controllers

BOOL CCompiler::Init(CController *game)
{
   if( !CFileParser::Init(game) ) return FALSE;
   m_code.SetSize(0,200);
   m_data.SetSize(0,200);
   return TRUE;
}

BOOL CCompiler::Close()
{
   if( !CFileParser::Close() ) return FALSE;
   m_code.RemoveAll();
   m_data.RemoveAll();
   m_symbols.RemoveAll();
   m_eventmaps.RemoveAll();
   return TRUE;
}

BOOL CCompiler::Save( LPCTSTR Filename )
// Saved the compiled stuff to a file
// This is called a "logical file"
{
   CLogicFile f;
   return f.Save(Filename,m_eventmaps,m_code,m_data);
};


//////////////////////////////////////////////////////////////////////
// Script controllers

BOOL CCompiler::ProcessScript( TCode *script )
// Top level parsing...
// The function parses over the script section defintion, usually
// formatted as:
//
// BEGIN SCRIPT
// TITLE "xxx"
// #event
//    <code>
// END SCRIPT
//
// No p-code is generated from these first script constructs
// but maintains the structure of the eventmaps and variable
// declare section, and as soon as code is recognized, the
// ParseCode() function is called.
{
   ASSERT(script);
   CString tok;
   TOKENTYPE type;
   while( TRUE ) {
      tok = GetToken(script,type);
      if( type==T_EOF ) break;
      if( tok!=_T("BEGIN") ) continue;

      tok = GetToken(script,type);
      if( tok!=_T("SCRIPT") ) continue;
      
      // Intialize event map
      TEventMap eventmap;
      ::ZeroMemory(&eventmap,sizeof(eventmap));
      m_symbols.RemoveAll();
      
      // Here we go...
      tok = GetToken(script,type);
      while (TRUE) {
         // The start of the WHILE loop requires a new token
         // to be already parsed...
         if( type==T_EXIT ) {
            if( GetToken(script,type)==_T("SCRIPT") ) break;
         }
         else if( type==T_NAME ) {
            if( tok==_T("TITLE") ) {
               tok = GetToken(script,type);
               _tcscpy( eventmap.Name, tok );
               _tcsupr( eventmap.Name );
            };
         }
         else if( type==T_DIRECTIVE ) {
            tok = GetToken(script,type);
            switch( type ) {
            case T_NAME:
               if( tok==_T("DECLARES") ) {
                  ParseDeclares(script,tok,type);
               }
               else if( tok==_T("INCLUDE") ) {
                  tok = GetToken(script,type);
                  CString Filename = BfxGetFilePath(script->Filename);
                  Filename += tok;
                  BOOL ok = ProcessFile( Filename );                  
               };
               break;
            case T_EVENT:
               // Initialize
               eventmap.Events[eventmap.NumEvents] = script->event;
               eventmap.EventPtrs[eventmap.NumEvents] = GetCodePtr();
               m_labels.RemoveAll(); // labels are local
               // Parse scripting code
               ParseCode(script,tok,type);
               // Done
               labellink();
               m_code.Add(CODE_PROGRAMEXIT);
               eventmap.NumEvents++;
               break;
            };
            continue;
         };
         tok = GetToken(script,type);
      }; // inner WHILE(TRUE)
      m_code.Add(CODE_PROGRAMEXIT);
      m_eventmaps.Add(eventmap);
   }; // WHILE(TRUE)

   return TRUE;
};

BOOL CCompiler::ParseDeclares(TCode *script, CString &tok, TOKENTYPE &type)
// A script declare section has been found.  This function
// parses the section, creating the symbols in the symbol table.
// The helper function addsymbol() takes care of the internal
// adding of a symbol.
{
   ASSERT(script);
   CString vartype,varname;
   while( TRUE ) {
      tok = GetToken(script,type);
      if( type!=T_TYPE ) return TRUE;
      vartype = tok;
      varname = GetToken(script,type);
      tok = GetToken(script,type);
      addsymbol(script,varname,script->symbol,0,0);
   };
   return FALSE;
}

BOOL CCompiler::ParseCode(TCode *script, CString &tok, TOKENTYPE &type)
// Scripting code is about to begin.  The top level code parser function.
{
   ASSERT(script);
   BOOL ok = TRUE;
   tok = GetToken(script,type);
   while ( ok ) {
      if (type == T_SEPARATOR)
         tok = GetToken(script,type);
      else
         ok = statement(script,tok,type);
   }
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Code helpers

CString CCompiler::newlabel( TCode *script )
// Function which creates a new label.
// Labels cannot be defined (the GOTO construct isn't supported), but
// label names are automatically assigned. The creation of a label
// doesn't really generate any code, but we must make a note of
// the current position, since we will later fix-up references to
// the label.
{
   ASSERT(script);
   static long labcnt = 1;
   CString sName;
   sName.Format("L%08d",labcnt++);

   TLabel l;
   // Make sure we don't have duplicates
   if( m_labels.Lookup(sName,l) ) {
      SCRIPTERROR(script,_T("Label '%s' already defined"),(LPCTSTR)sName);
      return CString();
   };
   // Add new label
   TLabel label;
   label.Name = sName;
   ::FillMemory( label.refs, sizeof(label.refs), 0xFF );
   label.nrrefs = 0;
   m_labels.SetAt(sName,label);

   return sName;
}

BOOL CCompiler::addlabelref(TCode *script, LPCTSTR sName)
// Add a reference to a label.
// The labellink() function (which is called after all code parsing)
// will fix-up the actual opcode for the jump.
{
   ASSERT(script);
   ASSERT(sName);
   TLabel label;
   // Make sure that the label exists
   if( !m_labels.Lookup(sName,label) ) {
      SCRIPTERROR(script,_T("Label not found"));
      return FALSE;
   };
   if( label.nrrefs>=sizeof(label.refs)/sizeof(CODEPTR) ) {
      SCRIPTERROR(script,_T("Label referenced too many times"));
      return FALSE;
   };
   label.refs[ label.nrrefs ] = GetCodePtr();
   label.nrrefs++;
   m_labels.SetAt(sName,label);
   return TRUE;
}

BOOL CCompiler::labellink()
// This function make the code-fixups for all jumps
// to all the labels.
// At the time the label-jump code was generated, it
// may not have been known where the jump would go to (the
// label may not have been created at that point), so we
// must fix the jump-to code pointers.
{
   CString sKey;
   TLabel label;
   POSITION pos = m_labels.GetStartPosition();
   while( pos!=NULL ) {
      m_labels.GetNextAssoc( pos, sKey, label );
      for( int i=0; i<label.nrrefs; i++ ) {
         m_code[ label.refs[i] ] = label.ptr;
      };
   };
   return TRUE;
}

DATAPTR CCompiler::save_string_const(LPCTSTR str)
// Saves a interal string to the data segment.
{
   DATAPTR p = GetDataPtr();
   int len = (_tcslen(str)*sizeof(TCHAR))/sizeof(DATAPTR);;
   len++;
   for( int i=0; i<len; i++ ) m_data.Add(0);
   _tcscpy( (LPTSTR)&m_data[p], str );
   return p;
}

SYMBTYPE CCompiler::procedure_code(TCode *script, CString &tok, TOKENTYPE &type)
// Parse a function call.
{
   const TFuncDef *func;
   int funcidx;
   CString varname;
   SYMBTYPE kind;
   funcidx = script->funcidx;
   func = &funcs[funcidx];
   const SYMBTYPE *argtypes = (SYMBTYPE *)func->Arguments;
   varname = tok;
   MatchToken(script,T_PROCEDURE,type);
   tok = MatchToken(script,T_LPAREN,type);
   int args = 0;
   while( type!= T_RPAREN ) {
      kind = expression(script,tok,type);
      emitcode(script,CODE_PUSH,kind); //push parameter
      if( *argtypes++ != kind ) {
         SCRIPTERROR(script,_T("Type mismatch or invalid arguments in function call"));
      };
      args++;
      if( type==T_COMMA ) tok = GetToken(script,type);
   };
   tok = MatchToken(script,T_RPAREN,type);
   emitcode(script,CODE_CALL,funcidx);
   emitcode(script,CODE_STACKSUB,args);
   return func->ReturnType;
}

SYMBTYPE CCompiler::findsymbol(TCode *script, LPCTSTR name, TSymbol &symbol)
{
   ASSERT(script);
   ASSERT(AfxIsValidString(name));
   if( !m_symbols.Lookup(name,symbol) ) return VAR_UNKNOWN;
   return symbol.Type;
}

BOOL CCompiler::addsymbol(TCode *script, LPCTSTR name, SYMBTYPE type, long arg1, long arg2)
{
   ASSERT(script);
   ASSERT(AfxIsValidString(name));
   TSymbol s;
   if( m_symbols.Lookup(name,s) ) {
      SCRIPTERROR(script,_T("Variable '%s' already defined"),(LPCTSTR)name);
      return FALSE;
   };
   // Add new symbol
   TSymbol symbol;
   CString sName( name );
   sName.MakeUpper();
   symbol.Name = name;
   symbol.Type = type;
   symbol.DataPtr = GetDataPtr();
   m_symbols.SetAt(sName,symbol);
   // Add memory to data segment
   m_data.Add(0);
   return TRUE;
}

BOOL CCompiler::emitcode(TCode *script, EMITCODE opcode, long arg1/*=0*/, LPCTSTR arg2/*=NULL*/)
// Function which generates the actual p-code.
{
   ASSERT( script );
   TSymbol symbol;

   // Some opcode does not need to be added to the code-segment
   // Instead we do some internal processing...
   switch( opcode ) {
   case CODE_PUTLABEL:
      {  // This assigns the actual code-position to the label
         ASSERT(arg2);
         TLabel label;
         if( !m_labels.Lookup(arg2,label) ) return SCRIPTERROR(script,_T("Label not found"));
         label.ptr = GetCodePtr();
         m_labels.SetAt(arg2,label);
      };
      return TRUE;
   };

   // Add opcode
   m_code.Add(opcode);
   
   // Some opcode may require some additional stuff
   switch( opcode ) {
   case CODE_CALL:
      m_code.Add(arg1);
      break;
   case CODE_STACKSUB:
      m_code.Add(arg1);
      break;

   case CODE_LOADCONST:
   case CODE_LOADOFFSET:
   case CODE_LOADOBJECT:
   case CODE_LOADPROP:
      m_code.Add(arg1);
      break;
   case CODE_LOADVAR:
      ASSERT( arg2 );
      findsymbol(script,arg2,symbol);
      m_code.Add(symbol.Type);
      m_code.Add(symbol.DataPtr);
      break;

   case CODE_POPADD:
   case CODE_POPSUB:
   case CODE_POPMUL:
   case CODE_POPDIV:
      m_code.Add(arg1); // symbol type
      break;
   case CODE_PUSH:
      m_code.Add(arg1); // symbol type
      break;

   case CODE_EQUAL:
   case CODE_EQUAL_NOT:
   case CODE_GREATER:
   case CODE_LESS:
   case CODE_EQUAL_GREATER:
   case CODE_EQUAL_LESS:
      m_code.Add(arg1); // symbol type
      break;

   case CODE_STORE:
      ASSERT(arg2);
      findsymbol(script,arg2,symbol);
      m_code.Add(symbol.Type);
      m_code.Add(symbol.DataPtr);
      break;
   case CODE_STOREPOINTER:
      ASSERT(arg2);
      findsymbol(script,arg2,symbol);
      m_code.Add(symbol.Type);
      m_code.Add(symbol.DataPtr);
      break;

   case CODE_INCVAR:
   case CODE_DECVAR:
      ASSERT(arg2);
      findsymbol(script,arg2,symbol);
      switch (symbol.Type) {
      case VAR_LONG:
         break;
      default:
         return SCRIPTERROR( script, 12 ); // type mismatch
      };
      m_code.Add(symbol.Type);
      m_code.Add(symbol.DataPtr);
      break;

   case CODE_JUMPTO:
   case CODE_IFJUMPTO:
   case CODE_IFNOTJUMPTO:
      ASSERT(arg2);
      addlabelref(script,arg2);
      m_code.Add(0); // here goes jump-pos when linked
      break;
   }; // SWITCH
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Code parsers

BOOL CCompiler::iscompareop(TOKENTYPE x) const
{
   return ((x >= T_EQUAL) && (x <= T_NOT_EQUAL));
}

BOOL CCompiler::isaddop(TOKENTYPE x) const
{
   return ((x >= T_PLUS) && (x <= T_MIN));
}

BOOL CCompiler::isboolop(TOKENTYPE x) const
{
   return ((x >= T_BOOL_AND) && (x <= T_BOOL_OR));
}

BOOL CCompiler::ismulop(TOKENTYPE x) const
{
   return ((x >= T_MUL) && (x <= T_DIV));
}


//////////////////////////////////////////////////////////////////////
// Expression helpers

void CCompiler::multiply(TCode *script, CString &tok, TOKENTYPE &type)
{
   SYMBTYPE kind;
   tok = MatchToken(script,T_MUL,type);
   kind = factor(script,tok,type);
   emitcode(script,CODE_POPMUL,kind);
}

void CCompiler::divide(TCode *script, CString &tok, TOKENTYPE &type)
{
   SYMBTYPE kind;
   tok = MatchToken(script,T_DIV,type);
   kind = factor(script,tok,type);
   emitcode(script,CODE_POPDIV,kind);
}

void CCompiler::add(TCode *script, CString &tok, TOKENTYPE &type)
{
   SYMBTYPE kind;
   tok = MatchToken(script,T_PLUS,type);
   kind = term(script,tok,type);
   emitcode(script,CODE_POPADD,kind);
}

void CCompiler::subtract(TCode *script, CString &tok, TOKENTYPE &type)
{
   SYMBTYPE kind;
   tok = MatchToken(script,T_MIN,type);
   kind = term(script,tok,type);
   emitcode(script,CODE_POPSUB,kind);
}

void CCompiler::bool_and(TCode *script, CString &tok, TOKENTYPE &type)
{
   SYMBTYPE kind;
   tok = MatchToken(script,T_BOOL_AND,type);
   kind = term(script,tok,type);
   emitcode(script,CODE_POPAND,kind);
}

void CCompiler::bool_or(TCode *script, CString &tok, TOKENTYPE &type)
{
   SYMBTYPE kind;
   tok = MatchToken(script,T_BOOL_OR,type);
   kind = term(script,tok,type);
   emitcode(script,CODE_POPOR,kind);
}


//////////////////////////////////////////////////////////////////////
// Expression parsers

SYMBTYPE CCompiler::property(TCode *script, CG_Object *obj, CString &tok, TOKENTYPE &type)
// Parses script and looks for a possible object property.
{
   ASSERT_VALID(obj);
   
   SYMBTYPE kind = VAR_OBJECT;
   tok = GetToken(script,type);
   if( type==T_PERIOD ) {
      // Object property should be here
      DATAPTR dataptr;
      tok = GetToken(script,type);
      // We now have the property name, this must be
      // stored as a literal in the DATA segment, because
      // properties are determined run-time by name.
      // TODO: Reuse possible duplicate names!
      dataptr = save_string_const(tok);
      LPVOID ptr;
      if( obj->GetProp(tok,ptr,&kind)==FALSE) {
         SCRIPTERROR(script,_T("Property '%s' not found"),(LPCTSTR)tok);
      };
      emitcode(script,CODE_LOADPROP,dataptr);
      tok = GetToken(script,type);
   };
   return kind;
};

SYMBTYPE CCompiler::value(TCode *script, CString &tok, TOKENTYPE &type)
// The core depth parsing code which finally boils the value
// out of the script.
{
   SYMBTYPE kind = VAR_UNKNOWN;
   switch(type) {
   case T_LPAREN:
      tok = MatchToken(script,T_LPAREN,type);
      kind = expression(script,tok,type);
      tok = MatchToken(script,T_RPAREN,type);
      break;

   case T_STRING_CONST: 
      {
         DATAPTR dataptr;
         emitcode(script,CODE_LOADOFFSET, GetDataPtr());
         dataptr = save_string_const(tok);
         tok = MatchToken(script,T_STRING_CONST,type);
         kind = VAR_STRING;
      };
      break;

   case T_NUMERIC_CONST:
      emitcode(script,CODE_LOADCONST,GetTokenNumber(script,tok,type));
      kind = VAR_LONG;
      break;

   case T_THIS: 
      emitcode(script,CODE_LOADTHIS);
      kind = property(script,m_game->m_current_hero,tok,type);
      break;

   case T_OBJECT: 
      {
         ASSERT_VALID(script->obj);
         CG_Object *obj = script->obj;
         emitcode(script,CODE_LOADOBJECT,obj->m_ID,tok);
         kind = property(script,obj,tok,type);
      };
      break;

   case T_PROCEDURE: 
      kind = procedure_code(script,tok,type);
      break;

   case T_NAME: 
      {
         CString name;
         TSymbol symbol;
         name = GetTokenName(script,tok,type);
         kind = findsymbol(script,name,symbol);
         emitcode(script,CODE_LOADVAR,0,name);
      }
      break;

   default:
      SCRIPTERROR(script,"Error in expression");
      break;
   };
   return kind;
}

SYMBTYPE CCompiler::factor(TCode *script, CString &tok, TOKENTYPE &type)
{
   TOKENTYPE tmp;
   SYMBTYPE kind = value(script,tok,type);

   while (iscompareop(type)) {
      emitcode(script,CODE_PUSH,kind);
      tmp = type;
      tok = MatchToken(script,type,type);
      SYMBTYPE newkind = value(script,tok,type);
      if( kind!=newkind ) { SCRIPTERROR(script,12); return VAR_UNKNOWN; };

      switch(tmp) {
      case T_EQUAL:
         emitcode(script,CODE_EQUAL,kind);
         break;
      case T_NOT_EQUAL:
         emitcode(script,CODE_EQUAL_NOT,kind);
         break;
      case T_GREATER:
         emitcode(script,CODE_GREATER,kind);
         break;
      case T_LESS:
         emitcode(script,CODE_LESS,kind);
         break;
      case T_GREATER_EQUAL:
         emitcode(script,CODE_EQUAL_GREATER,kind);
         break;
      case T_LESS_EQUAL:
         emitcode(script,CODE_EQUAL_LESS,kind);
         break;
      } //switch
   } //while
   return kind;
};

SYMBTYPE CCompiler::term(TCode *script, CString &tok, TOKENTYPE &type)
{
   SYMBTYPE kind;
   kind = factor(script,tok,type);
   while (ismulop(type)) {
      emitcode(script,CODE_PUSH,kind);
      switch(type) {
      case T_MUL:
         multiply(script,tok,type);
         break;
      case T_DIV:
         divide(script,tok,type);
         break;
      }
   }
   return kind;
}

SYMBTYPE CCompiler::expression(TCode *script, CString &tok, TOKENTYPE &type)
{
   SYMBTYPE kind = VAR_UNKNOWN;

   if (isaddop(type))
      emitcode(script,CODE_CLEAR);
   else
      kind = term(script,tok,type);

   while (isaddop(type) || isboolop(type)) {
      emitcode(script,CODE_PUSH,kind);
      switch(type) {
      case T_PLUS:
         add(script,tok,type);
         break;
      case T_MIN:
         subtract(script,tok,type);
         break;
      case T_BOOL_AND:
         bool_and(script,tok,type);
         break;
      case T_BOOL_OR:
         bool_or(script,tok,type);
         break;
      } //switch
   } //while
   return kind;
}

BOOL CCompiler::while_loop(TCode *script, CString &tok, TOKENTYPE &type)
{
   CString testlabel, donelabel;

   tok = MatchToken(script,T_WHILE,type); // while

   testlabel = newlabel(script);
   donelabel = newlabel(script);

   emitcode(script,CODE_PUTLABEL,0,testlabel);
   expression(script,tok,type);           // f!=0
   emitcode(script,CODE_IFNOTJUMPTO,0,donelabel);

   statement(script,tok,type);            // loop routine
   emitcode(script,CODE_JUMPTO,0,testlabel);

   emitcode(script,CODE_PUTLABEL,0,donelabel);
   return TRUE;
}

BOOL CCompiler::blockstatement(TCode *script, CString &tok, TOKENTYPE &type)
{
   tok = MatchToken(script,T_BEGIN,type);
   while( type != T_END ) {
      // Cycle systements as long as no }-end is met...      
      if (type == T_SEPARATOR)
         tok = GetToken(script,type);
      else
         statement(script,tok,type);
   }
   tok = MatchToken(script,T_END,type);
   return TRUE;
}

BOOL CCompiler::c_for_loop(TCode *script, CString &tok, TOKENTYPE &type)
{
   CString donelabel, statementlabel, counterlabel, testlabel;
   CString index, limit;

   MatchToken(script,T_FOR,type);             // for
   tok = MatchToken(script,T_LPAREN,type);    // (

   assignment(script,tok,type);               // f = 0
   tok = MatchToken(script,T_SEPARATOR,type); // ;

   testlabel = newlabel(script);
   donelabel = newlabel(script);
   statementlabel = newlabel(script);
   counterlabel = newlabel(script);

   //expression
   emitcode(script,CODE_PUTLABEL,0,testlabel);         
   expression(script,tok,type);               // f < 10
   tok = MatchToken(script,T_SEPARATOR,type); // ;
   emitcode(script,CODE_IFNOTJUMPTO,0,donelabel);
   emitcode(script,CODE_JUMPTO,0,statementlabel);

   //increment
   emitcode(script,CODE_PUTLABEL,0,counterlabel);
   statement(script,tok,type);                // f = f + 1
   tok = MatchToken(script,T_RPAREN,type);    // )
   emitcode(script,CODE_JUMPTO,0,testlabel);

   //statement
   emitcode(script,CODE_PUTLABEL,0,statementlabel);
   statement(script,tok,type);                // loop routine
   emitcode(script,CODE_JUMPTO,0,counterlabel);

   emitcode(script,CODE_PUTLABEL,0,donelabel);
   return TRUE;
}

BOOL CCompiler::do_while_loop(TCode *script, CString &tok, TOKENTYPE &type)
{
   CString label;

   tok = MatchToken(script,T_DO,type);      // do
   label = newlabel(script);
   emitcode(script,CODE_PUTLABEL,0,label);
   statement(script,tok,type);              // loop routine
   tok = MatchToken(script,T_WHILE,type);   // while
   expression(script,tok,type);             // f!=0
   emitcode(script,CODE_IFJUMPTO,0,label);
   tok = MatchToken(script,T_SEPARATOR,type);
   return TRUE;
}

BOOL CCompiler::if_then_else(TCode *script, CString &tok, TOKENTYPE &type)
{
   CString elselabel, donelabel;

   tok = MatchToken(script,T_IF,type);      // if
   elselabel = newlabel(script);
   donelabel = newlabel(script);
   expression(script,tok,type);             // f==0
   emitcode(script,CODE_IFNOTJUMPTO,0,elselabel);
   statement(script,tok,type);              // statement
   if (type != T_ELSE)
      emitcode(script,CODE_PUTLABEL,0,elselabel);
   else {
      tok = MatchToken(script,T_ELSE,type); // else
      emitcode(script,CODE_JUMPTO,0,donelabel);
      emitcode(script,CODE_PUTLABEL,0,elselabel);
      statement(script,tok,type);           // statement
   }
   emitcode(script,CODE_PUTLABEL,0,donelabel);
   return TRUE;
}

BOOL CCompiler::assignment(TCode *script, CString &tok, TOKENTYPE &type)
{
   CString name_var;
   TSymbol symbol;
   SYMBTYPE kind;
   name_var = GetTokenName(script,tok,type);
   if( name_var.IsEmpty() ) return FALSE;

   kind = findsymbol(script,name_var,symbol);
   if( kind==VAR_UNKNOWN ) return FALSE;

   switch(type) {
   case T_PLUSPLUS:
      tok = MatchToken(script,T_PLUSPLUS,type);
      emitcode(script,CODE_INCVAR,0,name_var);
      break;

   case T_MINMIN:
      tok = MatchToken(script,T_MINMIN,type);
      emitcode(script,CODE_DECVAR,0,name_var);
      break;

   default:
      tok = MatchToken(script,T_ASSIGN,type);
      expression(script,tok,type);
      findsymbol(script,name_var,symbol);
      if (symbol.Type == VAR_OBJECT)
         emitcode(script,CODE_STOREPOINTER,0,name_var);
      else
         emitcode(script,CODE_STORE,0,name_var);
      break;
   } //switch
   return TRUE;
}

BOOL CCompiler::statement(TCode *script, CString &tok, TOKENTYPE &type)
{
   switch(type) {
   case T_SEPARATOR:
      break;
   case T_IF:
      if_then_else(script,tok,type);
      break;
   case T_BEGIN:
      blockstatement(script,tok,type);
      break;
   case T_WHILE:
      while_loop(script,tok,type);
      break;
   case T_FOR:
      c_for_loop(script,tok,type);
      break;
   case T_DO:
      do_while_loop(script,tok,type);
      break;
   case T_PROCEDURE:
      procedure_code(script,tok,type);
      break;
   case T_DIRECTIVE:
   case T_EXIT:
   case T_RETURN:
      // This is the end...
      return FALSE;
   default:
      assignment(script,tok,type);
      break;
   };
   return TRUE;
};


//////////////////////////////////////////////////////////////////////
//
// Implementation of the CStateCompiler class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStateCompiler::CStateCompiler()
{
   m_sClassIdea = _T("States");
}

CStateCompiler::~CStateCompiler()
{
   Close();
}


//////////////////////////////////////////////////////////////////////
// Process controllers

BOOL CStateCompiler::Init(CController *game)
{
   ASSERT_VALID(game);
   if( !CFileParser::Init(game) ) return FALSE;
   m_pGame = game;
   return TRUE;
}

BOOL CStateCompiler::Close()
{
   if( !CFileParser::Close() ) return FALSE;
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Script controllers

BOOL CStateCompiler::ProcessScript( TCode *script )
{
   ASSERT(script);
   CString tok;
   TOKENTYPE type;
   while( TRUE ) {
      tok = GetToken(script,type);
      if( type==T_EOF ) break;
      if( tok!=_T("BEGIN") ) continue;
      if( GetToken(script,type)!=_T("STATEMAP") ) continue;
      
      // Intialize event map
      TStateMap statemap;
      
      tok = GetToken(script,type);
      while (TRUE) {
         // The start of the WHILE loop requires a new token
         // to be already parsed...
         if( type==T_EXIT ) {
            if( GetToken(script,type)==_T("STATEMAP") ) break;
         }
         else if( type==T_NAME ) {
            if( tok==_T("NAME") ) {
               tok = GetToken(script,type);
               _tcscpy( statemap.Name, tok );
               _tcsupr( statemap.Name );
            };
         }
         else if( type==T_DIRECTIVE ) {
            tok = GetToken(script,type);
            switch( type ) {
            case T_NAME:
               if( tok==_T("INCLUDE") ) {
                  tok = GetToken(script,type);
                  CString Filename = BfxGetFilePath(script->Filename);
                  Filename += tok;
                  ProcessFile( Filename );                  
               }
               else if( tok==_T("STATE") ) {
                  // Initialize
                  tok = GetToken(script,type);
                  long idx = _ttol(tok);
                  if( idx<0 || idx>MAX_STATES ) return SCRIPTERROR(script,_T("State out of range"));
                  // Parse scripting code
                  statemap.statecode[idx] = ParseState(script,tok,type);
                  statemap.statecode[idx].MakeUpper();
                  // Done
               }
               break;
            };
            continue;
         };
         tok = GetToken(script,type);
      }; // inner WHILE(TRUE)
      m_pGame->m_statemaps.Add(statemap);
   }; // WHILE(TRUE)

   return TRUE;
};

CString CStateCompiler::ParseState(TCode *script, CString &tok, TOKENTYPE &type)
{
   ASSERT(script);
   CString s;
   BOOL ok = TRUE;
   tok = GetToken(script,type);
   while ( ok ) {
      switch( type ) {
      case T_DIRECTIVE:
      case T_EXIT:
      case T_RETURN:
         // This is the end...
         if( s.Right(1)!=_T('\n')) s+=_T('\n');
         s += _T("END\n");
         return s;
      default:
         // grab complete line and add it to state code
         // (note that line has been de-commented and trimmed)
         s += tok + script->line;
         s += _T("\n");
      };
      script->bEOL = TRUE; // force new line feed
      tok = GetToken(script,type);
   };
   return CString();
}




//////////////////////////////////////////////////////////////////////
//
// Implementation of the CLogicFile class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogicFile::CLogicFile()
{
}

CLogicFile::~CLogicFile()
{
}

BOOL CLogicFile::Load(LPCTSTR Filename, 
                      TEventMap *&eventmaps, DWORD &eventsize,
                      CODEPTR *&code, DWORD &codesize,
                      DATAPTR *&data, DWORD &datasize )
{
   CFile file;
   TRY
	{	
		if( !file.Open( Filename, CFile::modeRead|CFile::typeBinary ) ) return FALSE;
      //
      TLogicFileHeader header;
      file.Read(&header,sizeof(header));
      if( header.ID!=CODEFILE_ID ) return FALSE;
      if( header.Version!=CODEFILE_VER ) return FALSE;
      if( header.CodeSize<=0 ) return FALSE;
      if( header.DataSize<=0 ) return FALSE;
      if( header.CodeSize>0x100000 ) return FALSE;
      if( header.DataSize>0x100000 ) return FALSE;
      //
      eventmaps = new TEventMap[header.NumEventMaps];
      code = new CODEPTR[header.CodeSize];
      data = new DATAPTR[header.DataSize];
      if( (eventmaps==NULL) || (code==NULL) || (data==NULL) ) return FALSE; // out of memory
      eventsize = header.NumEventMaps;
      codesize = header.CodeSize;
      datasize = header.DataSize;
      //
      file.Read(eventmaps,header.EventMapSize);
      file.Read(code,header.CodeSize);
      file.Read(data,header.DataSize);
      //
      file.Close();
	}
	CATCH( CFileException, e )
	{
#ifdef _DEBUG
      e->ReportError();
#endif
		// error: file error
      file.Abort();
      return FALSE;
	}
	END_CATCH;
   return TRUE;
}

BOOL CLogicFile::Save(LPCTSTR Filename, 
                      CCompiler::CEventMapArray &eventmaps,
                      CDWordArray &code,
                      CDWordArray &data )
{  
   // Set up header for file
   TLogicFileHeader header;
   header.ID = CODEFILE_ID;
   header.Version = CODEFILE_VER;
   header.NumEventMaps = eventmaps.GetSize();
   header.EventMapSize = eventmaps.GetSize()*sizeof(TEventMap);
   header.CodeSize = code.GetSize()*sizeof(CODEPTR);
   header.DataSize = data.GetSize()*sizeof(DATAPTR);
   // Validate
   if( header.CodeSize==0 ) return FALSE;
   if( header.DataSize==0 ) return FALSE;
   // Write out file...
   CFile file;
   TRY
	{	
      if( file.Open( Filename, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary )==FALSE ) return FALSE;
      file.Write(&header,sizeof(header));
      file.Write(&eventmaps[0],header.EventMapSize);
      file.Write(&code[0],header.CodeSize);
      file.Write(&data[0],header.DataSize);
      file.Close();
	}
	CATCH( CFileException, e )
	{
#ifdef _DEBUG
      e->ReportError();
#endif
      file.Abort();
      return FALSE;
	}
	END_CATCH;
   return TRUE;
}
