// VM.h: interface for the CVirtualMachine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VM_H__836BD723_3F8A_11D2_87A7_0080AD509054__INCLUDED_)
#define AFX_VM_H__836BD723_3F8A_11D2_87A7_0080AD509054__INCLUDED_

#include "pcode.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CController;
class CG_Item;

class CVirtualMachine : public CObject  
{
public:
	CVirtualMachine();
	virtual ~CVirtualMachine();

// Types
public:
#define MAXSTACK 200  // Maximum number of defined events handler
   // Defines a Message in the Message-queue
   typedef struct {
      EVENTTYPE event;
      LPVOID sender;
      long param1;
      long param2;
      DWORD time;
   } TMESSAGE;
   // Defines the message queue
   typedef CList< TMESSAGE, TMESSAGE & > CMessageQueue;

// Variables
protected:
   CController *m_pGame;
   CG_Item *m_pObject;     // Object who owns Virtal Machine
   TEventMap *m_eventmap;  // Pointer to EventMap being processed
   CODEPTR *m_codebase;
   DATAPTR *m_database;
   CODEPTR m_coderoof;
   DATAPTR m_dataroof;
   //
   CMessageQueue m_msgqueue;  // VM message queue
   BOOL m_bInError;  // VM has errored and should not be run anymore
   BOOL m_bIdle;     // VM is currently idle (will accept new msg)
   //
   struct TICK {
      short nFreq;    // Tick frequency
      short nCnt;     // Internal counter for tick post handler
      BOOL  bInQueue; // Msg-queue has a TICK event waiting in queue
   } m_tick;
   //
   CODEPTR  *m_cp;                 // VM Code Pointer
   DWORD     m_ax, m_sp;           // VM registers
   DATAPTR  *m_stack;              // VM stack
   CPtrArray m_strings;            // VM variables (string type only)
   CPtrArray m_garbage_strings;    // VM garbage collector (string type only)
   CG_Item  *m_SenderRef;

// Methods
public:
   BOOL Bind(CController *pGame, CG_Item *pObject, LPCTSTR Title, TEventMap *eventmap, DWORD eventsize, CODEPTR *codebase, DWORD codesize, DATAPTR *database, DWORD datasize);
	BOOL Schedule( long slice );
	BOOL PostMessage( EVENTTYPE Event, LPVOID Sender=NULL, long Param1=0, long Param2=0 );
   BOOL IsIdle() const { return m_bIdle; };
protected:
	DATAPTR GetArg( int Idx );
	DATAPTR FUNC_Int2Str(void);
	DATAPTR FUNC_Str2Int(void);
	DATAPTR FUNC_Log(void);
	DATAPTR FUNC_PlaySound(void);
	DATAPTR FUNC_UCase(void);
	DATAPTR FUNC_LCase(void);
	DATAPTR FUNC_Left(void);
	DATAPTR FUNC_Right(void);
	DATAPTR FUNC_Mid(void);
   DATAPTR FUNC_SetTimer(void);
   DATAPTR FUNC_Sleep(void);
   DATAPTR FUNC_PostMessage(void);
   DATAPTR FUNC_SwitchState(void);
   DATAPTR FUNC_Sender(void);
	//
   BOOL GarbageCollector();
	DWORD ValidateStackString( CString *defaultvalue=NULL );
   CString * ValidateDataString( DWORD idx, CString *defaultvalue=NULL );
	DATAPTR Pop();
	void Push( DATAPTR value );
	BOOL ExecuteCode( long slice );
};

#endif // !defined(AFX_VM_H__836BD723_3F8A_11D2_87A7_0080AD509054__INCLUDED_)
