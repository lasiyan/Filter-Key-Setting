
// FilterKeySettingDlg.cpp : implementation file
//

// clang-format off
#include "pch.h"
#include "framework.h"
#include "FilterKeySetting.h"
#include "FilterKeySettingDlg.h"
#include "afxdialogex.h"

#include "DialogRename.h"
// clang-format on

#ifdef _DEBUG
  #define new DEBUG_NEW
#endif

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

// CFilterKeySettingDlg dialog

CFilterKeySettingDlg::CFilterKeySettingDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_FILTERKEYSETTING_DIALOG, pParent)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

  global_config_ = new FilterKey::Container(USER_REGISTRY_PATH);
}

CFilterKeySettingDlg::~CFilterKeySettingDlg()
{
  SAFE_DELETE(global_config_);
  SAFE_DELETE(preset_);
}

void CFilterKeySettingDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFilterKeySettingDlg, CDialogEx)
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()

ON_BN_CLICKED(IDC_BTN_PRESET_OFF, OnBnClickedPresetOff)
ON_BN_CLICKED(IDC_BTN_PRESET1, OnBnClickedPreset1)
ON_BN_CLICKED(IDC_BTN_PRESET2, OnBnClickedPreset2)
ON_BN_CLICKED(IDC_BTN_APPLY, OnBnClickedApply)

ON_EN_SETFOCUS(IDC_EDIT_TESTING, &CFilterKeySettingDlg::OnEnSetFocusTesting)
ON_EN_KILLFOCUS(IDC_EDIT_TESTING, &CFilterKeySettingDlg::OnEnKillFocusTesting)

ON_BN_CLICKED(IDC_CHECK_FAST_APPLY,
              &CFilterKeySettingDlg::OnBnClickedCheckFastApply)
ON_BN_CLICKED(IDC_CHECK_RESTORE_SETTING,
              &CFilterKeySettingDlg::OnBnClickedCheckRestoreSetting)
ON_BN_CLICKED(IDC_CHECK_DISABLE_HOTKEY,
              &CFilterKeySettingDlg::OnBnClickedCheckDisableHotkey)
END_MESSAGE_MAP()

// CFilterKeySettingDlg message handlers

BOOL CFilterKeySettingDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);   // Set big icon
  SetIcon(m_hIcon, FALSE);  // Set small icon

  // Set Button UI
  {
    GetDlgItem(IDC_BTN_PRESET_OFF)->SetWindowText(_T("끄기"));
  }

  {
    FilterKey::Container preset(FilterKey::PRESET::ONE);
    auto                 title = preset.getString(KEY_PRESET_TITLE);
    GetDlgItem(IDC_BTN_PRESET1)->SetWindowText(title);
  }

  {
    FilterKey::Container preset(FilterKey::PRESET::TWO);
    auto                 title = preset.getString(KEY_PRESET_TITLE);
    GetDlgItem(IDC_BTN_PRESET2)->SetWindowText(title);
  }

  // Set Global configure
  auto last_preset = global_config_->getInteger(KEY_LAST_PRESET);
  if (last_preset >= static_cast<int>(FilterKey::PRESET::ONE)) {
    SAFE_DELETE(preset_)
    preset_ =
        new FilterKey::Container(static_cast<FilterKey::PRESET>(last_preset));
  }

  // Set Default Preset
  UpdateOption(FALSE);
  UpdateInterface();

  // Kill Focus Edit control
  OnEnKillFocusTesting();

  return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFilterKeySettingDlg::OnPaint()
{
  if (IsIconic()) {
    CPaintDC dc(this);  // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()),
                0);

    // Center icon in client rectangle
    int   cxIcon = GetSystemMetrics(SM_CXICON);
    int   cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else {
    CDialogEx::OnPaint();
  }
}

// The system calls this function to obtain the cursor to display while the user
// drags
//  the minimized window.
HCURSOR CFilterKeySettingDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void CFilterKeySettingDlg::OnBnClickedPresetOff()
{
  SAFE_DELETE(preset_);

  if (press_ctrl_) {
    // PopupRenameDialog();
  }
  else {
    UpdateInterface();
    ApplyFilterKey(fast_apply_ ? FALSE : TRUE);
  }
}

void CFilterKeySettingDlg::OnBnClickedPreset1()
{
  SAFE_DELETE(preset_);
  preset_ = new FilterKey::Container(FilterKey::PRESET::ONE);

  if (press_ctrl_) {
    PopupRenameDialog();
  }
  else {
    UpdateInterface();

    if (fast_apply_) {
      ApplyFilterKey(FALSE);
    }
  }
}

void CFilterKeySettingDlg::OnBnClickedPreset2()
{
  SAFE_DELETE(preset_);
  preset_ = new FilterKey::Container(FilterKey::PRESET::TWO);

  if (press_ctrl_) {
    PopupRenameDialog();
  }
  else {
    UpdateInterface();

    if (fast_apply_) {
      ApplyFilterKey(FALSE);
    }
  }
}

