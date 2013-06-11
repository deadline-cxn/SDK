// ScriptParser.cpp: implementation of the CScriptParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static LPCTSTR keywords[] = {
   _T("CONST"),
   _T("BEGIN"),
   _T("END"),
   _T("INCLUDE"),
//--------------
   _T("ID"),
   _T("TITLE"),
   _T("NAME"),
   _T("DESCRIPTION"),
   _T("SURFACE"),
//
   _T("FRAME"),
   _T("EXIT"),
   _T("OBJ"),
//
   _T("FRAMES"),
   _T("POS"),
   _T("STARTROOM"),
   _T("STARTPOS"),
   _T("STATECODE"),
   _T("EVENTCODE"),
//--------------
   NULL };

CScriptParser::CScriptParser()
{
}

CScriptParser::~CScriptParser()
{
}

BOOL CScriptParser::Close()
{
   return TRUE;
}

BOOL CScriptParser::Init(CG_Controller *game)
{
   ASSERT_VALID( game );
   if( game==NULL ) return FALSE;
   m_game = game;
   return TRUE;
}

BOOL CScriptParser::SCRIPTMESSAGE(BOOL bError, TScript *script, LPCTSTR szFormat, ...)
{
   CApp *app = (CApp *)AfxGetApp();
   ASSERT( script );
   CString str;
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);
   va_end(args);
   if( bError ) 
      return app->m_debug.Error(str);
   else
      return app->m_debug.Print(str);
}

BOOL CScriptParser::SCRIPTERROR(TScript *script, LPCTSTR szFormat, ...)
{
   ASSERT( script );
   CString msg, str;
   msg.Format("Error in script %s line %d: ", script->Filename, script->LineNr );
   va_list args;   
   va_start(args, szFormat);
   str.FormatV(szFormat, args);
   va_end(args);
   msg += str;
   SCRIPTMESSAGE(TRUE,script,msg);
   SCRIPTMESSAGE(TRUE,script,_T("Content: '%s'\r\n"),script->orgline);
   return FALSE;
}

BOOL CScriptParser::SCRIPTERROR(TScript *script, short ErrCode )
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
      NULL };
   LPCTSTR *p;
   ASSERT( script );
   p = ErrMsgs;
   while( *p!=NULL ) {
      if( ErrCode<=0 ) break;
      ErrCode--;
      p++;
   };
   return SCRIPTERROR(script,*p);
}



//////////////////////////////////////////////////////////////////////
// Process controllers

