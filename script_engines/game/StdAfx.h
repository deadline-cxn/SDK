// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__F7C577E9_39DF_11D2_879C_0080AD509054__INCLUDED_)
#define AFX_STDAFX_H__F7C577E9_39DF_11D2_879C_0080AD509054__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3drm.lib")

#define ATLASSERT void(0)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		 // MFC support for Internet Explorer 4 Common Controls
#include <afxtempl.h>		 // MFC templates

#include <ctype.h>

#include "Global.h"
#include "Ini.h"
#include "FixedString.h"

#include "Misc\Misc.h"
#include "Misc\Debug.h"

#include "Picture\XPicture.h"
#include "Picture\XILBM.h"

#include "Direct\DDraw.h"
#include "Direct\DSurface.h"
#include "Direct\DInput.h"
#include "Direct\DSound.h"

#include "Compiler\pcode.h"

#include "Cg\G_Object.h"

#include "Misc\StateMachine.h"
#include "Compiler\vm.h"

#include "Cg\G_Room.h"
#include "Cg\G_Prop.h"
#include "Cg\G_Item.h"
#include "Cg\G_Character.h"
#include "Cg\G_Hero.h"

#include "Controller.h"

#include "Yacc\Script.h"
#include "Compiler\Compiler.h"

#include "Game.h"


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__F7C577E9_39DF_11D2_879C_0080AD509054__INCLUDED_)