void CFilterKeySettingDlg::OnBnClickedApply()
{
  UpdateFilterKey();
  ApplyFilterKey();
}

void CFilterKeySettingDlg::OnBnClickedCheckFastApply()
{
  UpdateOption();  //
}

void CFilterKeySettingDlg::OnBnClickedCheckRestoreSetting()
{
  UpdateOption();  //
}

void CFilterKeySettingDlg::OnBnClickedCheckDisableHotkey()
{
  UpdateOption();  //
}

void CFilterKeySettingDlg::OnEnSetFocusTesting()
{
  GetDlgItem(IDC_EDIT_TESTING)->SetWindowText("");
}

void CFilterKeySettingDlg::OnEnKillFocusTesting()
{
  GetDlgItem(IDC_EDIT_TESTING)->SetWindowText(_T("설정 값 테스트 해보기"));
}

////////////////////////////////////////////////////////////////////
void CFilterKeySettingDlg::PopupRenameDialog()
{
  DialogRename dlg(preset_->getString(KEY_PRESET_TITLE), this);

  if (dlg.DoModal() == IDOK) {
    // Update registry & caption

    auto uid = IDC_BTN_PRESET1 + preset_->number() - 1;

    preset_->set(KEY_PRESET_TITLE, dlg.new_name_);
    GetDlgItem(uid)->SetWindowText(dlg.new_name_);
  }
  press_ctrl_ = false;
}

void CFilterKeySettingDlg::ApplyFilterKey(BOOL alert)
{
  int accept_delay = DEFAULT_ACCEPT_DELAY;
  int repeat_delay = DEFAULT_REPEAT_DELAY;
  int repeat_rate  = DEFAULT_REPEAT_RATE;
  int filter_flag  = WINDOW_FILTER_FLAG;

  if (preset_) {
    accept_delay = preset_->getInteger(KEY_ACCEPT_DELAY);
    repeat_delay = preset_->getInteger(KEY_REPEAT_DELAY);
    repeat_rate  = preset_->getInteger(KEY_REPEAT_RATE);
    filter_flag  = preset_->getInteger(KEY_FILTER_FLAG);
  }

  if (accept_delay > 20000) {
    AfxMessageBox("Accept Delay 값 최대치는 20000입니다.");
    GetDlgItem(IDC_EDIT_ACCEPT_DELAY)->SetFocus();
    return;
  }

  if (repeat_delay > 20000) {
    AfxMessageBox("Repeat Delay 값 최대치는 20000입니다.");
    GetDlgItem(IDC_EDIT_REPEAT_DELAY)->SetFocus();
    return;
  }

  if (repeat_rate > 20000) {
    AfxMessageBox("Repeat Rate 값 최대치는 20000입니다.");
    GetDlgItem(IDC_EDIT_REPEAT_RATE)->SetFocus();
    return;
  }

  FILTERKEYS filter_keys  = { sizeof(FILTERKEYS) };
  filter_keys.iWaitMSec   = accept_delay;
  filter_keys.iDelayMSec  = repeat_delay;
  filter_keys.iRepeatMSec = repeat_rate;
  filter_keys.dwFlags     = filter_flag;

  if (global_config_->getInteger(KEY_DISABLE_HOTKEY)) {
    filter_keys.dwFlags &=
        ~(FKF_HOTKEYACTIVE | FKF_CONFIRMHOTKEY | FKF_HOTKEYSOUND);
  }

  bool ok = SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS),
                                 &filter_keys, SPIF_UPDATEINIFILE);

  if (ok) {
    DWORD number = preset_ ? preset_->number() : 0;
    global_config_->set(KEY_LAST_PRESET, number);
  }

  if (ok && alert) {
    if (preset_) {
      AfxMessageBox("필터키가 활성화 되었습니다");
    }
    else {
      AfxMessageBox("필터키가 비활성화 되었습니다");
    }
  }

  if (!ok && alert) {
    AfxMessageBox("필터키를 설정할 수 없습니다\r\n관리자 권한으로 실행하세요.");
  }
}

void CFilterKeySettingDlg::UpdateOption(BOOL write /* = TRUE*/)
{
  ASSERT(global_config_);

  CButton* btn = nullptr;

  if (write) {
    // Fast Apply
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_FAST_APPLY));
      auto checked = btn ? btn->GetCheck() : false;

      global_config_->set(KEY_FAST_APPLY, static_cast<DWORD>(checked));
      fast_apply_ = global_config_->getInteger(KEY_FAST_APPLY);
    }

    // Restore setting
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_RESTORE_SETTING));
      auto checked = btn ? btn->GetCheck() : false;
      global_config_->set(KEY_RESTORE_SETTING, static_cast<DWORD>(checked));
    }

    // Disable Hotkey
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DISABLE_HOTKEY));
      auto checked = btn ? btn->GetCheck() : false;
      global_config_->set(KEY_DISABLE_HOTKEY, static_cast<DWORD>(checked));
    }
  }
  else {
    // Fast Apply
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_FAST_APPLY));
      fast_apply_ = global_config_->getInteger(KEY_FAST_APPLY);
      btn->SetCheck(fast_apply_ ? BST_CHECKED : BST_UNCHECKED);
    }

    // Restore setting
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_RESTORE_SETTING));
      auto restore = global_config_->getInteger(KEY_RESTORE_SETTING);
      btn->SetCheck(restore ? BST_CHECKED : BST_UNCHECKED);
    }

    // Disable Hotkey
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DISABLE_HOTKEY));
      auto restore = global_config_->getInteger(KEY_DISABLE_HOTKEY);
      btn->SetCheck(restore ? BST_CHECKED : BST_UNCHECKED);
    }
  }
}

