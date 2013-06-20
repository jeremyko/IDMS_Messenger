; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CUpdateManagerDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "updatemanager.h"
LastPage=0

ClassCount=3
Class1=CUpdateManagerApp
Class2=CUpdateManagerDlg

ResourceCount=2
Resource1=IDD_UPDATE_MANAGER
Class3=CSelectServer
Resource2=IDD_IDMS_SET_SERVER

[CLS:CUpdateManagerApp]
Type=0
BaseClass=CWinApp
HeaderFile=UpdateManager.h
ImplementationFile=UpdateManager.cpp
LastObject=CUpdateManagerApp

[CLS:CUpdateManagerDlg]
Type=0
BaseClass=CDialog
HeaderFile=UpdateManagerDlg.h
ImplementationFile=UpdateManagerDlg.cpp
LastObject=CUpdateManagerDlg

[DLG:IDD_UPDATE_MANAGER]
Type=1
Class=CUpdateManagerDlg
ControlCount=2
Control1=ID_ST_COMMENT,static,1342308352
Control2=ID_ANIMATE,SysAnimate32,1342242821

[DLG:IDD_IDMS_SET_SERVER]
Type=1
Class=CSelectServer
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,button,1342177287
Control4=IDC_IDMS_UP_CUSTOM,button,1342177289
Control5=IDC_IDMS_UP_RATE,button,1342177289

[CLS:CSelectServer]
Type=0
HeaderFile=SelectServer.h
ImplementationFile=SelectServer.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_IDMS_UP_CUSTOM
VirtualFilter=dWC

