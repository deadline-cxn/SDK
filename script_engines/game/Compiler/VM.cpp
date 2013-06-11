// VM.cpp: implementation of the CVirtualMachine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVirtualMachine::CVirtualMachine()
{
   m_eventmap = NULL;
   m_codebase = NULL;
   m_database = NULL;
   //
   m_bIdle = TRUE;
   m_tick.bInQueue = FALSE;
   m_tick.nFreq = 0;
   m_bInError = FALSE;
   m_stack = new DATAPTR[MAXSTACK];
}

CVirtualMachine::~CVirtualMachine()
{
   int i;
   //
   for( i=0; i<m_strings.GetSize(); i++ ) delete (CString *)m_strings[i];
   m_strings.RemoveAll();
   //
   if( m_stack!=NULL ) delete [] m_stack;
   m_stack = NULL;
}


//////////////////////////////////////////////////////////////////////
// Helper functions

BOOL CVirtualMachine::Bind(CController *pGame,
                           CG_Item *pObject,
                           LPCTSTR Title,
                           TEventMap *maps, DWORD mapsize,
                           CODEPTR *codebase, DWORD codesize,
                           DATAPTR *database, DWORD datasize)
// You must call this function to bind the VM to an event map (code functions).
// This will bind the defined object's Event Maps in the loaded code to the
// VM. It will also initialize all the code and data pointers.
{
   ASSERT_VALID(pGame);
   ASSERT_VALID(pObject);
   ASSERT(AfxIsValidString(Title));
   ASSERT(AfxIsValidAddress(maps,mapsize*sizeof(TEventMap),FALSE));
   ASSERT(AfxIsValidAddress(codebase,codesize,FALSE));
   ASSERT(AfxIsValidAddress(database,datasize,FALSE));
   
   //
   // Bind Virtual Machine to object
   //
   m_pGame = pGame;
   m_pObject = pObject;
   //
   // Bind eventmap to VM
   //
   // Find an event map with the correct title
   CString sTitle( Title );
   sTitle.MakeUpper();
   BOOL bFound = FALSE;
   for( DWORD i=0; i<mapsize; i++ ) {
      if( sTitle == maps->Name ) { bFound=TRUE; break; };
      maps++;
   };
   if( !bFound ) return FALSE;
   // Bind VM code and data stores...
   m_eventmap = maps;
   m_codebase = codebase;
   m_coderoof = codesize/sizeof(CODEPTR);
   m_database = database;
   m_dataroof = datasize/sizeof(DATAPTR);
   // Done
   return TRUE;
}

