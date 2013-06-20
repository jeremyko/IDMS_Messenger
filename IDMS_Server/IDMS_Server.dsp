# Microsoft Developer Studio Project File - Name="IDMS_Server" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=IDMS_Server - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IDMS_Server.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IDMS_Server.mak" CFG="IDMS_Server - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IDMS_Server - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "IDMS_Server - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/IDMS_MSN_NEW/NEWMESSENGER/IDMS_Server", SGAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IDMS_Server - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/IDMS_Server"
# PROP Intermediate_Dir "../Release/IDMS_Server"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /ZI /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FAcs /FR /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /base:"0x64000000" /subsystem:windows /map /debug /machine:I386 /out:"../BinRelease/IDMS_Server.exe"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "IDMS_Server - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/IDMS_Server"
# PROP Intermediate_Dir "../Debug/IDMS_Server"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /map /debug /machine:I386 /out:"../BinDebug/IDMS_Server.exe"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "IDMS_Server - Win32 Release"
# Name "IDMS_Server - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ControlPos.cpp
# End Source File
# Begin Source File

SOURCE=.\DbLoginDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FoodMenuDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IDMS_Server.cpp
# End Source File
# Begin Source File

SOURCE=.\IDMS_Server.rc
# End Source File
# Begin Source File

SOURCE=.\IDMS_ServerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\INIManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\MemoryPool.cpp
# End Source File
# Begin Source File

SOURCE=.\msjexhnd.cpp
# End Source File
# Begin Source File

SOURCE=.\OraObjectArray.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\ThreadSockAsyncServer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Server_IOCP\AllCommonDefines.h
# End Source File
# Begin Source File

SOURCE=.\AllSockDefines.h
# End Source File
# Begin Source File

SOURCE=.\CatchExcept.h
# End Source File
# Begin Source File

SOURCE=.\ControlPos.h
# End Source File
# Begin Source File

SOURCE=.\DbLoginDlg.h
# End Source File
# Begin Source File

SOURCE=.\FoodMenuDlg.h
# End Source File
# Begin Source File

SOURCE=.\IDMS_Server.h
# End Source File
# Begin Source File

SOURCE=.\IDMS_ServerDlg.h
# End Source File
# Begin Source File

SOURCE=.\INIManager.h
# End Source File
# Begin Source File

SOURCE=.\Log.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool.h
# End Source File
# Begin Source File

SOURCE=.\msjexhnd.h
# End Source File
# Begin Source File

SOURCE=.\OraMultithrd.h
# End Source File
# Begin Source File

SOURCE=.\OraObjectArray.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SP_Utility.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\thread_pool.h
# End Source File
# Begin Source File

SOURCE=.\ThreadSockAsyncServer.h
# End Source File
# Begin Source File

SOURCE=..\Server_IOCP\tpipv6.h
# End Source File
# Begin Source File

SOURCE=.\utility.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\IDMS_IDMS_Server.ico
# End Source File
# Begin Source File

SOURCE=.\res\IDMS_Server.ico
# End Source File
# Begin Source File

SOURCE=.\res\IDMS_Server.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
