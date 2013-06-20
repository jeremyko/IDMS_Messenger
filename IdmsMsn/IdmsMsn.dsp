# Microsoft Developer Studio Project File - Name="IdmsMsn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=IdmsMsn - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IdmsMsn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IdmsMsn.mak" CFG="IdmsMsn - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IdmsMsn - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "IdmsMsn - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/IDMS_MSN_NEW/NEWMESSENGER/IdmsMsn", EBAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IdmsMsn - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release/client"
# PROP Intermediate_Dir "../Release/client"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FAcs /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /map /debug /machine:I386 /out:"../BinRelease/IdmsMsn.exe" /OPT:ICF /OPT:REF
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "IdmsMsn - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug/client"
# PROP Intermediate_Dir "../Debug/client"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"../BinDebug/IdmsMsn.exe"

!ENDIF 

# Begin Target

# Name "IdmsMsn - Win32 Release"
# Name "IdmsMsn - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AllMsgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AutoAppendMsgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BuddySlipMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatReplyEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatSession.cpp
# End Source File
# Begin Source File

SOURCE=.\ChatWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CheckedTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ChgPwdDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\ColourPicker.cpp
# End Source File
# Begin Source File

SOURCE=.\ColourPopup.cpp
# End Source File
# Begin Source File

SOURCE=..\common\ControlPos.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\DesWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\dib256.cpp
# End Source File
# Begin Source File

SOURCE=.\dibpal.cpp
# End Source File
# Begin Source File

SOURCE=.\EmoticonBase.cpp
# End Source File
# Begin Source File

SOURCE=.\EmoticonRichEditCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ExtTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\FileRecvDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FileSendDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTransSock.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupMngDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IDMSBitmapDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IDMSComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\IdmsMsn.cpp
# End Source File
# Begin Source File

SOURCE=.\IdmsMsn.rc
# End Source File
# Begin Source File

SOURCE=.\IdmsMsnDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageDataObject.cpp
# End Source File
# Begin Source File

SOURCE=.\INIManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ITCompDlgMng.cpp
# End Source File
# Begin Source File

SOURCE=.\LogInDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgBoxThread.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiChatWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MyButton.cpp
# End Source File
# Begin Source File

SOURCE=.\MyButtonMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\MyButtonUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\MyProgressCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\NewCompanionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RegNewUserDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SplitterControl.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\ThreadSockAsync.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewAllMsgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewBuddySlipMsg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\common\AllCommonDefines.h
# End Source File
# Begin Source File

SOURCE=.\AllMsgDlg.h
# End Source File
# Begin Source File

SOURCE=.\AutoAppendMsgDlg.h
# End Source File
# Begin Source File

SOURCE=.\BuddySlipMsg.h
# End Source File
# Begin Source File

SOURCE=.\ChatReplyEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChatSession.h
# End Source File
# Begin Source File

SOURCE=.\ChatWnd.h
# End Source File
# Begin Source File

SOURCE=.\CheckedTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ChgPwdDlg.h
# End Source File
# Begin Source File

SOURCE=.\ColorListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ColorStatic.h
# End Source File
# Begin Source File

SOURCE=.\ColourPicker.h
# End Source File
# Begin Source File

SOURCE=.\ColourPopup.h
# End Source File
# Begin Source File

SOURCE=..\common\ControlPos.h
# End Source File
# Begin Source File

SOURCE=.\ConvEdit.h
# End Source File
# Begin Source File

SOURCE=.\DesWrapper.h
# End Source File
# Begin Source File

SOURCE=.\dib256.h
# End Source File
# Begin Source File

SOURCE=.\dibpal.h
# End Source File
# Begin Source File

SOURCE=.\EmoticonBase.h
# End Source File
# Begin Source File

SOURCE=.\EmoticonRichEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ExtTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\FileRecvDlg.h
# End Source File
# Begin Source File

SOURCE=.\FileSendDlg.h
# End Source File
# Begin Source File

SOURCE=.\FileTransSock.h
# End Source File
# Begin Source File

SOURCE=.\GroupMngDlg.h
# End Source File
# Begin Source File

SOURCE=.\GSEncryptX.h
# End Source File
# Begin Source File

SOURCE=.\IDMSBitmapDialog.h
# End Source File
# Begin Source File