BOOL CVirtualMachine::PostMessage(EVENTTYPE Event, LPVOID Sender/*=NULL*/, long Param1, long Param2)
// Post a message to the internal message queue of the
// virtual machine.
{
   if( m_eventmap==NULL ) return TRUE; // OK: not bound
   if( m_bInError ) return FALSE;
   if( m_tick.bInQueue && (Event==EVENT_TICK) ) return TRUE; // OK: already a timer tick in queue
   // Add message to queue
   TMESSAGE msg;
   msg.event = Event;
   msg.sender = Sender;
   msg.param1 = Param1;
   msg.param2 = Param2;
   msg.time = KfxGetTime();
   m_msgqueue.AddTail(msg);
   // Some book-keeping
   m_tick.bInQueue = (Event==EVENT_TICK);
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Execute p-code

BOOL CVirtualMachine::Schedule(long slice)
// This is the task scheduler.
// The argument 'slice' is the approximate of how many cycles
// the process is allowed to use (actually how many opcodes it
// can execute), before it has to give up its time-slice.
// The scheduler will grab a message from the message queue
// and map the message to a code entry-point. The code will
// then be executed.
{
   if( m_eventmap==NULL ) return TRUE;

   ASSERT(m_codebase); // We have code?
   ASSERT(m_database); // We have variable space?
   ASSERT(slice>0);    // We are executing any opcodes?

   if( m_bInError ) return FALSE; // Process in ERROR?
   if( m_codebase==NULL ) return FALSE;

   
   //
   // TICK (timer) handler
   //
   if( m_tick.nFreq>0 ) {
      if( --m_tick.nCnt <= 0 ) {
         PostMessage(EVENT_TICK);
         m_tick.nCnt = m_tick.nFreq; // reset timer
      };
   };

   //
   // Ready to process a new message?
   //  If so, make sure we initialize VM properly.
   //
   if( m_bIdle ) {
      if( m_msgqueue.IsEmpty() ) return TRUE; // nothing to do!!!
      TMESSAGE msg;
      msg = m_msgqueue.RemoveHead();
      // See if there is a handler defined for this event
      BOOL bFound = FALSE;
      for( int i=0; i<m_eventmap->NumEvents; i++ ) {
         if( m_eventmap->Events[i] == msg.event ) { bFound=TRUE; break; };
      };
      if( !bFound ) return TRUE; // give up timeslice and
                                 // try to locate new event next time
      if( msg.event==EVENT_TICK ) m_tick.bInQueue = FALSE;
      m_SenderRef = (CG_Item *)msg.sender;
      //
      // Initialize
      //
      CODEPTR ptr;
      ptr = m_eventmap->EventPtrs[i];
      ASSERT( ptr<=m_coderoof );
      // Set registers
      m_cp = m_codebase + ptr;
      m_ax = 0;
      m_sp = 0;
      m_bIdle = FALSE; // not ready for event messages any longer...
   };
  
   //
   // Run some code...
   //
   m_bInError = !ExecuteCode(slice);
   return m_bInError;
};

BOOL CVirtualMachine::ExecuteCode(long slice)
// Execute a number of opcodes.
// The argument 'slice' determines how many opcodes
// can be execution before the function will return.
// Returns TRUE if everything was OK, returns FALSE
// if an error occoured. If FALSE is returned the
// process will be forced to halt.
{
#define GETOPCODE (EMITCODE)*m_cp++
#define GETSYMBTYPE (SYMBTYPE)*m_cp++
#define ASSERT_DATAPTR(x) ASSERT( ((DATAPTR)x>=0) && ((DATAPTR)x<=m_dataroof) );
#define ASSERT_CODEPTR(x) ASSERT( ((CODEPTR)x>=0) && ((CODEPTR)x<=m_coderoof) );
#define ASSERT_SYMBTYPE(x) ASSERT( ((SYMBTYPE)x>=0) && ((SYMBTYPE)x<VAR_LAST) );
#define ASSERT_STRING(x) ASSERT( AfxIsValidString((LPCTSTR)*(CString *)x) );

   DWORD val;
   DWORD arg1;
   SYMBTYPE vartype;

   while( slice-- > 0 ) {
#if _DEBUG
      EMITCODE code = GETOPCODE;
      ASSERT( (code>=0)&&(code<CODE_LAST) );
      //TRACE1(" - %s\n", emitcodestr[code]);
      switch( code ) {
#else
      switch( GETOPCODE ) {
#endif
      case CODE_PROGRAMINIT:
         break;
      case CODE_PROGRAMEXIT:
         m_bIdle = TRUE; // End of program! Signal that we want
                         // new event from msg-queue
         GarbageCollector();
         return TRUE;

      case CODE_CLEAR:
         m_ax = 0L;
         break;
      
      case CODE_LOADOFFSET:
         {
            arg1 = *m_cp++;
            ASSERT_DATAPTR( arg1 );
            CString str((LPCTSTR)(m_database + arg1));
            m_ax = ValidateStackString( &str );
         };
         break;
      case CODE_LOADCONST:
         m_ax = *m_cp++;
         break;
      case CODE_LOADTHIS:
         ASSERT_VALID(m_pObject);
         m_ax = (DATAPTR)m_pObject->m_ID;
         break;
      case CODE_LOADVAR:
         vartype = GETSYMBTYPE;
         arg1 = *m_cp++;
         ASSERT_DATAPTR(arg1);
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            val = m_database[ arg1 ];
            break;
         case VAR_STRING:
            ValidateDataString(arg1);
            val = m_database[ arg1 ];
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         m_ax = val;
         break;

      case CODE_STORE:
         vartype = GETSYMBTYPE;
         arg1 = *m_cp++;
         ASSERT_DATAPTR(arg1);
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_database[ arg1 ] = m_ax;
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            ValidateDataString(arg1,(CString *)m_ax);
            break;
         };
         break;
      case CODE_STOREPOINTER:
         arg1 = *m_cp++;
         ASSERT_DATAPTR( m_ax );
         ASSERT_DATAPTR( arg1 );
         m_database[ arg1 ] = m_ax;
         break;

      case CODE_LOADOBJECT:
         m_ax = *m_cp++;  // get object m_ID value
         break;
      case CODE_LOADPROP:
         {
            CG_Object *obj;
            obj = m_pGame->m_objects.Find((CG_ID)m_ax);
            ASSERT_VALID(obj);
            if( obj==NULL ) {
               m_ax = 0;
               return FALSE; // Error. Object not found
            };
            val = *m_cp++;
            LPCTSTR PropName = (LPCTSTR)&m_database[val];
            ASSERT(AfxIsValidString(PropName));
            LPVOID ptr;
            if( obj->GetProp(PropName,ptr,&vartype)==FALSE) {
               // not found; oh well...
               m_ax = 0;
               break;
            };
            ASSERT_SYMBTYPE(vartype);
            switch( vartype ) {
            case VAR_LONG:
               val = *(LPDWORD)ptr;
               break;
            case VAR_STRING:
               val = ValidateStackString((CString *)ptr);
               break;
            default:
               ASSERT(FALSE);
               return FALSE;
            };
            m_ax = val;
         };
         break;

      case CODE_INCVAR:
         vartype = GETSYMBTYPE;
         arg1 = *m_cp++;
         ASSERT_DATAPTR( arg1 );
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_database[ arg1 ]++;
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_DECVAR:
         vartype = GETSYMBTYPE;
         arg1 = *m_cp++;
         ASSERT_DATAPTR( arg1 );
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_database[ arg1 ]--;
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;

      case CODE_PUSH:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            Push( m_ax );
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            Push( ValidateStackString((CString *)m_ax) );
            break;
         };
         break;
      case CODE_POPADD:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax += Pop();
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            CString str( CString(*(CString *)Pop() + *(CString *)m_ax) );
            m_ax = ValidateStackString(&str);
            break;
         };
         break;
      case CODE_POPSUB:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax -= Pop();
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_POPMUL:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax *= Pop();
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_POPDIV:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            val = Pop();
            if( val==0 ) {               
               ASSERT(FALSE);
               return FALSE; // Error. Division by zero
            };
            m_ax /= val;
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_POPAND:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax &= Pop();
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_POPOR:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax |= Pop();
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;

      case CODE_GREATER:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax = (BOOL)(m_ax-Pop() < 0);
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            m_ax = (BOOL)( *(CString *)m_ax < *(CString *)Pop() );
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_LESS:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax = (BOOL)(m_ax-Pop() > 0);
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            m_ax = (BOOL)( *(CString *)m_ax > *(CString *)Pop() );
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_EQUAL:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax = (BOOL)(m_ax-Pop() == 0);
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            m_ax = (BOOL)( *(CString *)m_ax == *(CString *)Pop() );
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_EQUAL_NOT:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax = (BOOL)(m_ax-Pop() != 0);
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            m_ax = (BOOL)( *(CString *)m_ax != *(CString *)Pop() );
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_EQUAL_GREATER:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax = (BOOL)(m_ax-Pop() <= 0);
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            m_ax = (BOOL)( *(CString *)m_ax <= *(CString *)Pop() );
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;
      case CODE_EQUAL_LESS:
         vartype = GETSYMBTYPE;
         ASSERT_SYMBTYPE(vartype);
         switch( vartype ) {
         case VAR_LONG:
            m_ax = (BOOL)(m_ax-Pop() >= 0);
            break;
         case VAR_STRING:
            ASSERT_STRING(m_ax);
            m_ax = (BOOL)( *(CString *)m_ax >= *(CString *)Pop() );
            break;
         default:
            ASSERT(FALSE);
            return FALSE;
         };
         break;

      case CODE_JUMPTO:
         arg1 = *m_cp++;
         ASSERT_CODEPTR( arg1 );
         m_cp = m_codebase + arg1;
         break;
      case CODE_IFJUMPTO:
         arg1 = *m_cp++;
         ASSERT_CODEPTR( arg1 );
         if( m_ax!=0L ) m_cp = m_codebase + arg1;
         break;
      case CODE_IFNOTJUMPTO:
         arg1 = *m_cp++;
         ASSERT_CODEPTR( arg1 );
         if( m_ax==0L ) m_cp = m_codebase + arg1;
         break;

      case CODE_CALL:
         arg1 = *m_cp++;
         // All function arguments are grabbed within each
         // function implementation, so we don't need to
         // pop arguments from the stack here (see the CODE_STACKSUB
         // opcode below).
         switch( arg1 ) {
         case 0:  m_ax = FUNC_PlaySound(); break;
         case 1:  m_ax = FUNC_Int2Str(); break;
         case 2:  m_ax = FUNC_Str2Int(); break;
         case 3:  m_ax = FUNC_Log(); break;
         case 4:  m_ax = FUNC_PostMessage(); break;
         case 5:  m_ax = FUNC_Sleep(); break;
         case 6:  m_ax = FUNC_UCase(); break;
         case 7:  m_ax = FUNC_LCase(); break;
         case 8:  m_ax = FUNC_Left(); break;
         case 9:  m_ax = FUNC_Mid(); break;
         case 10: m_ax = FUNC_Right(); break;
         case 11: m_ax = FUNC_SetTimer(); break;
         case 12: m_ax = FUNC_SwitchState(); break;
         default:
            ASSERT(FALSE);
            break;
         };
         break;
      case CODE_STACKSUB:
         // This opcode usually is fired after a CODE_CALL
         // to set the stack back to where it was (usually
         // after an argument call to get to the stack-frame
         // before arguments were pushed).
         arg1 = *m_cp++;
         m_sp -= arg1;
         break;

      default:
         // UNSUPPORTED OPCODE!?!
#if _DEBUG
      TRACE1("Unsupported opcode - %s\n", emitcodestr[code]);
#endif
         ASSERT(FALSE);
         return FALSE;
      };
   };
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Interpreter helper functions

