; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMemoItemDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "sec3_dll.h"
LastPage=0

ClassCount=8
Class1=CButtonST
Class2=CButtonIconSelect
Class3=CCalendarCtrl
Class4=CCalNumberEdit
Class5=CIconSelector
Class6=CSec3Dlg
Class7=CMemoItemDlg
Class8=CSec3_DLLApp

ResourceCount=3
Resource1=IDD_DIALOG_SEC3
Resource2=IDD_MEMOITEM
Resource3=IDM_POPUP

[CLS:CButtonST]
Type=0
BaseClass=CButton
HeaderFile=BtnST.h
ImplementationFile=BtnST.cpp

[CLS:CButtonIconSelect]
Type=0
BaseClass=CButtonST
HeaderFile=ButtonIconSelect.h
ImplementationFile=ButtonIconSelect.cpp
LastObject=CButtonIconSelect

[CLS:CCalendarCtrl]
Type=0
BaseClass=CWnd
HeaderFile=CalendarCtrl.h
ImplementationFile=CalendarCtrl.cpp

[CLS:CCalNumberEdit]
Type=0
BaseClass=CEdit
HeaderFile=CalNumberEdit.h
ImplementationFile=CalNumberEdit.cpp

[CLS:CIconSelector]
Type=0
BaseClass=CWnd
HeaderFile=IconSelector.h
ImplementationFile=IconSelector.cpp

[CLS:CMemoItemDlg]
Type=0
BaseClass=CDialog
HeaderFile=MemoItemDlg.h
ImplementationFile=MemoItemDlg.cpp
LastObject=CMemoItemDlg
Filter=D
VirtualFilter=dWC

[CLS:CSec3_DLLApp]
Type=0
BaseClass=CWinApp
HeaderFile=sec3_DLL.h
ImplementationFile=sec3_DLL.cpp

[CLS:CSec3Dlg]
Type=0
BaseClass=CDialog
HeaderFile=Sec3Dlg.h
ImplementationFile=Sec3Dlg.cpp

[DLG:IDD_MEMOITEM]
Type=1
Class=CMemoItemDlg
ControlCount=9
Control1=IDC_TITLE,edit,1350631552
Control2=IDC_CONTENT,edit,1352732676
Control3=ID_CLOSE,button,1342275585
Control4=IDCANCEL,button,1342275584
Control5=IDC_STATIC,static,1342308866
Control6=IDC_BTN_ICON,button,1073741824
Control7=IDC_DATETIMEPICKER_DATE,SysDateTimePick32,1342242848
Control8=IDC_DATETIMEPICKER_TIME,SysDateTimePick32,1342242857
Control9=IDC_CHECK_ALARM,button,1342242819

[DLG:IDD_DIALOG_SEC3]
Type=1
Class=CSec3Dlg
ControlCount=1
Control1=IDC_BTN_TEST,button,1073807360

[MNU:IDM_POPUP]
Type=1
Class=?
Command1=ID_FILE_NEW_ITEM
Command2=ID_FILE_EDIT_ITEM
Command3=ID_FILE_DELETE_ITEM
CommandCount=3

