// StateMachine.cpp: implementation of the CStateMachine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StateMachine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStateMachine::CStateMachine()
{
   ResetStateAttributes();
   m_statemap = NULL;
   m_nState = 0;
   m_nIndex = 0;
}

CStateMachine::~CStateMachine()
{

}


//////////////////////////////////////////////////////////////////////
// Implementation

BOOL CStateMachine::Bind(TStateMap *map)
{
   ASSERT(AfxIsValidAddress(map,sizeof(TStateMap)));
   m_statemap = map;
   return TRUE;
}

BOOL CStateMachine::ResetStateAttributes()
{
   walk.steps = 0;
   wait.target = 0;
   path.trips = 0;
   anim.name.Empty();
   anim.wait = FALSE;
   follow.target = NULL;
   return TRUE;
}

BOOL CStateMachine::IsReadyForNewState()
{
   // Here we test if we're ready
   // get grab a new state line
   // We must test if we're still walking, waiting
   // etc...
   if( m_statemap==NULL ) return FALSE; // no statemap bound
   if( m_nState<=0 ) return FALSE; // not a valid state
   BOOL bReady = TRUE;
   if( walk.steps > 0 ) bReady = FALSE;
   else if( wait.target > 0 ) bReady = FALSE;
   else if( path.trips > 0 ) bReady = FALSE;
   else if( anim.wait ) bReady = FALSE;
   else if( follow.target!=NULL ) bReady = FALSE;
   return bReady;
}

BOOL CStateMachine::GetNewState( CController *pGame, CG_Item *pPlayer )
{
   if( m_statemap==NULL ) return TRUE; // no statemap bound
   if( m_nState<=0 ) return FALSE; // not a valid state
   ASSERT_VALID(pGame);
   ASSERT_VALID(pPlayer);

   //
   // Get next state line
   //
   CString *str = &m_statemap->statecode[m_nState];
   CString line;
   int pos;
   pos = str->Find(_T('\n'),m_nIndex);
   if( pos==0 ) pos = str->GetLength();
   line = str->Mid(m_nIndex,pos-m_nIndex);

   // Ajust new index
   m_nIndex += line.GetLength()+1;
   if( m_nIndex >= str->GetLength() ) m_nIndex = 0;

   //
   // Interpret command
   //
   CFixedString<32> cmd = BfxRemoveToken(line,_T(' '));
   if(cmd==_T("WAIT")) {
      // Wait some time...
      wait.target = KfxGetTime() + (_ttol(line)*KfxGetTimerFreq());
   }
   else if(cmd==_T("SWITCHSTATE")) {
      // Enter a new state
      EnterNewState(_ttol(line));
   }
   else if(cmd==_T("PLAYANIM")) {
      // Play an animation sequence
      anim.name = GetQuotedString(line);
      anim.wait = (line==_T("WAIT"));
   }
   else if(cmd==_T("GOTO")) {
      // Walk to a new location
      // (possibly in a new room!)
      CFixedString<128> roomname = GetQuotedString(line);
      CG_Room *room = pGame->m_rooms.Find(roomname);
      if( room==NULL ) return FALSE;
      path.destroom = room->m_ID;
      sscanf( line, _T("(%d,%d)"), &path.destpos.x, &path.destpos.y);
      // Before we go ahead, we'll just check
      // a few things...
      if( pPlayer->m_RoomID < 0 ) {
         // Player is invisible, make visible
         pPlayer->m_RoomID = path.destroom;
         pPlayer->m_pos = path.destpos;
      };
      //
      // Let's get ready to calculate a new path
      // which should be walked...
      CG_Room *curroom = pGame->m_rooms.Find(pPlayer->m_RoomID);
      path.stops.RemoveAll();
      CalcComplexWalk( pGame, pPlayer, curroom );
      path.trips = path.stops.GetSize();
      path.idx = 0;
   }
   else if(cmd==_T("FOLLOW")) {
      // Player in follow mode
      // (walks after hero whenever in the same room)
      CFixedString<128> name = GetQuotedString(line);
      CG_Character *pFollow = pGame->m_chars.Find(name);
      if(pFollow==NULL) return FALSE;
      follow.target = pFollow;
      follow.cnt = 0;
      follow.distance = _ttol(line);
      if( follow.distance<=0 ) follow.distance = 1200;  // default distance to target
   }
   else if(cmd==_T("IF")) {
      // Parse conditional IF expression
      BOOL ok = EvaluateExpression(line,pPlayer);
      if( !ok ) {
         // DO NOT
         //
         // Parse over all lines, untill we meet an ENDIF
         // (doesn't do nested IFs!)
         pos = str->Find(_T("ENDIF"),m_nIndex);
         if( pos==0 ) 
            m_nIndex = 0; // some error: ENDIF not found
         else {
            m_nIndex = pos;
         };
      }
      else {
         // DO
         //
         // Execute commands now...
         // (actually we just leave things as they are
         //  since we will eventually execute the statements
         //  after the IF line)
      };
   }
   else if(cmd==_T("ENDIF")) 
      ;
   else if(cmd==_T("DIE")) {
      // Die motherfxxxxx...
   }
   else if(cmd==_T("LOG")) {
      // Log to debug log
      CFixedString<128> s = GetQuotedString(line);
      DEBUGLOG(s);
   }
   else if(cmd==_T("LOOP")) {
      m_nIndex = 0; // restart state code
   }
   else if(cmd==_T("RESET")) {
      ResetStateAttributes(); // reset state processes
   }
   else if(cmd==_T("END"))
      m_nState = 0; // clear state
                    // but leave settings as they are

   return TRUE;
}

