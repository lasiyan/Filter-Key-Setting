
// FilterKeySetting.cpp : Defines the class behaviors for the application.
//

// clang-format off
#include "pch.h"
#include "framework.h"
#include "FilterKeySetting.h"
#include "FilterKeySettingDlg.h"
#include "FilterValue.h"
// clang-format on

#ifdef _DEBUG
  #define new DEBUG_NEW
#endif

// CFilterKeySettingApp

BEGIN_MESSAGE_MAP(CFilterKeySettingApp, CWinApp)
ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CFilterKeySettingApp construction

CFilterKeySettingApp::CFilterKeySettingApp()
{
  // support Restart Manager
  m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

  // TODO: add construction code here,
  // Place all significant initialization in InitInstance
}

// The one and only CFilterKeySettingApp object

CFilterKeySettingApp theApp;

// CFilterKeySettingApp initialization

BOOL CFilterKeySettingApp::InitInstance()
{
  // InitCommonControlsEx() is required on Windows XP if an application
  // manifest specifies use of ComCtl32.dll version 6 or later to enable
  // visual styles.  Otherwise, any window creation will fail.
  INITCOMMONCONTROLSEX InitCtrls;
  InitCtrls.dwSize = sizeof(InitCtrls);
  // Set this to include all the common control classes you want to use
  // in your application.
  InitCtrls.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&InitCtrls);

  CWinApp::InitInstance();

  AfxEnableControlContainer();

  // Create the shell manager, in case the dialog contains
  // any shell tree view or shell list view controls.
  CShellManager* pShellManager = new CShellManager;

  // Activate "Windows Native" visual manager for enabling themes in MFC
  // controls
  CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

  // Standard initialization
  // If you are not using these features and wish to reduce the size
  // of your final executable, you should remove from the following
  // the specific initialization routines you do not need
  // Change the registry key under which our settings are stored
  // TODO: You should modify this string to be something appropriate
  // such as the name of your company or organization
  if (FilterKey::Container::initialize() == false) {
    AfxMessageBox(_T("레지스트리 생성에 실패하였습니다.\r\n프로그램을 관리자 ")
                  _T("권한으로 실행하세요."));
    return FALSE;
  }

  CFilterKeySettingDlg dlg;
  m_pMainWnd        = &dlg;
  INT_PTR nResponse = dlg.DoModal();
  if (nResponse == IDOK) {
    // TODO: Place code here to handle when the dialog is
    //  dismissed with OK
  }
  else if (nResponse == IDCANCEL) {
    // TODO: Place code here to handle when the dialog is
    //  dismissed with Cancel
  }
  else if (nResponse == -1) {
    TRACE(traceAppMsg, 0,
          "Warning: dialog creation failed, so application is terminating "
          "unexpectedly.\n");
    TRACE(traceAppMsg, 0,
          "Warning: if you are using MFC controls on the dialog, you cannot "
          "#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
  }

  // Delete the shell manager created above.
  if (pShellManager != nullptr) {
    delete pShellManager;
  }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
  ControlBarCleanUp();
#endif

  // Since the dialog has been closed, return FALSE so that we exit the
  //  application, rather than start the application's message pump.
  return FALSE;
}

int CFilterKeySettingApp::ExitInstance()
{
  FilterKey::Container global(USER_REGISTRY_PATH);

  if (global.getInteger(KEY_RESTORE_SETTING)) {
    FILTERKEYS filter_keys  = { sizeof(FILTERKEYS) };
    filter_keys.iWaitMSec   = DEFAULT_ACCEPT_DELAY;
    filter_keys.iDelayMSec  = DEFAULT_REPEAT_DELAY;
    filter_keys.iRepeatMSec = DEFAULT_REPEAT_RATE;
    filter_keys.dwFlags     = WINDOW_FILTER_FLAG;

    if (global.getInteger(KEY_DISABLE_HOTKEY)) {
      filter_keys.dwFlags &=
          ~(FKF_HOTKEYACTIVE | FKF_CONFIRMHOTKEY | FKF_HOTKEYSOUND);
    }

    SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &filter_keys,
                         SPIF_UPDATEINIFILE);
    global.set(KEY_LAST_PRESET, static_cast<DWORD>(0));
  }

  return CWinApp::ExitInstance();
}
