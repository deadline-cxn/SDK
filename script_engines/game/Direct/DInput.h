// DInput.h: interface for the CDirectInput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DINPUT_H__780689A3_3AB2_11D2_879D_0080AD509054__INCLUDED_)
#define AFX_DINPUT_H__780689A3_3AB2_11D2_879D_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment(lib, "dinput.lib")
#include <dinput.h>


class CDirectInput : public CObject  
{
public:
	CDirectInput();
	virtual ~CDirectInput();

// Attributes
protected:
   LPDIRECTINPUT        m_lpDI; 
   LPDIRECTINPUTDEVICE  m_lpDIDevice; 
   char     KBState[256]; 

// Implementation
public:
	long Init( HWND hwnd );
	long Close();
	void Idle();
   BOOL IsKeyDown(short key) const { return (KBState[key] & 0x80); };

};

#endif // !defined(AFX_DINPUT_H__780689A3_3AB2_11D2_879D_0080AD509054__INCLUDED_)