BOOL CStateMachine::EnterNewState(long idx)
{
   if( idx<0 || idx>MAX_STATES ) return FALSE; // not a valid state
   if( idx == m_nState ) return TRUE; // already in state
   ResetStateAttributes();
   m_nState = idx;
   m_nIndex = 0;
   return TRUE;
}

BOOL CStateMachine::ProcessState( CController *pGame, CG_Item *pPlayer )
{
   // Do something
   CFixedString<128> sAnimName;

   //
   // Handle different states
   //

   //--- Process wait AI
   if( wait.target!=0 ) {
      if( KfxGetTime() > wait.target ) wait.target = 0;
      // Player stands when waiting!
      pPlayer->m_ai = STAND;
   };

   //--- Process long walk AI
   if( (path.trips>0) && (walk.steps<=0) ) {
      int idx = path.idx;
      if( idx < path.trips ) {
         if( path.stops[idx].walking ) {
            walk.x = (float)pPlayer->m_pos.x;
            walk.y = (float)pPlayer->m_pos.y;
            BOOL ok = KfxCalcWalk(pPlayer->m_pos,
                                  path.stops[idx].pos, 
                                  pPlayer->m_nVelocity,
                                  walk.steps,
                                  walk.delta_x, walk.delta_y);
            pPlayer->m_ai = WALKING; // we're walking
         }
         else {
            pPlayer->m_pos = path.stops[idx].pos; 
            // If player is jumping from room to another,
            // then we must change room and alert all VMs
            if( pPlayer->m_RoomID != path.stops[idx].room ) {
               pGame->PostMessageToAllInRoom( EVENT_EXITROOM, pPlayer->m_RoomID, pPlayer );
               pPlayer->m_RoomID = path.stops[idx].room; 
               pGame->PostMessageToAllInRoom( EVENT_ENTERROOM, pPlayer->m_RoomID, pPlayer );
            };
            walk.steps = 0;
            pPlayer->m_ai = STAND; // we're walking
         };
      }
      else
         path.trips = 0; // done with this... path travelled!
      path.idx++;
   };

   //--- Process follow AI
   if( follow.target!=NULL ) {
      // Must be in same room to follow
      if( pPlayer->m_RoomID == follow.target->m_RoomID ) {
         // Thinking about changing direction?
         if( --follow.cnt<=0 ) {
            // If we're not near, walk
            if( KfxCalcDistance( pPlayer->m_pos, follow.target->m_pos ) < follow.distance ) 
               pPlayer->m_ai = STAND;
            else {
               walk.x = (float)pPlayer->m_pos.x;
               walk.y = (float)pPlayer->m_pos.y;
               BOOL ok = KfxCalcWalk(pPlayer->m_pos,
                                     follow.target->m_pos, 
                                     pPlayer->m_nVelocity,
                                     walk.steps,
                                     walk.delta_x, walk.delta_y);
               pPlayer->m_ai = WALKING;
            };
            follow.cnt = 20;  // change direction every 20 frames
         };
      };
   };

   //--- Process anim AI
   if( !anim.name.IsEmpty() ) pPlayer->m_ai = ANIMSEQ;

   //
   // Raw AI programming
   //
   switch( pPlayer->m_ai ) {
   case STAND:
      {
         // Stop and don't move!
         sAnimName = _T("STAND");
      }
      break;
   case ANIMSEQ:
      {
         // Playing anim sequence...
         if( !anim.name.IsEmpty() ) 
            sAnimName = anim.name; 
         else 
            sAnimName = _T("STAND");
      }
      break;
   case WATCH:
      {
         //
         sAnimName = _T("STAND");
         pPlayer->m_bTurningLeft = (pGame->m_current_hero->m_pos.x > pPlayer->m_pos.x);
      }
      break;
   case WALKING:
      {
         sAnimName = _T("WALK");
         // Walk this way...
         CPoint newpos( (long)walk.x, (long)walk.y );
         CPoint checkpos( newpos );
         checkpos.x += pPlayer->m_size.cx / 2;
         checkpos.y += pPlayer->m_size.cy;
         if( pGame->IsValidSpot( checkpos ) ) {
            pPlayer->m_pos = newpos;
            walk.x += walk.delta_x;
            walk.y += walk.delta_y;
            walk.steps--;
            pPlayer->m_bTurningLeft = (walk.delta_x>0);
         }
         else
            walk.steps = 0; // way blocked
         // If we're not walking, we're standing!
         // This will fake any subsequent confusion of which
         // state the player is in (also good for HERO because
         // it automatically goes into stop after walk!)
         if( walk.steps<=0 ) pPlayer->m_ai = STAND;
         break;
      };
   default:
      {
         return DEBUGERROR(_T("Unknown state!"));
      };
   };

   return pPlayer->ManageAnims(pGame,sAnimName);
}


