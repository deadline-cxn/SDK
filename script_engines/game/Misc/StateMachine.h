// StateMachine.h: interface for the CStateMachine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATEMACHINE_H__7ABB2D98_48C5_11D2_87BD_0080AD509054__INCLUDED_)
#define AFX_STATEMACHINE_H__7ABB2D98_48C5_11D2_87BD_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CController;
class CG_Room;
class CG_Item;

class CStateMachine : public CObject  
{
public:
	CStateMachine();
	virtual ~CStateMachine();

// Attributes
public:
   //
   struct tagWAIT {
      CG_TIME target;    // time we're waiting for
   } wait;

   struct tagWALK {
      float x;           // position destination
      float y;
      float delta_x;
      float delta_y;
      long steps;        // number of steps to target position
   } walk;

   typedef struct tagWALK_STOP {
      CG_ID room;
      CPoint pos;
      BOOL   walking;
   } WALK_STOP;
   struct tagPATH {
      short   trips;       // number of trips to follow (size of 'stops')
      short   idx;         // current trip we're processing
      CPoint  destpos;     // final destination
      CG_ID   destroom;    // final room
      CArray< WALK_STOP, WALK_STOP & > stops;
   } path;

   struct tagANIM {
      CString name;        // name of animation to play
      int     frame;       // frame index
      BOOL    wait;        // wait for animation completion
   } anim;

   struct tagFOLLOW {
      CG_Item *target;     // target to follow (should be a player)
      long     distance;   // distance to keep to target
      short    cnt;
   } follow;


public:
   long m_nState;         // which state is active
protected:
   long m_nIndex;         // which index in state(ment)array
   TStateMap *m_statemap;

// Methods
public:
	BOOL Bind( TStateMap *map );
	BOOL EnterNewState( long idx );
	BOOL ResetStateAttributes();
	BOOL IsReadyForNewState();
	BOOL GetNewState( CController *pGame, CG_Item *pPlayer );
	BOOL ProcessState( CController *pGame, CG_Item *pPlayer );
   long GetCurrentState() const { return m_nState; };

protected:
	BOOL CalcComplexWalk( CController *pGame, CG_Item *pPlayer, CG_Room *pRoom );
	BOOL EvaluateValue( CG_Item *pPlayer, CString &name, SYMBTYPE &vartype, CString &output_str, DWORD &output_int );
	BOOL EvaluateExpression( CString &line, CG_Item *pPlayer );
	CString GetQuotedString( CString &str );
};

#endif // !defined(AFX_STATEMACHINE_H__7ABB2D98_48C5_11D2_87BD_0080AD509054__INCLUDED_)