void CVirtualMachine::Push(DATAPTR value)
// Implementation of push data on the stack
{
   m_stack[m_sp++] = value;
   ASSERT( m_sp>0 && m_sp<=MAXSTACK );
}

DATAPTR CVirtualMachine::Pop()
// Implementation of pop data from the stack
{
   ASSERT( m_sp>0 );
   return m_stack[--m_sp];
}

BOOL CVirtualMachine::GarbageCollector()
// Our crude garbage collector.
// So far we only need to free the strings (CString objects) we
// temporarily have used during the code execution.
{
   int i;
   for( i=0; i<m_garbage_strings.GetSize(); i++ ) delete (CString *)m_garbage_strings[i];
   m_garbage_strings.RemoveAll();
   return TRUE;
}

CString * CVirtualMachine::ValidateDataString(DWORD idx, CString *defaultvalue/*=NULL*/)
// Temporary string helper function.
// A function which replaces a string in the DATA segment (usually a string variable).
//
// The DATA segment strings are dynamically created. Strings will be destroyed
// only when the VM goes out of scope. The member variable 'm_strings' is
// used to hold the CString object, while 'm_database' holds a reference pointer
// only.
{
   if( m_database[ idx ] == 0L ) {
      CString *str = new CString(""); // must be empty to avoid NULL once again!
      m_strings.Add( str );
      m_database[ idx ] = (DWORD)str;
   };
   if( defaultvalue!=NULL ) *(CString *)m_database[ idx ] = *defaultvalue;
   return (CString *)m_database[ idx ];
}