CString CStateMachine::GetQuotedString(CString &str)
// Extract a quoted string from the state code
// A quoted string has surrounding "-chars
{
   if( str.IsEmpty() ) return CString();
   if( str[0]!=_T('"') ) return CString();
   CString tok;
   str = str.Mid(1);
   tok = BfxRemoveToken(str,_T('"'));
   if( tok.IsEmpty() ) return CString();
   str.TrimLeft();
   return tok;
}

BOOL CStateMachine::EvaluateExpression(CString &line, CG_Item *pPlayer)
// Evaluate expression (IF statement, mostly)
// in state code
{
   enum { OP_OR, OP_AND } op = OP_OR;
   BOOL ok = FALSE;
   do {
      SYMBTYPE symbtype;
      BOOL b;
      // Get property
      CString left_expr( BfxRemoveToken(line,_T(' ')) );
      CString str_left;
      DWORD int_left;
      b = EvaluateValue(pPlayer,left_expr,symbtype,str_left,int_left);
      if(!b) return FALSE;

      // Get expression match symbol (=,<,>,<= or >=)
      CFixedString<16> match = BfxRemoveToken(line,_T(' '));
      if( match.IsEmpty() ) return FALSE;

      // Get right side of expression
      CString str_right;
      DWORD int_right;
      CString right_expr( BfxRemoveToken(line,_T(' ')) );
      b = EvaluateValue(pPlayer,right_expr,symbtype,str_right,int_right);
      if(!b) return FALSE;

      // Get expression result
      BOOL result;
      switch( symbtype ) {
      case VAR_LONG:
         if( match==_T("=") )
            result = (int_left==int_right);
         else if( match==_T("<") )
            result = (int_left<int_right);
         else if( match==_T(">") )
            result = (int_left>int_right);
         else if( match==_T("<=") )
            result = (int_left<=int_right);
         else if( match==_T(">=") )
            result = (int_left>=int_right);
         break;
      case VAR_STRING:
         str_left.MakeUpper();
         str_right.MakeUpper();
         if( match==_T("=") )
            result = (str_left==str_right);
         else if( match==_T("<") )
            result = (str_left<str_right);
         else if( match==_T(">") )
            result = (str_left>str_right);
         else if( match==_T("<=") )
            result = (str_left<=str_right);
         else if( match==_T(">=") )
            result = (str_left>=str_right);
         break;
      }
      
      // Get AND/OR operator
      CFixedString<16> opr = BfxRemoveToken(line,_T(' '));
      if(opr==_T("AND")) op = OP_AND; 
      else if(opr==_T("OR")) op = OP_OR;
      else break;
      // Match operator
      if( opr==OP_AND ) ok &= result;
      else if( opr==OP_OR ) ok |= result;
   
   } while( ok );
   // Return result of expression match
   return ok;
}