void CFilterKeySettingDlg::UpdateInterface()
{
  if (preset_) {
    // Set window title
    CString window_title;
    window_title.Format(_T("필터키 : %s"),
                        preset_->getString(KEY_PRESET_TITLE));
    SetWindowText(window_title);

    CString value;

    value.Format(_T("%d"), preset_->getInteger(KEY_ACCEPT_DELAY));
    GetDlgItem(IDC_EDIT_ACCEPT_DELAY)->SetWindowText(value);

    value.Format(_T("%d"), preset_->getInteger(KEY_REPEAT_DELAY));
    GetDlgItem(IDC_EDIT_REPEAT_DELAY)->SetWindowText(value);

    value.Format(_T("%d"), preset_->getInteger(KEY_REPEAT_RATE));
    GetDlgItem(IDC_EDIT_REPEAT_RATE)->SetWindowText(value);
  }
  else {
    // Set window title
    SetWindowText(_T("필터키 : 끄기"));
    /*GetDlgItem()*/
  }

  GetDlgItem(IDC_EDIT_ACCEPT_DELAY)->EnableWindow(preset_ ? TRUE : FALSE);
  GetDlgItem(IDC_EDIT_REPEAT_DELAY)->EnableWindow(preset_ ? TRUE : FALSE);
  GetDlgItem(IDC_EDIT_REPEAT_RATE)->EnableWindow(preset_ ? TRUE : FALSE);
  GetDlgItem(IDC_BTN_APPLY)->EnableWindow(preset_ ? TRUE : FALSE);
}

void CFilterKeySettingDlg::UpdateFilterKey()
{
  if (preset_ == nullptr) {
    AfxMessageBox(_T("좌측 프리셋을 먼저 선택해주세요."));
    return;
  }

  // Update configure value to registry
  DWORD value = 0;

  value = static_cast<DWORD>(GetDlgItemInt(IDC_EDIT_ACCEPT_DELAY));
  preset_->set(KEY_ACCEPT_DELAY, value);

  value = static_cast<DWORD>(GetDlgItemInt(IDC_EDIT_REPEAT_DELAY));
  preset_->set(KEY_REPEAT_DELAY, value);

  value = static_cast<DWORD>(GetDlgItemInt(IDC_EDIT_REPEAT_RATE));
  preset_->set(KEY_REPEAT_RATE, value);
}

BOOL CFilterKeySettingDlg::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN) {
    // ESC, Enter 키 이벤트 제거
    if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) {
      return TRUE;
    }

    // 이름 변경 다이얼로그 이벤트 목적
    if (pMsg->wParam == VK_CONTROL) {
      press_ctrl_ = true;
      TRACE("ctrl pressing ...\n");
    }
  }
  else if (pMsg->message == WM_KEYUP) {
    if (pMsg->wParam == VK_CONTROL) {
      press_ctrl_ = false;
      TRACE("ctrl key up\n");
    }
  }

  return CDialogEx::PreTranslateMessage(pMsg);
}

#if 0
void CFilterKeySettingDlg::SetFlags(DWORD dwFlags)
{
  m_bOn            = !!(dwFlags & FKF_FILTERKEYSON);
  m_bAvailable     = !!(dwFlags & FKF_AVAILABLE);
  m_bIndicator     = !!(dwFlags & FKF_INDICATOR);
  m_bClick         = !!(dwFlags & FKF_CLICKON);
  m_bHotKeyActive  = !!(dwFlags & FKF_HOTKEYACTIVE);
  m_bConfirmHotKey = !!(dwFlags & FKF_CONFIRMHOTKEY);
  m_bHotKeySound   = !!(dwFlags & FKF_HOTKEYSOUND);
}

DWORD CFilterKeySettingDlg::GetFlagsVal()
{
  DWORD dwFlags = 0;
  if (m_bOn)
    dwFlags |= FKF_FILTERKEYSON;
  if (m_bAvailable)
    dwFlags |= FKF_AVAILABLE;
  if (m_bIndicator)
    dwFlags |= FKF_INDICATOR;
  if (m_bClick)
    dwFlags |= FKF_CLICKON;
  if (m_bHotKeyActive)
    dwFlags |= FKF_HOTKEYACTIVE;
  if (m_bConfirmHotKey)
    dwFlags |= FKF_CONFIRMHOTKEY;
  if (m_bHotKeySound)
    dwFlags |= FKF_HOTKEYSOUND;
  return dwFlags;
}
#endif