SOURCE=.\IDMSComboBox.h
# End Source File
# Begin Source File

SOURCE=.\IdmsMsn.h
# End Source File
# Begin Source File

SOURCE=.\IdmsMsnDlg.h
# End Source File
# Begin Source File

SOURCE=.\ImageDataObject.h
# End Source File
# Begin Source File

SOURCE=.\INIManager.h
# End Source File
# Begin Source File

SOURCE=.\ITCompDlgMng.h
# End Source File
# Begin Source File

SOURCE=.\LogInDlg.h
# End Source File
# Begin Source File

SOURCE=.\MemDC.h
# End Source File
# Begin Source File

SOURCE=.\MsgBoxThread.h
# End Source File
# Begin Source File

SOURCE=.\MsgDlg.h
# End Source File
# Begin Source File

SOURCE=.\MultiChatWnd.h
# End Source File
# Begin Source File

SOURCE=.\MyButton.h
# End Source File
# Begin Source File

SOURCE=.\MyButtonMenu.h
# End Source File
# Begin Source File

SOURCE=.\MyButtonUtil.h
# End Source File
# Begin Source File

SOURCE=.\MyProgressCtrl.h
# End Source File
# Begin Source File

SOURCE=.\NewCompanionDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptionDlg.h
# End Source File
# Begin Source File

SOURCE=.\ping.h
# End Source File
# Begin Source File

SOURCE=.\RegNewUserDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SP_Utility.h
# End Source File
# Begin Source File

SOURCE=.\SplitterControl.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\ThreadSockAsync.h
# End Source File
# Begin Source File

SOURCE=.\utility.h
# End Source File
# Begin Source File

SOURCE=.\ViewAllMsgDlg.h
# End Source File
# Begin Source File

SOURCE=.\ViewBuddySlipMsg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\allmsgtree.bmp
# End Source File
# Begin Source File

SOURCE=.\res\allmsgtreeStatus.bmp
# End Source File
# Begin Source File

SOURCE=.\res\AwaySmall.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap_n.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap_s.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Blocked.bmp
# End Source File
# Begin Source File

SOURCE=.\res\busyB.bmp
# End Source File
# Begin Source File

SOURCE=.\res\busySmall.bmp
# End Source File
# Begin Source File

SOURCE=.\res\DEFAULT.ico
# End Source File
# Begin Source File

SOURCE=.\res\EmoticonBMP.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Group.bmp
# End Source File
# Begin Source File

SOURCE=.\res\GrpOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon_chk.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon_CHK_OFF.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon_chk_ON.ico
# End Source File
# Begin Source File

SOURCE=.\res\IdmsMsn.ico
# End Source File
# Begin Source File

SOURCE=.\res\IdmsMsn.rc2
# End Source File
# Begin Source File

SOURCE=.\res\IT_COMP.bmp
# End Source File
# Begin Source File

SOURCE=.\res\leftbottom.bmp
# End Source File
# Begin Source File

SOURCE=.\res\link.cur
# End Source File
# Begin Source File

SOURCE=.\res\login_bg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\login_bg.jpg
# End Source File
# Begin Source File

SOURCE=.\res\MainBg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\MainBgBottom.bmp
# End Source File
# Begin Source File

SOURCE=.\res\MainBgTop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\MeAway.bmp
# End Source File
# Begin Source File

SOURCE=.\res\my_status_click.bmp
# End Source File
# Begin Source File

SOURCE=.\res\my_status_over.bmp
# End Source File
# Begin Source File

SOURCE=.\res\OffLine.bmp
# End Source File
# Begin Source File

SOURCE=.\res\OnLine.bmp
# End Source File
# Begin Source File

SOURCE=.\res\onlineB.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pin1.ico
# End Source File
# Begin Source File

SOURCE=.\res\pin2.ico
# End Source File
# Begin Source File

SOURCE=.\res\rightbottom.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn1_hide.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn1_over.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn2_hide.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn2_over.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn3_hide.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sec_btn3_over.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SENDFAIL.ICO
# End Source File
# Begin Source File

SOURCE=.\res\SENDOK.ICO
# End Source File
# Begin Source File

SOURCE=.\res\static_Password.bmp
# End Source File
# Begin Source File

SOURCE=.\res\static_UserID.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\topleft.bmp
# End Source File
# Begin Source File

SOURCE=.\res\topright.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