BOOL CStateMachine::EvaluateValue(CG_Item *pPlayer,
                                  CString &name, 
                                  SYMBTYPE &vartype, 
                                  CString &output_str, DWORD &output_int)
// Extract a value from the state code
// The value can be a string, constant (integer) or
// a numerical value (also integer)
{
   if( name.IsEmpty() ) return FALSE;
   TCHAR c = name[0];
   if( isdigit(c) ) {
      output_int = _ttol(name);
      vartype = VAR_LONG;
   }
   else if( c==_T('"') ) {
      output_str = GetQuotedString(name);
      vartype = VAR_STRING;
   }
   else {
      LPVOID ptr;
      SYMBTYPE symbtype;
      BOOL b = pPlayer->GetProp(name,ptr,&symbtype);
      if( !b ) return FALSE;
      switch( symbtype ) {
      case VAR_LONG:
         output_int = *(LPDWORD)ptr;
         vartype = VAR_LONG;
         break;
      case VAR_STRING:
         output_str = *(CString *)ptr;
         vartype = VAR_STRING;
         break;
      };
   };
   return TRUE;
}

BOOL CStateMachine::CalcComplexWalk(CController *pGame,
                                    CG_Item *pPlayer,
                                    CG_Room *pRoom)
// A recursive function which calculates the
// possible path to a room/position
{
   // Are we at the goal?
   // If we are in the same room, we're very close...
   if( path.destroom == pRoom->m_ID ) {      
      // Same room! Easy to calc path to target position...
      WALK_STOP stop;
      stop.walking = TRUE;
      stop.pos = path.destpos;
      stop.room = path.destroom;
      path.stops.Add( stop );
      // The goal has been reached!
      // When we jump out of the recursive calls, we
      // signal success!!!!
      return TRUE;
   }

   // Traverse all exits in the room while
   // pushing the path to it on the stack (list of exits)
   // If the path does not lead to the goal, pop the
   // path of the stack again...
   short idx = 0;
   while( idx < pRoom->m_exits.GetSize() ) {
      CG_Room *pNewRoom;
      CPoint newpos = CPoint( pRoom->m_exits[idx].exitarea.CenterPoint() );
      WALK_STOP walk;
      //
      walk.walking = TRUE;
      walk.pos = newpos;
      walk.room = pRoom->m_ID;
      path.stops.Add( walk );
      //
      pNewRoom = pGame->m_rooms.Find(pRoom->m_exits[idx].room);
      ASSERT_VALID(pNewRoom);
      walk.walking = FALSE;
      walk.pos = pRoom->m_exits[idx].startpos;
      walk.room = pNewRoom->m_ID;
      path.stops.Add( walk );
      
      // Process new room
      BOOL ok = CalcComplexWalk(pGame,pPlayer,pNewRoom );
      if( ok ) return TRUE; // goal was found, exit now...

      // Goal was not found, pop the last
      // pushed path and try next one...
      path.stops.RemoveAt(path.stops.GetUpperBound());
      path.stops.RemoveAt(path.stops.GetUpperBound());
      // Ready to try out next exit in room 
      idx++;
   };   

   // No exits lead to goal from this room,
   // signal this...
   return FALSE;
}