DWORD CVirtualMachine::ValidateStackString(CString *defaultvalue/*=NULL*/)
// Temporary string helper function.
// A function which creates a copy of an existing string.
//
// We need to create a copy of the string because we may modify
// the string later.
// The new string is added to the garbage collector.
{
   CString *str;
   if( defaultvalue==NULL )
      str = new CString;
   else
      str = new CString( *defaultvalue );
   m_garbage_strings.Add(str);
   return (DWORD)str;
}



//////////////////////////////////////////////////////////////////////
// Build-in functions

DATAPTR CVirtualMachine::GetArg(int Idx)
// Helper function to retrieve an function call argument
// which was pushed on the stack.
// NOTE: Arguments should retrieved in retrieved
//       in reverse order.
{
   ASSERT(m_sp-Idx>=0);
   if( m_sp-Idx < 0 ) return 0;
   return m_stack[ m_sp - Idx ];
}

DATAPTR CVirtualMachine::FUNC_PlaySound()
{
   CString *str = (CString *)GetArg(1);
   TRACE1( "PlaySound %s\n", (LPCTSTR)*str);
   // TODO: Add implementation
   return 0;
}

DATAPTR CVirtualMachine::FUNC_Log()
{
   CString *arg1 = (CString *)GetArg(1);
   ASSERT( AfxIsValidString(*arg1) );
   //
   CApp *app = (CApp *)AfxGetApp();
   return app->m_debug.Print(*arg1);
}

