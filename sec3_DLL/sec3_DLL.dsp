# Microsoft Developer Studio Project File - Name="sec3_DLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sec3_DLL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sec3_DLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sec3_DLL.mak" CFG="sec3_DLL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sec3_DLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sec3_DLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/idms_msn/IDMS_MSN/sec3_DLL", CBBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sec3_DLL - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/Sec3"
# PROP Intermediate_Dir "../Release/Sec3"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 imm32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../BinRelease/sec3_DLL.dll"

!ELSEIF  "$(CFG)" == "sec3_DLL - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/sec3_DLL"
# PROP Intermediate_Dir "../Debug/sec3_DLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 imm32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../BinDebug/sec3_DLL.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sec3_DLL - Win32 Release"
# Name "sec3_DLL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BtnST.cpp
# End Source File
# Begin Source File

SOURCE=.\ButtonIconSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\CalendarCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\CalNumberEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ControlPos.cpp
# End Source File
# Begin Source File

SOURCE=.\DataManager.cpp
# End Source File
# Begin Source File

SOURCE=.\IconSelector.cpp
# End Source File
# Begin Source File

SOURCE=.\KoreanDate.cpp
# End Source File
# Begin Source File

SOURCE=.\MemoItemDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sec3_DLL.cpp
# End Source File
# Begin Source File

SOURCE=.\sec3_DLL.def
# End Source File
# Begin Source File

SOURCE=.\sec3_DLL.rc
# End Source File
# Begin Source File

SOURCE=.\Sec3Dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BtnST.h
# End Source File
# Begin Source File

SOURCE=.\ButtonIconSelect.h
# End Source File
# Begin Source File

SOURCE=.\CalendarCtrl.h
# End Source File
# Begin Source File

SOURCE=.\CalNumberEdit.h
# End Source File
# Begin Source File

SOURCE=.\ControlPos.h
# End Source File
# Begin Source File

SOURCE=.\DataManager.h
# End Source File
# Begin Source File

SOURCE=.\ICalData.h
# End Source File
# Begin Source File

SOURCE=.\IconSelector.h
# End Source File
# Begin Source File

SOURCE=.\KoreanDate.h
# End Source File
# Begin Source File

SOURCE=.\MemDC.h
# End Source File
# Begin Source File

SOURCE=.\MemoItemDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\sec3_DLL.h
# End Source File
# Begin Source File

SOURCE=.\Sec3Dlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\_item01.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item02.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item03.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item04.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item05.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item06.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item07.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item08.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item09.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item10.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item11.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item12.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item13.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item14.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item15.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item16.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item17.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item18.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item19.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item20.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item21.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item22.ico
# End Source File
# Begin Source File

SOURCE=.\res\_item23.ico
# End Source File
# Begin Source File

SOURCE=.\res\MemoCal.ico
# End Source File
# Begin Source File

SOURCE=.\res\monthvw.ico
# End Source File
# Begin Source File

SOURCE=.\res\sec3_DLL.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
