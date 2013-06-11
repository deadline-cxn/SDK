#if !defined(AFX_PCODE_H__836BD723_3F8A_11D2_87A7_0080AD509054__INCLUDED_)
#define AFX_PCODE_H__836BD723_3F8A_11D2_87A7_0080AD509054__INCLUDED_
/*
 *
 *   Global types and definitions for all compiler related code
 *
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef long CODEPTR;
typedef long DATAPTR;

// Opcodes
typedef enum { 
   CODE_PROGRAMINIT, CODE_PROGRAMEXIT,
   CODE_CALL, CODE_STACKSUB, CODE_RETURN,
   CODE_CLEAR,
   CODE_LOADCONST, CODE_LOADVAR, CODE_LOADTHIS, 
   CODE_LOADOBJECT, CODE_LOADPROP,
   CODE_PUSH, 
   CODE_POPADD, CODE_POPSUB, CODE_POPMUL, CODE_POPDIV, CODE_POPAND, CODE_POPOR,
   CODE_STORE,
   CODE_PUTLABEL, CODE_JUMPTO, CODE_IFJUMPTO, CODE_IFNOTJUMPTO,
   CODE_EQUAL, CODE_EQUAL_NOT,
   CODE_GREATER, CODE_LESS,
   CODE_EQUAL_GREATER, CODE_EQUAL_LESS,
   CODE_LOADOFFSET, CODE_LOADPOINTER, CODE_STOREPOINTER,
   CODE_BRACKET_OFFSET, CODE_LOAD_ARRAY_ELEMENT, CODE_SAVE_ARRAY_ELEMENT,
   CODE_INCVAR, CODE_DECVAR,
   CODE_LAST
} EMITCODE;

#if _DEBUG
// These strings are defined during DEBUG builds only to allow
// you to print the opcdes during VM execution
// (They must closely match the EMITCODE enum above!!!!)
static LPCTSTR emitcodestr[] = {
   "CODE_PROGRAMINIT","CODE_PROGRAMEXIT",
   "CODE_CALL","CODE_STACKSUB","CODE_RETURN",
   "CODE_CLEAR",
   "CODE_LOADCONST","CODE_LOADVAR", "CODE_LOADTHIS",
   "CODE_LOADOBJECT","CODE_LOADPROP",
   "CODE_PUSH",
   "CODE_POPADD","CODE_POPSUB","CODE_POPMUL","CODE_POPDIV","CODE_POPAND","CODE_POPOR",
   "CODE_STORE",
   "CODE_PUTLABEL","CODE_JUMPTO","CODE_IFJUMPTO","CODE_IFNOTJUMPTO",
   "CODE_EQUAL","CODE_EQUAL_NOT",
   "CODE_GREATER","CODE_LESS",
   "CODE_EQUAL_GREATER","CODE_EQUAL_LESS",
   "CODE_LOADOFFSET","CODE_LOADPOINTER", "CODE_STOREPOINTER",
   "CODE_BRACKET_OFFSET","CODE_LOAD_ARRAY_ELEMENT","CODE_SAVE_ARRAY_ELEMENT",
   "CODE_INCVAR","CODE_DECVAR",
   "CODE_LAST"
};
#endif

// Variable types
// (Must closely match the strings 'varnames' defined
//  in Compiler.Cpp)
typedef enum {
   VAR_UNKNOWN = 0,
   VAR_VOID,
   VAR_LONG,
   VAR_STRING,
   VAR_FLOAT,
   //
   VAR_FUNCTION,
   VAR_OBJECT,
   VAR_LAST
} SYMBTYPE;

// Defined the available event
// (must closely match the strings 'eventnames' defined
//  in Compiler.Cpp)
typedef enum {
   EVENT_INIT,
   EVENT_DESTROY,
   EVENT_SPAWN,
   EVENT_DIE,
   //
   EVENT_TICK,
   EVENT_TOUCH,
   EVENT_NEAR,
   EVENT_ENTERROOM,
   EVENT_EXITROOM,
   //
   EVENT_EXAMINE,
   EVENT_TAKE,
   EVENT_GIVE,
   EVENT_DROP,
   EVENT_HIT,
   EVENT_X,
   EVENT_TALK,
   //
} EVENTTYPE;


// Defines the current AI types capable of setting an object
typedef enum { 
   STAND, 
   ANIMSEQ,
   WATCH, 
   WALKING,
} AITYPE;


#define MAX_EVENTS 20  // maximum event handlers for each object

typedef struct {
   TCHAR Name[32];
   long NumEvents;                // # of event defined
   EVENTTYPE Events[MAX_EVENTS];  // supported event-types
   CODEPTR EventPtrs[MAX_EVENTS]; // index in code for event handler
} TEventMap;

#define MAX_STATES 20  // maximum states for each object

typedef struct {
   TCHAR Name[32];
   CString statecode[MAX_STATES];
} TStateMap;


#endif // !defined(AFX_PCODE_H__836BD723_3F8A_11D2_87A7_0080AD509054__INCLUDED_)