BOOL CScriptParser::ProcessFile(LPCTSTR Filename)
{
   CApp *app = (CApp *)AfxGetApp();

   app->m_debug.PrintReplaceLine(_T("Parsing ") + CString(Filename));

   TScript script;
   TRY
	{	
      script.Filename = Filename;
      script.LineNr = 0;
      script.bEOFLine = TRUE;
      //
      if( !script.file.Open( Filename, CFile::modeRead | CFile::typeText ) ) return SCRIPTERROR(&script,8);
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
   app->m_debug.PrintReplaceLine(_T("Definitions parsed."));
   return TRUE;
}


BOOL CScriptParser::GetLine(TScript *script, CString &s)
{
   TRY
	{	
      if( !script->file.ReadString(s) ) {
         script->file.Abort();
         s.Empty();
         return FALSE;
      };
      script->orgline = s;
      script->LineNr++;
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
   // Trim string and remove comment
   s.TrimLeft();
   long pos = s.Find(_T("//"));
   if( pos>=0 ) s = s.Left(pos);
   s.TrimRight();
   script->line = s;
   return TRUE;
}

CString CScriptParser::GetToken(TScript *script, TOKENTYPE &type)
{
GrabToken:;
   if( script->bEOFLine ) {
      if( !GetLine( script, script->line ) ) {
         type = TT_EOF;
         return CString();
      };
      script->bEOFLine = FALSE;
   };
   if( script->line.IsEmpty() ) {
      type = TT_EOL;
      script->bEOFLine = TRUE;
      return CString();
   };
   script->line.TrimLeft();
   if( script->line.IsEmpty() ) goto GrabToken;
   CString newtoken;

   int pos = 0;
   int len = script->line.GetLength();
   TCHAR c = script->line[0];

   if( _istdigit(c) ) {
      while( len-->0 ) {
         c = script->line[pos];
         if( !_istdigit(c) ) break;
         newtoken = newtoken + c;
         pos++;
      };
      type = TT_VALUE;
      goto GotToken;
   };
   
   if( c==_T('"') ) {
      pos++;
      len--;
      while( len-->0 ) {
         c = script->line[pos];
         if( c==_T('"') ) break;
         newtoken = newtoken + c;
         pos++;
      };
      pos++;
      len--;
      type = TT_STRING;
      goto GotToken;
   };

   if( !_istalnum(script->line[0]) ) {
      while( len-->0 ) {
         c = script->line[pos];
         if( _istalnum(c) ) break;
         if( !_istgraph(c) ) break;
         newtoken = newtoken + c;
         pos++;
      };
      type = TT_SEPERATOR;
      goto GotToken;
   };
   
   // At last get string (might be a keyword)
   while( len-->0 ) {
      c = script->line[pos];
      if( !_istalnum(c) ) break;
      newtoken = newtoken + c;
      pos++;
   };

   // Here we check for all known keywords
   // We make the seach case-insensitive (by making a temporary string
   // which is uppercased)
   {
      CString tmp( newtoken );
      LPCTSTR *p = keywords;
      tmp.MakeUpper();
      type = TT_CONSTANT;
      while( *p!=NULL ) {
         if( tmp==*p ) {
            type = TT_KEYWORD;
            break;
         };
         p++;
      };
   };
   
GotToken:
   script->line = script->line.Mid( pos );
   return newtoken;
}

CString CScriptParser::GetToken(TScript *script, TOKENTYPE &type, TOKENTYPE match)
{
   CString str = GetToken(script,type);
   if( type!=match ) {
      SCRIPTERROR(script,12);
      return CString();
   };
   return str;
};

CRect CScriptParser::GetTokenRect(TScript *script)
{
   ASSERT(script);
   CString tok;
   TOKENTYPE type;
   tok = GetToken(script,type);
   if( type!=TT_STRING ) { SCRIPTERROR(script,7); return CRect(); };
   short x,y,w,h;
   sscanf(tok, _T("%d,%d,%d,%d"), &x,&y,&w,&h );
   return CRect( x,y,x+w-1,y+h-1 );
}

CPoint CScriptParser::GetTokenPos(TScript *script)
{
   ASSERT(script);
   CString tok;
   TOKENTYPE type;
   tok = GetToken(script,type);
   if( type!=TT_STRING ) { SCRIPTERROR(script,7); return CPoint(); };
   short x,y;
   sscanf(tok, _T("%d,%d"), &x,&y );
   return CPoint(x,y);
}


//////////////////////////////////////////////////////////////////////
// Process script

#define BEGIN_PARSE { \
   ASSERT(script); \
   CString tok; \
   TOKENTYPE type; \
   while( TRUE ) { \
      tok = GetToken(script, type); \
      switch( type ) {
#define END_PARSE ; \
      case TT_EOL: \
         break; \
      case TT_EOF: \
         return TRUE; \
      default: \
         return SCRIPTERROR(script,2); \
      }; \
   }; \
};


BOOL CScriptParser::ProcessScript(TScript *script)
{
   BOOL ok;

   BEGIN_PARSE
   case TT_KEYWORD: {
      if( tok==_T("INCLUDE") )
         ok = ParseInclude(script);
      else if( tok==_T("BEGIN") ) {
         tok = GetToken(script, type);
         if( type!=TT_CONSTANT ) return SCRIPTERROR(script,7);
         if( tok==_T("ROOM") ) 
            ok = ParseRoom(script);
         else if( tok==_T("PLAYER") ) 
            ok = ParseCharacter(script);
         else if( tok==_T("PROPS") ) 
            ok = ParseProps(script);
         else
            return SCRIPTERROR(script,2);
      }
      else
         return SCRIPTERROR(script,2);
      if( !ok ) return FALSE;
      break;
   }
   END_PARSE
   return FALSE;  // if we get here, we're lost
};

BOOL CScriptParser::ParseInclude(TScript *script)
{
   CString tok;
   TOKENTYPE type;
   tok = GetToken(script, type );
   if( type!=TT_STRING ) return SCRIPTERROR(script,7);
   CString Filename = BfxGetFilePath(script->Filename);
   Filename += tok;
   BOOL ok = ProcessFile( Filename );
   if (!ok) SCRIPTERROR(script,9);
   return TRUE; //parse on, dude...
}


BOOL CScriptParser::ParseRoom(TScript *script)
{
   CG_Room *obj = new CG_Room;
   if( obj==NULL ) return SCRIPTERROR(script,6);
   BEGIN_PARSE
   case TT_KEYWORD: {
      if( tok==_T("END") ) {
         if( GetToken(script, type)!=_T("ROOM") ) return SCRIPTERROR(script,1);
         m_game->m_rooms.Add(obj);
         return TRUE;
      }
      else if( tok==_T("TITLE") )
         obj->m_sName = GetToken(script,type,TT_STRING);
      else if( tok==_T("SURFACE") ) {
         obj->m_sSurfaceName = GetToken(script,type,TT_STRING);
         obj->m_sSurfaceName.MakeUpper();
      }
      else if( tok==_T("DESCRIPTION") )
         obj->m_sDescription = GetToken(script,type,TT_STRING);
      else if( tok==_T("ID") )
         obj->m_ID = _ttol(GetToken(script,type,TT_VALUE));
      else if( tok==_T("BEGIN") ) {
         tok = GetToken(script,type);
         if( type!=TT_CONSTANT ) return SCRIPTERROR(script,7);
         if( tok==_T("OBJECTS") ) {
            BEGIN_PARSE
            case TT_KEYWORD: {
               if( tok==_T("END") ) {
                  if( GetToken(script,type)!=_T("OBJECTS") ) return SCRIPTERROR(script,1);
                  goto Done_Room;
               }
               else if (tok==_T("OBJ") )
                  ParseRoomObject(script,obj,1);
               else
                  return SCRIPTERROR(script,2);
            };
            break;
            END_PARSE
         }
         else if( tok==_T("STATICOBJECTS") ) {
            BEGIN_PARSE
            case TT_KEYWORD: {
               if( tok==_T("END") ) {
                  if( GetToken(script,type)!=_T("STATICOBJECTS") ) return SCRIPTERROR(script,1);
                  goto Done_Room;
               }
               else if (tok==_T("OBJ") )
                  ParseRoomObject(script,obj,0);
               else
                  return SCRIPTERROR(script,2);
            };
            break;
            END_PARSE
         }
         else if( tok==_T("EXITS") ) {
            BEGIN_PARSE
            case TT_KEYWORD: {
               if( tok==_T("END") ) {
                  if( GetToken(script,type)!=_T("EXITS") ) return SCRIPTERROR(script,1);
                  goto Done_Room;
               }
               else if (tok==_T("EXIT") )
                  ParseRoomExit(script,obj);
               else
                  return SCRIPTERROR(script,2);
            };
            break;
            END_PARSE
         }
         else
            return SCRIPTERROR(script,1);
      }
      else
         return SCRIPTERROR(script,2);
Done_Room: ;
   };
   break;
   END_PARSE
}


BOOL CScriptParser::ParseProps(TScript *script)
{
   CString sFilename;

   BEGIN_PARSE
   case TT_KEYWORD: {
      if( tok==_T("END") ) {
         tok = GetToken(script, type);
         if( tok!=_T("PROPS") ) return SCRIPTERROR(script,1);
         return TRUE;
      }
      else if( tok==_T("SURFACE") ) {
         sFilename = GetToken(script,type);
         sFilename.MakeUpper();
      }
      else if( tok==_T("BEGIN") ) {
         tok = GetToken(script,type);
         if( type!=TT_CONSTANT ) return SCRIPTERROR(script,7);
         if( tok==_T("OBJECT") ) {
            CG_Prop *obj = new CG_Prop;
            if( obj==NULL ) return SCRIPTERROR(script,6);
            obj->m_sSurfaceName = sFilename;
            BEGIN_PARSE
            case TT_KEYWORD: {
               if( tok==_T("END") ) {
                  if( GetToken(script, type)!=_T("OBJECT") ) return SCRIPTERROR(script,1);
                  // Add object to known list of objects
                  m_game->m_props.Add(obj);
                  goto done_object;
               }
               else if( tok==_T("NAME") )
                  obj->m_sName = GetToken(script,type,TT_STRING);
               else if( tok==_T("FRAME") )
                  obj->m_frames.Add(GetTokenRect(script));
            };
            break;
            END_PARSE
         }
         else if( tok==_T("ITEM") ) {
            CG_Item *obj = new CG_Item;
            if( obj==NULL ) return SCRIPTERROR(script,6);
            obj->m_sSurfaceName = sFilename;
            BEGIN_PARSE
            case TT_KEYWORD: {
               if( tok==_T("END") ) {
                  if( GetToken(script, type)!=_T("ITEM") ) return SCRIPTERROR(script,1);
                  // Add object to known list of objects
                  obj->m_sName.MakeUpper();
                  m_game->m_items.Add(obj);
                  goto done_object;
               }
               else if( tok==_T("NAME") )
                  obj->m_sName = GetToken(script,type,TT_STRING);
               else if( tok==_T("DESCRIPTION") )
                  obj->m_sDescription = GetToken(script,type,TT_STRING);
               else if( tok==_T("ID") )
                  obj->m_ID = _ttol(GetToken(script,type,TT_VALUE));
               else if( tok==_T("STARTROOM") )
                  obj->m_sStartRoom = GetToken(script,type,TT_STRING);
               else if( tok==_T("STARTPOS") )
                  obj->m_StartPos = GetTokenPos(script);
               else if( tok==_T("FRAME") )
                  obj->m_frames.Add(GetTokenRect(script));
               else
                  return SCRIPTERROR(script,2);
            };
            break;
            END_PARSE
         }
         else
            SCRIPTERROR(script,10);
done_object: ;
      };
   };
   break;
   END_PARSE
   return FALSE;
}

BOOL CScriptParser::ParseCharacter(TScript *script)
{
   CG_Character *obj = new CG_Character;
   if( obj==NULL ) return SCRIPTERROR(script,6);
   BEGIN_PARSE
   case TT_KEYWORD: {
      if( tok==_T("END") ) {
         if( GetToken(script, type)!=_T("PLAYER") ) return SCRIPTERROR(script,1);
         m_game->m_chars.Add(obj);
         return TRUE;
      }
      else if( tok==_T("ID") )
         obj->m_ID = _ttol(GetToken(script,type,TT_VALUE));
      else if( tok==_T("NAME") )
         obj->m_sName = GetToken(script,type,TT_STRING);
      else if( tok==_T("DESCRIPTION") )
         obj->m_sDescription = GetToken(script,type,TT_STRING);
      else if( tok==_T("SURFACE") ) {
         obj->m_sSurfaceName = GetToken(script,type,TT_STRING);
         obj->m_sSurfaceName.MakeUpper();
      }
      else if( tok==_T("STARTROOM") )
         obj->m_sStartRoom = GetToken(script,type,TT_STRING);
      else if( tok==_T("STARTPOS") )
         obj->m_StartPos = GetTokenPos(script);
      else if( tok==_T("EVENTCODE") )
         obj->m_sLinkEventCode = GetToken(script,type,TT_STRING);
      else if( tok==_T("STATECODE") )
         obj->m_sLinkStateCode = GetToken(script,type,TT_STRING);
      else if( tok==_T("BEGIN") ) {
         tok = GetToken(script,type);
         if( type!=TT_CONSTANT ) return SCRIPTERROR(script,7);
         if( tok==_T("ANIM") ) {
            short frames_rep = 1;
            BEGIN_PARSE
            case TT_KEYWORD: {
               if( tok==_T("END") ) {
                  if( GetToken(script, type)!=_T("ANIM") ) return SCRIPTERROR(script,1);
                  // Add object to known list of objects
                  obj->m_nNumAnims++;
                  goto done_object;
               }
               else if( tok==_T("TITLE") )
                  obj->m_anims[obj->m_nNumAnims].Title = GetToken(script,type,TT_STRING);
               else if( tok==_T("FRAMES") )
                  frames_rep = _ttoi(GetToken(script,type,TT_VALUE));
               else if( tok==_T("FRAME") ) {
                  CRect rc = GetTokenRect(script);
                  for( int i=0; i<frames_rep; i++ ) {
                     obj->m_anims[obj->m_nNumAnims].Frames.Add(rc);
                     rc.left += rc.Width();
                  };
               }
               else
                  return SCRIPTERROR(script,2);
            };
            break;
            END_PARSE
         }
      }
      else
         return SCRIPTERROR(script,2);
done_object: ;
   };
   break;
   END_PARSE
   return FALSE;
}

BOOL CScriptParser::ParseRoomObject(TScript *script, CG_Room *room, short objtype)
{
   ASSERT(script);
   ASSERT_VALID(room);
   CString tok;
   TOKENTYPE type;
   CG_Room::TRoomObject obj;
   tok = GetToken(script,type);
   tok.MakeUpper();
   BOOL bFound = FALSE;
   for(int i=0; i<m_game->m_props.GetSize(); i++ ) {
      if( m_game->m_props[i]->m_sName == tok ) { bFound = TRUE; break; };
   };
   if(!bFound) return SCRIPTERROR(script,11);
   obj.obj = m_game->m_props[i];
   obj.pos = GetTokenPos(script);
   switch( objtype ) {
   case 0:
      room->m_staticobjects.Add(obj);
      break;
   default:
      room->m_objects.Add(obj);
   };
   return TRUE;
}

BOOL CScriptParser::ParseRoomExit(TScript *script, CG_Room *room)
{
   ASSERT(script);
   ASSERT_VALID(room);
   CString tok;
   TOKENTYPE type;
   CG_Room::TRoomExit obj;
   obj.ID = _ttoi(GetToken(script,type));
   if(type!=TT_VALUE) return SCRIPTERROR(script,7);
   obj.exit = GetTokenRect(script);
   room->m_exits.Add(obj);
   return TRUE;
}

BOOL CScriptParser::Validate()
{
#define ADDOBJ(x) { \
   CString tmp(x->m_sName); \
   tmp.MakeUpper(); \
   m_game->m_objects.SetAt( tmp, x ); \
   };

   ASSERT_VALID( m_game );
   int i;
   //
   // Add all objects to Map (gives quick access to names)
   //
   m_game->m_objects.RemoveAll();
   for( i=0; i<m_game->m_chars.GetSize(); i++ ) ADDOBJ(m_game->m_chars[i]);
   for( i=0; i<m_game->m_props.GetSize(); i++ ) ADDOBJ(m_game->m_props[i]);
   for( i=0; i<m_game->m_rooms.GetSize(); i++ ) ADDOBJ(m_game->m_rooms[i]);
   for( i=0; i<m_game->m_items.GetSize(); i++ ) ADDOBJ(m_game->m_items[i]);
   //
   // Validate all ROOM references
   //
   return TRUE;
}