DATAPTR CVirtualMachine::FUNC_Int2Str()
{
   DATAPTR arg1 = GetArg(1);
   CString s;
   s.Format(_T("%d"),arg1);
   return (DATAPTR)ValidateStackString(&s);
}

DATAPTR CVirtualMachine::FUNC_Str2Int()
{
   CString *arg1 = (CString *)GetArg(1);
   ASSERT( AfxIsValidString(*arg1) );
   return _ttol( *arg1 );
}

DATAPTR CVirtualMachine::FUNC_UCase()
{
   CString *arg1 = (CString *)GetArg(1);
   ASSERT( AfxIsValidString(*arg1) );
   CString s( *arg1 );
   s.MakeUpper();
   return (DATAPTR)ValidateStackString(&s);
}

DATAPTR CVirtualMachine::FUNC_LCase()
{
   CString *arg1 = (CString *)GetArg(1);
   ASSERT( AfxIsValidString(*arg1) );
   CString s( *arg1 );
   s.MakeLower();
   return (DATAPTR)ValidateStackString(&s);
}

DATAPTR CVirtualMachine::FUNC_Left()
{
   CString *arg1 = (CString *)GetArg(2);
   DATAPTR arg2 = GetArg(1);
   ASSERT( AfxIsValidString(*arg1) );
   CString s(*arg1);
   s = s.Left(arg2);
   return (DATAPTR)ValidateStackString(&s);
}

DATAPTR CVirtualMachine::FUNC_Right()
{
   CString *arg1 = (CString *)GetArg(2);
   DATAPTR arg2 = GetArg(1);
   ASSERT( AfxIsValidString(*arg1) );
   CString s(*arg1);
   CString tmp;
   tmp = s.Right(arg2);
   return (DATAPTR)ValidateStackString(&tmp);
}

DATAPTR CVirtualMachine::FUNC_Mid()
{
   CString *arg1 = (CString *)GetArg(3);
   DATAPTR arg2 = GetArg(2);
   DATAPTR arg3 = GetArg(1);
   ASSERT( AfxIsValidString(*arg1) );
   CString s(*arg1);
   s = s.Mid(arg2,arg3);
   return (DATAPTR)ValidateStackString(&s);
}

DATAPTR CVirtualMachine::FUNC_SetTimer()
{
   DATAPTR arg1 = GetArg(1);
   if( arg1<0 ) arg1 = 0;
   m_tick.nFreq = m_tick.nCnt = (short)arg1;
   return 0;
}

DATAPTR CVirtualMachine::FUNC_Sleep()
{
   DATAPTR arg1 = GetArg(1);
   if( arg1<0 ) arg1 = 0;
   // TODO: Add implementation
   return 0;
}

DATAPTR CVirtualMachine::FUNC_PostMessage()
{
   ASSERT_VALID(m_pObject);
   DATAPTR arg1 = GetArg(3);
   DATAPTR arg2 = GetArg(2);
   DATAPTR arg3 = GetArg(1);
   PostMessage((EVENTTYPE)arg1,m_pObject,arg2,arg3);
   return 0;
}

DATAPTR CVirtualMachine::FUNC_SwitchState()
{
   ASSERT_VALID(m_pObject);
   DATAPTR arg1 = GetArg(1);
   if( arg1<0 ) arg1 = 0;
   m_pObject->m_state.EnterNewState(arg1);
   return 0;
}

DATAPTR CVirtualMachine::FUNC_Sender()
{
   return (DATAPTR)m_SenderRef;
}
