# Microsoft Developer Studio Project File - Name="Game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Game - Win32 Debug and Browser
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Game.mak" CFG="Game - Win32 Debug and Browser"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Game - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Game - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Game - Win32 Debug and Browser" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Game - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "C:\Temp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x406 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "C:\Temp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x406 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "Game - Win32 Debug and Browser"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Game___Win32_Debug_and_Browser"
# PROP BASE Intermediate_Dir "Game___Win32_Debug_and_Browser"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Browser"
# PROP Intermediate_Dir "C:\Temp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x406 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Game - Win32 Release"
# Name "Game - Win32 Debug"
# Name "Game - Win32 Debug and Browser"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "DirectX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Direct\DDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Direct\DInput.cpp
# End Source File
# Begin Source File

SOURCE=.\Direct\DSound.cpp
# End Source File
# Begin Source File

SOURCE=.\Direct\DSurface.cpp
# End Source File
# End Group
# Begin Group "GameObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CG\G_Character.cpp
# End Source File
# Begin Source File

SOURCE=.\CG\G_Hero.cpp
# End Source File
# Begin Source File

SOURCE=.\CG\G_Item.cpp
# End Source File
# Begin Source File

SOURCE=.\CG\G_Object.cpp
# End Source File
# Begin Source File

SOURCE=.\CG\G_Prop.cpp
# End Source File
# Begin Source File

SOURCE=.\CG\G_Room.cpp
# End Source File
# Begin Source File

SOURCE=.\Misc\StateMachine.cpp
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Misc\Debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\Global.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\Ini.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\LogFile.cpp
# End Source File
# Begin Source File

SOURCE=Misc\Misc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\Picture\XILBM.cpp
# End Source File
# End Group
# Begin Group "Compiler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Compiler\Compiler.cpp
# End Source File
# Begin Source File

SOURCE=.\Yacc\script.cpp
# End Source File
# Begin Source File

SOURCE=.\Compiler\VM.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Controller.cpp
# End Source File
# Begin Source File

SOURCE=.\Controller2.cpp
# End Source File
# Begin Source File

SOURCE=.\Game.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "DirectX .h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Direct\DDraw.h
# End Source File
# Begin Source File

SOURCE=.\Direct\DInput.h
# End Source File
# Begin Source File

SOURCE=.\Direct\DSound.h
# End Source File
# Begin Source File

SOURCE=.\Direct\DSurface.h
# End Source File
# End Group
# Begin Group "GameObjects .h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CG\G_Character.h
# End Source File
# Begin Source File

SOURCE=.\CG\G_Hero.h
# End Source File
# Begin Source File

SOURCE=.\CG\G_Item.h
# End Source File
# Begin Source File

SOURCE=.\CG\G_Object.h
# End Source File
# Begin Source File

SOURCE=.\CG\G_Prop.h
# End Source File
# Begin Source File

SOURCE=.\CG\G_Room.h
# End Source File
# Begin Source File

SOURCE=.\Misc\StateMachine.h
# End Source File
# End Group
# Begin Group "Compiler .h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Compiler\Compiler.h
# End Source File
# Begin Source File

SOURCE=.\Compiler\pcode.h
# End Source File
# Begin Source File

SOURCE=.\Yacc\script.h
# End Source File
# Begin Source File

SOURCE=.\Compiler\VM.h
# End Source File
# End Group
# Begin Group "Misc .h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Misc\Debug.h
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\Global.h
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\Ini.h
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\LogFile.h
# End Source File
# Begin Source File

SOURCE=.\Misc\Misc.h
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\Picture\XILBM.h
# End Source File
# Begin Source File

SOURCE=..\..\Source\MFC\Common\Picture\XPicture.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Controller.h
# End Source File
# Begin Source File

SOURCE=.\Game.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Game.ico
# End Source File
# Begin Source File

SOURCE=.\Game.rc
# End Source File
# Begin Source File

SOURCE=.\res\Game.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
