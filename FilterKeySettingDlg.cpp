
// FilterKeySettingDlg.cpp : implementation file
//

// clang-format off
#include "pch.h"
#include "framework.h"
#include "FilterKeySetting.h"
#include "FilterKeySettingDlg.h"
#include "afxdialogex.h"

#include "DialogKeyBinding.h"
#include "DialogDebug.h"
#include "DialogRename.h"
#include "UserDevLog.hpp"
#include "UserFilterKey.hpp"
#include "UserKeyBinding.hpp"
#include "UserPlaySound.hpp"
#include "UserPresetOSD.hpp"
#include <algorithm>
// clang-format on

#ifdef _DEBUG
  #define new DEBUG_NEW
#endif

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

// CFilterKeySettingDlg dialog

CFilterKeySettingDlg::CFilterKeySettingDlg(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_FILTERKEYSETTING_DIALOG, pParent)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CFilterKeySettingDlg::~CFilterKeySettingDlg()
{
}

void CFilterKeySettingDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFilterKeySettingDlg, CDialogEx)
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_WM_SIZE()
ON_WM_TIMER()
ON_WM_DESTROY()
ON_WM_HOTKEY()
ON_MESSAGE(WM_TRAYICON_MSG, &CFilterKeySettingDlg::OnTrayIcon)
ON_MESSAGE(TIMER_MOUSE_MOVE_WATCH, &CFilterKeySettingDlg::OnMouseTrackerTriggered)
ON_MESSAGE(CFilterKeySettingDlg::WM_DEV_DEBUG_CLOSED, &CFilterKeySettingDlg::OnDebugDialogClosed)
ON_MESSAGE(CFilterKeySettingDlg::WM_DEV_DEBUG_OPTIONS_CHANGED, &CFilterKeySettingDlg::OnDebugOptionsChanged)
ON_COMMAND_RANGE(CFilterKeySettingDlg::dynamic_preset_button_base_,
                 CFilterKeySettingDlg::dynamic_preset_button_base_ + PRESET_MAX_COUNT - 1,
                 &CFilterKeySettingDlg::OnCommandPresetButton)
ON_EN_SETFOCUS(IDC_EDIT_TESTING, &CFilterKeySettingDlg::OnEnSetFocusTesting)
ON_EN_KILLFOCUS(IDC_EDIT_TESTING, &CFilterKeySettingDlg::OnEnKillFocusTesting)
ON_EN_SETFOCUS(IDC_EDIT_TOGGLE_KEYBIND, &CFilterKeySettingDlg::OnEnSetFocusToggleKeybind)
ON_BN_CLICKED(IDC_CHECK_EDIT_MODE, &CFilterKeySettingDlg::OnBnClickedCheckEditMode)
ON_BN_CLICKED(IDC_CHECK_RESTORE_SETTING, &CFilterKeySettingDlg::OnBnClickedCheckRestoreSetting)
ON_BN_CLICKED(IDC_CHECK_DISABLE_HOTKEY, &CFilterKeySettingDlg::OnBnClickedCheckDisableHotkey)
ON_BN_CLICKED(IDC_CHECK_MOVE_TO_TRAY, &CFilterKeySettingDlg::OnBnClickedCheckMoveToTray)
ON_BN_CLICKED(IDC_CHECK_ENABLE_KEYBIND, &CFilterKeySettingDlg::OnBnClickedCheckEnableKeybind)
ON_BN_CLICKED(IDC_CHECK_ENABLE_TOGGLE_KEYBIND, &CFilterKeySettingDlg::OnBnClickedCheckEnableToggleKeybind)
ON_BN_CLICKED(IDC_CHECK_SET_MOUSE_DBLCLICK_TRACKER, &CFilterKeySettingDlg::OnBnClickedCheckSetMouseDblclickTracker)
ON_BN_CLICKED(IDC_CHECK_DISABLE_WITH_ESC, &CFilterKeySettingDlg::OnBnClickedCheckDisableWithEsc)
END_MESSAGE_MAP()

// CFilterKeySettingDlg message handlers

BOOL CFilterKeySettingDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);   // Set big icon
  SetIcon(m_hIcon, FALSE);  // Set small icon

  FilterKey::BackupCurrentFilterKeysToOption();

  InitializePresetCount();
  BuildPresetButtons();
  LayoutDynamicControls();

  // Get last activated preset (with clamping), then sync UI only.
  auto last_set = static_cast<int>(GLOBAL_OPTION.getInteger(KEY_LAST_PRESET));
  if (PRESET_IS_VALID(last_set) == false)
    last_set = PRESET_OFF;
  ActivePreset(last_set, FALSE);

  // Update Interface
  UpdateOption(FALSE);
  ValidateActiveHotkeysAndAlert();

  // Kill Focus Edit control
  OnEnKillFocusTesting();

  // Set initial focus to the OFF preset button if available
  if (auto* off_button = GetDlgItem(GetPresetButtonControlId(PRESET_OFF));
      off_button && ::IsWindow(off_button->GetSafeHwnd()) &&
      off_button->IsWindowEnabled())
  {
    off_button->SetFocus();
    return FALSE;
  }

  return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFilterKeySettingDlg::OnPaint()
{
  if (IsIconic())
  {
    CPaintDC dc(this);  // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
  else
  {
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

void CFilterKeySettingDlg::OnDestroy()
{
  if (bg_esc_watch_active_)
  {
    KillTimer(TIMER_BG_ESC_WATCH);
    bg_esc_watch_active_ = false;
    bg_esc_prev_down_    = false;
  }

  if (debug_dialog_ && ::IsWindow(debug_dialog_->GetSafeHwnd()))
  {
    debug_dialog_->DestroyWindow();
    debug_dialog_ = nullptr;
  }

  mouse_tracker_.Release();

  if (GLOBAL_OPTION.getInteger(KEY_RESTORE_SETTING))
    ActivePreset(PRESET_OFF, FALSE);

  UnregisterPresetHotkeys();
  RemoveTrayIcon();

  CDialogEx::OnDestroy();
}

void CFilterKeySettingDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
  if (nHotKeyId == static_cast<UINT>(KeyBinding::TOGGLE_HOTKEY_ID))
  {
    const int current_preset = static_cast<int>(GLOBAL_OPTION.getInteger(KEY_LAST_PRESET));
    const int target_preset  = KeyBinding::ResolveToggleTargetPreset(current_preset, last_on_preset_, preset_count_);

    if (!PRESET_IS_VALID(target_preset))
      return;

    DevLog::Writef(_T("Global hotkey toggled: from=%d to=%d"), current_preset, target_preset);
    ResetEditMode();
    ActivePreset(target_preset, FALSE, TRUE);
    return;
  }

  if (KeyBinding::IsEnabled() == false)
  {
    CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
    return;
  }

  if (nHotKeyId < KeyBinding::HOTKEY_ID_BASE || nHotKeyId >= KeyBinding::HOTKEY_ID_BASE + static_cast<UINT>(preset_count_))
  {
    CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
    return;
  }

  const int target_preset = nHotKeyId - KeyBinding::HOTKEY_ID_BASE;
  if (PRESET_IS_VALID(target_preset) == false)
    return;

  DevLog::Writef(_T("Global hotkey activated: preset=%d"), target_preset);
  ResetEditMode();
  ActivePreset(target_preset, FALSE, TRUE);
}

void CFilterKeySettingDlg::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == TIMER_BG_ESC_WATCH)
  {
    const bool esc_down  = ((::GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0);
    const bool edge_down = (esc_down && !bg_esc_prev_down_);
    bg_esc_prev_down_    = esc_down;

    if (!edge_down)
      return;

    if (IsDialogForeground())
      return;

    if (const UINT modifiers = KeyBinding::CurrentModifiers(); modifiers != 0)
      return;

    if (KeyBinding::IsActiveHotkeyAssigned(VK_ESCAPE, 0, preset_count_))
      return;

    if (GLOBAL_OPTION.getInteger(KEY_LAST_PRESET) == static_cast<DWORD>(PRESET_OFF))
      return;

    DevLog::Write(_T("Background ESC detected. Turning preset off."));
    ResetEditMode();
    ActivePreset(PRESET_OFF, FALSE, TRUE);
    return;
  }

  CDialogEx::OnTimer(nIDEvent);
}

void CFilterKeySettingDlg::OnSize(UINT nType, int cx, int cy)
{
  CDialogEx::OnSize(nType, cx, cy);

  if (nType == SIZE_MINIMIZED)
  {
    if ((GLOBAL_OPTION.getInteger(KEY_MOVE_TO_TRAY) != 0) && AddTrayIcon())
      ShowWindow(SW_HIDE);
  }
}

LRESULT CFilterKeySettingDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
  if (wParam != TRAY_ICON_ID)
    return 0;

  if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP)
  {
    RestoreFromTray();
  }
  else if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU)
  {
    CMenu popup;
    if (popup.CreatePopupMenu())
    {
      popup.AppendMenu(MF_STRING, IDM_TRAY_OPEN, _T("열기"));
      popup.AppendMenu(MF_SEPARATOR);

      const UINT active_preset = GLOBAL_OPTION.getInteger(KEY_LAST_PRESET);
      for (int preset = PRESET_OFF; preset < preset_count_; ++preset)
      {
        const UINT menu_id = IDM_TRAY_PRESET_BASE + static_cast<UINT>(preset);
        const UINT flags   = MF_STRING | (active_preset == static_cast<UINT>(preset) ? MF_CHECKED : 0);
        popup.AppendMenu(flags, menu_id, PresetOption::TAG(preset));
      }

      popup.AppendMenu(MF_SEPARATOR);
      popup.AppendMenu(MF_STRING, IDM_TRAY_EXIT, _T("종료"));

      POINT cursor;
      GetCursorPos(&cursor);
      SetForegroundWindow();

      const UINT cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY, cursor.x, cursor.y, this);
      if (cmd == IDM_TRAY_OPEN)
      {
        RestoreFromTray();
      }
      else if (cmd == IDM_TRAY_EXIT)
      {
        RemoveTrayIcon();
        PostMessage(WM_CLOSE);
      }
      else if (cmd >= IDM_TRAY_PRESET_BASE && cmd < IDM_TRAY_PRESET_BASE + static_cast<UINT>(preset_count_))
      {
        const int selected_preset = static_cast<int>(cmd - IDM_TRAY_PRESET_BASE);
        last_selected_            = selected_preset;
        ActivePreset(selected_preset, FALSE, TRUE);
      }

      PostMessage(WM_NULL);
    }
  }

  return 0;
}

LRESULT CFilterKeySettingDlg::OnMouseTrackerTriggered(WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);

  const auto trigger = static_cast<MouseTrackerTrigger>(wParam);

  if (trigger == MouseTrackerTrigger::MoveDistance)
  {
    if (GLOBAL_OPTION.getInteger(KEY_ENABLE_MOUSE_MOVE_TRACKER) == 0)
      return 0;
  }
  else if (trigger == MouseTrackerTrigger::DoubleClick)
  {
    if (GLOBAL_OPTION.getInteger(KEY_ENABLE_MOUSE_DBLCLICK_TRACKER) == 0)
      return 0;
  }
  else
    return 0;

  if (IsDialogForeground())
    return 0;

  if (GLOBAL_OPTION.getInteger(KEY_LAST_PRESET) == static_cast<DWORD>(PRESET_OFF))
    return 0;

  DevLog::Writef(_T("Mouse tracker trigger received: type=%u"), static_cast<UINT>(wParam));
  ResetEditMode();
  ActivePreset(PRESET_OFF, FALSE, TRUE);
  return 0;
}

LRESULT CFilterKeySettingDlg::OnDebugDialogClosed(WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(wParam);
  UNREFERENCED_PARAMETER(lParam);

  debug_dialog_ = nullptr;
  return 0;
}

LRESULT CFilterKeySettingDlg::OnDebugOptionsChanged(WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(wParam);
  UNREFERENCED_PARAMETER(lParam);

  UpdateOption(FALSE);
  return 0;
}

void CFilterKeySettingDlg::OpenDeveloperDebugDialog()
{
  if (debug_dialog_ && ::IsWindow(debug_dialog_->GetSafeHwnd()))
  {
    debug_dialog_->ShowWindow(SW_SHOW);
    debug_dialog_->SetForegroundWindow();
    return;
  }

  auto dialog = new DialogDebug(this);
  if (!dialog->Create(IDD_DEBUG_CONSOLE, this))
  {
    delete dialog;
    return;
  }

  debug_dialog_ = dialog;
  debug_dialog_->ShowWindow(SW_SHOW);
  debug_dialog_->SetForegroundWindow();
}

bool CFilterKeySettingDlg::AddTrayIcon()
{
  if (tray_icon_added_)
    return true;

  ZeroMemory(&tray_icon_data_, sizeof(tray_icon_data_));
  tray_icon_data_.cbSize           = sizeof(tray_icon_data_);
  tray_icon_data_.hWnd             = GetSafeHwnd();
  tray_icon_data_.uID              = TRAY_ICON_ID;
  tray_icon_data_.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  tray_icon_data_.uCallbackMessage = WM_TRAYICON_MSG;
  tray_icon_data_.hIcon            = m_hIcon;
  _tcscpy_s(tray_icon_data_.szTip, _T("Filter Key Setting"));

  tray_icon_added_ = (Shell_NotifyIcon(NIM_ADD, &tray_icon_data_) == TRUE);
  return tray_icon_added_;
}

void CFilterKeySettingDlg::RemoveTrayIcon()
{
  if (!tray_icon_added_)
    return;

  Shell_NotifyIcon(NIM_DELETE, &tray_icon_data_);
  tray_icon_added_ = false;
}

void CFilterKeySettingDlg::RestoreFromTray()
{
  RemoveTrayIcon();
  ShowWindow(SW_RESTORE);
  SetForegroundWindow();
}

void CFilterKeySettingDlg::RegisterPresetHotkeys()
{
  KeyBinding::RegisterConfiguredHotkeys(GetSafeHwnd(), preset_count_, hotkey_registered_,
                                        &toggle_hotkey_registered_,
                                        KeyBinding::HOTKEY_ID_BASE,
                                        KeyBinding::TOGGLE_HOTKEY_ID);
}

void CFilterKeySettingDlg::UnregisterPresetHotkeys()
{
  KeyBinding::UnregisterConfiguredHotkeys(GetSafeHwnd(), preset_count_, hotkey_registered_,
                                          &toggle_hotkey_registered_,
                                          KeyBinding::HOTKEY_ID_BASE,
                                          KeyBinding::TOGGLE_HOTKEY_ID);
}

void CFilterKeySettingDlg::RefreshPresetButtonCaption(const int preset)
{
  if (PRESET_IS_VALID(preset) == false)
    return;

  PresetOption option(preset);
  CString      title   = option.getString(KEY_PRESET_TITLE);
  CString      caption = title;

  if (KeyBinding::IsEnabled() && alt_hotkey_view_)
  {
    caption = KeyBinding::HotkeyToString(option.getInteger(KEY_PRESET_HOTKEY, 0), true);
  }

  if (auto button = GetDlgItem(GetPresetButtonControlId(preset)); button)
    button->SetWindowText(caption);
}

void CFilterKeySettingDlg::RefreshAllPresetButtonCaptions()
{
  for (int preset = PRESET_OFF; preset < preset_count_; ++preset)
    RefreshPresetButtonCaption(preset);
}

BOOL CFilterKeySettingDlg::PreTranslateMessage(MSG* pMsg)
{
  const auto IsCtrlAltF12 = [](const MSG* msg) -> bool {
    if (!msg)
      return false;

    if ((msg->message != WM_KEYDOWN) && (msg->message != WM_SYSKEYDOWN))
      return false;

    if (msg->wParam != VK_F12)
      return false;

    const bool ctrl_pressed    = ((GetKeyState(VK_CONTROL) & 0x8000) != 0) || ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0);
    const bool alt_from_syskey = (msg->message == WM_SYSKEYDOWN) && ((msg->lParam & (1UL << 29)) != 0);
    const bool alt_pressed     = alt_from_syskey || ((GetKeyState(VK_MENU) & 0x8000) != 0) || ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0);

    return ctrl_pressed && alt_pressed;
  };

  if (IsCtrlAltF12(pMsg))
  {
    OpenDeveloperDebugDialog();
    return TRUE;
  }

  if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
  {
    if (pMsg->wParam == VK_MENU || pMsg->wParam == VK_LMENU || pMsg->wParam == VK_RMENU)
    {
      if (alt_hotkey_view_ == false)
      {
        alt_hotkey_view_ = true;
        RefreshAllPresetButtonCaptions();
      }
    }

    // ESC, Enter 키 이벤트 제거
    if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
    {
      return TRUE;
    }
  }
  else if (pMsg->message == WM_KEYUP || pMsg->message == WM_SYSKEYUP)
  {
    if (pMsg->wParam == VK_MENU || pMsg->wParam == VK_LMENU || pMsg->wParam == VK_RMENU)
    {
      if (alt_hotkey_view_)
      {
        alt_hotkey_view_ = false;
        RefreshAllPresetButtonCaptions();
      }
    }
  }

  return CDialogEx::PreTranslateMessage(pMsg);
}

void CFilterKeySettingDlg::OnBnClickedCheckEditMode()
{
  if (PRESET_IS_ON(last_selected_))
  {
    // UnChecked -> Checked된 상태인가?
    const bool edit_on = IsEditModeChecked();
    TRACE(_T("EditMode : %s : Target Preset(%d)\n"), edit_on ? "Checked" : "UnChecked", last_selected_);

    if (edit_on)
    {
      const int edit_target_preset = last_selected_;

      // 필터키 일시적으로 끄기
      preset_before_edit_ = GLOBAL_OPTION.getInteger(KEY_LAST_PRESET);
      ActivePreset(PRESET_OFF, FALSE);

      // 편집 대상은 유지한다. (적용 시 OFF로 저장/적용되는 문제 방지)
      last_selected_ = edit_target_preset;
      UpdateInterface(last_selected_);
    }
    else
    {
      const int target_preset_for_apply = PRESET_IS_ON(preset_before_edit_) ? preset_before_edit_ : last_selected_;
      if (!SaveCurrentEditingValues(target_preset_for_apply))
      {
        if (auto edit_mode_button = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_EDIT_MODE)); edit_mode_button)
          edit_mode_button->SetCheck(BST_CHECKED);
        return;
      }

      ActivePreset(target_preset_for_apply, TRUE);
      preset_before_edit_ = PRESET_OFF;
    }

    UpdateOption();
  }
}

void CFilterKeySettingDlg::OnBnClickedCheckRestoreSetting()
{
  UpdateOption();  //
}

void CFilterKeySettingDlg::OnBnClickedCheckDisableHotkey()
{
  UpdateOption();  //
}

void CFilterKeySettingDlg::OnBnClickedCheckMoveToTray()
{
  UpdateOption();
}

void CFilterKeySettingDlg::OnBnClickedCheckEnableKeybind()
{
  if (auto btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_ENABLE_KEYBIND)); btn)
  {
    const bool checked  = (btn->GetCheck() == BST_CHECKED);
    const bool previous = KeyBinding::IsEnabled();
    if (checked && !previous)
    {
      GLOBAL_OPTION.set(KEY_ENABLE_KEYBIND, true);
      if (!ValidateActiveHotkeysAndAlert())
      {
        GLOBAL_OPTION.set(KEY_ENABLE_KEYBIND, false);
        btn->SetCheck(BST_UNCHECKED);
      }
    }
  }

  UpdateOption();
}

void CFilterKeySettingDlg::OnBnClickedCheckEnableToggleKeybind()
{
  if (auto btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_ENABLE_TOGGLE_KEYBIND)); btn)
  {
    const bool checked  = (btn->GetCheck() == BST_CHECKED);
    const bool previous = KeyBinding::IsToggleEnabled();
    if (checked && !previous)
    {
      GLOBAL_OPTION.set(KEY_ENABLE_TOGGLE_KEYBIND, true);
      if (!ValidateActiveHotkeysAndAlert())
      {
        GLOBAL_OPTION.set(KEY_ENABLE_TOGGLE_KEYBIND, false);
        btn->SetCheck(BST_UNCHECKED);
      }
    }
  }

  UpdateOption();
}

void CFilterKeySettingDlg::OnBnClickedCheckSetMouseDblclickTracker()
{
  UpdateOption();
}

void CFilterKeySettingDlg::OnBnClickedCheckDisableWithEsc()
{
  if (auto btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DISABLE_WITH_ESC)); btn)
  {
    const bool checked  = (btn->GetCheck() == BST_CHECKED);
    const bool previous = (GLOBAL_OPTION.getInteger(KEY_DISABLE_WITH_ESC, 0) != 0);

    if (checked && !previous)
    {
      if (KeyBinding::IsActiveHotkeyAssigned(VK_ESCAPE, 0, preset_count_))
      {
        AfxMessageBox(_T("안내: 현재 활성 단축키에 ESC가 포함되어 있습니다.\r\n"
                         "이 경우 '백그라운드에서 ESC로 프리셋 끄기' 동작은 실행되지 않습니다."));
      }
    }
  }

  UpdateOption();
}

void CFilterKeySettingDlg::OnEnSetFocusTesting()
{
  GetDlgItem(IDC_EDIT_TESTING)->SetWindowText(_T(""));
}

void CFilterKeySettingDlg::OnEnKillFocusTesting()
{
  GetDlgItem(IDC_EDIT_TESTING)->SetWindowText(_T("설정 값 테스트 해보기"));
}

void CFilterKeySettingDlg::OnEnSetFocusToggleKeybind()
{
  if (opening_toggle_hotkey_dialog_)
    return;

  if (!KeyBinding::IsToggleEnabled())
    return;

  PopupToggleKeyBindingDialog();

  if (auto fallback = GetDlgItem(IDC_EDIT_TESTING); fallback)
    fallback->SetFocus();
}

////////////////////////////////////////////////////////////////////
void CFilterKeySettingDlg::BnClickPreset(const int target_preset)
{
  if (PRESET_IS_VALID(target_preset) == false)
    return;

  ResetEditMode();
  last_selected_ = target_preset;

  const bool press_ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
  const bool press_alt  = (GetKeyState(VK_MENU) & 0x8000) != 0;
  const bool keybind_on = KeyBinding::IsEnabled();

  if (press_alt && keybind_on)
  {
    PopupKeyBindingDialog(target_preset);
  }
  else if (press_ctrl)
  {
    if (PRESET_IS_ON(target_preset))
      PopupRenameDialog(target_preset);
  }
  else
  {
    ActivePreset(target_preset, FALSE, TRUE);
  }
}

void CFilterKeySettingDlg::PopupRenameDialog(const int target_preset)
{
  DialogRename dlg(PresetOption::TAG(target_preset), this);

  if (dlg.DoModal() == IDOK)
  {
    // Update registry & caption

    const UINT uid = GetPresetButtonControlId(target_preset);

    PresetOption option(target_preset);
    option.set(KEY_PRESET_TITLE, dlg.new_name_);
    if (auto preset_button = GetDlgItem(uid); preset_button)
      preset_button->SetWindowText(dlg.new_name_);
  }
  RefreshPresetButtonCaption(target_preset);
}

void CFilterKeySettingDlg::PopupKeyBindingDialog(const int target_preset)
{
  if (KeyBinding::IsEnabled() == false)
    return;

  PresetOption option(target_preset);
  DWORD        current_hotkey = option.getInteger(KEY_PRESET_HOTKEY, 0);

  // 키 바인딩 입력 중에는 전역 프리셋 핫키를 잠시 비활성화한다.
  UnregisterPresetHotkeys();

  const auto FinishBindingFlow = [&]() {
    alt_hotkey_view_ = false;
    RegisterPresetHotkeys();
    RefreshAllPresetButtonCaptions();
  };

  while (true)
  {
    DialogKeyBinding dlg(current_hotkey, this);
    if (dlg.DoModal() != IDOK)
    {
      FinishBindingFlow();
      return;
    }

    const DWORD hotkey_value = dlg.hotkey_value_;

    if (hotkey_value != 0)
    {
      constexpr DWORD reserved_debug_hotkey =
          (static_cast<DWORD>(MOD_CONTROL | MOD_ALT) << 16) | VK_F12;
      if (hotkey_value == reserved_debug_hotkey)
      {
        AfxMessageBox(_T("Ctrl+Alt+F12는 사용할 수 없는 단축키입니다.\r\n다른 단축키를 입력하세요."));
        current_hotkey = hotkey_value;
        continue;
      }

      CString duplicate_desc;
      if (KeyBinding::FindDuplicateActiveHotkeyCandidate(preset_count_, hotkey_value,
                                                         target_preset, false,
                                                         &duplicate_desc))
      {
        CString message;
        message.Format(_T("이미 '%s'에 등록된 단축키입니다.\r\n다른 단축키를 입력하세요."),
                       (LPCTSTR)duplicate_desc);
        AfxMessageBox(message);

        // 다시 입력할 수 있도록 방금 입력했던 값으로 창을 다시 연다.
        current_hotkey = hotkey_value;
        continue;
      }
    }

    option.set(KEY_PRESET_HOTKEY, hotkey_value);
    FinishBindingFlow();
    return;
  }
}

void CFilterKeySettingDlg::PopupToggleKeyBindingDialog()
{
  if (!KeyBinding::IsToggleEnabled())
    return;

  const DWORD current_hotkey = KeyBinding::GetToggleHotkey();

  UnregisterPresetHotkeys();
  opening_toggle_hotkey_dialog_ = true;

  const auto FinishBindingFlow = [&]() {
    opening_toggle_hotkey_dialog_ = false;
    RegisterPresetHotkeys();
    RefreshToggleHotkeyEditText();
    RefreshAllPresetButtonCaptions();
  };

  DWORD next_hotkey = current_hotkey;
  while (true)
  {
    DialogKeyBinding dlg(next_hotkey, this);
    if (dlg.DoModal() != IDOK)
    {
      FinishBindingFlow();
      return;
    }

    const DWORD hotkey_value = dlg.hotkey_value_;
    if (hotkey_value != 0)
    {
      constexpr DWORD reserved_debug_hotkey =
          (static_cast<DWORD>(MOD_CONTROL | MOD_ALT) << 16) | VK_F12;
      if (hotkey_value == reserved_debug_hotkey)
      {
        AfxMessageBox(_T("Ctrl+Alt+F12는 사용할 수 없는 단축키입니다.\r\n다른 단축키를 입력하세요."));
        next_hotkey = hotkey_value;
        continue;
      }

      CString duplicate_desc;
      if (KeyBinding::FindDuplicateActiveHotkeyCandidate(preset_count_, hotkey_value,
                                                         -1, true,
                                                         &duplicate_desc))
      {
        CString message;
        message.Format(_T("이미 '%s'에 등록된 단축키입니다.\r\n다른 단축키를 입력하세요."),
                       (LPCTSTR)duplicate_desc);
        AfxMessageBox(message);
        next_hotkey = hotkey_value;
        continue;
      }
    }

    GLOBAL_OPTION.set(KEY_TOGGLE_HOTKEY, hotkey_value);
    FinishBindingFlow();
    return;
  }
}

void CFilterKeySettingDlg::ActivePreset(const int preset, BOOL alert, BOOL beep)
{
  const int target_preset = PRESET_IS_VALID(preset)
                                ? preset
                                : PRESET_OFF;

  if (target_preset == static_cast<int>(GLOBAL_OPTION.getInteger(KEY_LAST_PRESET)))
  {
    last_selected_ = target_preset;
    UpdateInterface(target_preset);
    return;
  }

  bool ok = FilterKey::ActivePreset(target_preset);
  DevLog::Writef(_T("ActivePreset called: preset=%d, ok=%d"), target_preset, ok ? 1 : 0);
  if (ok && PRESET_IS_ON(target_preset) && target_preset < preset_count_)
    last_on_preset_ = target_preset;

  if (ok)
    UserPresetOSD::ShowPresetIndex(target_preset);

  if (ok && beep)
    UserPlaySound::PlayPresetAppliedSound(target_preset);

  if (alert)
  {
    if (ok)
      AfxMessageBox(_T("프리셋이 저장되었습니다."));
    else
      AfxMessageBox(_T("프리셋을 수정할 수 없습니다\r\n관리자 권한으로 실행하세요."));
  }

  // UI 업데이트
  last_selected_ = target_preset;
  UpdateInterface(target_preset);
}

void CFilterKeySettingDlg::ResetEditMode()
{
  auto btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_EDIT_MODE));
  btn->SetCheck(BST_UNCHECKED);
  preset_before_edit_ = PRESET_OFF;
}

void CFilterKeySettingDlg::UpdateOption(BOOL write /* = TRUE*/)
{
  CButton* btn = nullptr;

  if (write)
  {
    // Restore setting
    {
      btn          = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_RESTORE_SETTING));
      auto checked = btn ? btn->GetCheck() : false;
      GLOBAL_OPTION.set(KEY_RESTORE_SETTING, static_cast<DWORD>(checked));
    }

    // Disable Hotkey
    {
      btn          = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DISABLE_HOTKEY));
      auto checked = btn ? btn->GetCheck() : false;
      GLOBAL_OPTION.set(KEY_DISABLE_HOTKEY, static_cast<DWORD>(checked));

      // Remove Hotkey flag if disable hotkey is checked
      PresetOption option(PRESET_OFF);
      DWORD        value = checked ? WINDOW_FILTER_FLAG & ~(FKF_HOTKEYACTIVE | FKF_CONFIRMHOTKEY | FKF_HOTKEYSOUND)
                                   : WINDOW_FILTER_FLAG;
      option.set(KEY_FILTER_FLAG, value);
    }

    // Move To Tray
    {
      btn          = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_MOVE_TO_TRAY));
      auto checked = btn ? btn->GetCheck() : false;
      GLOBAL_OPTION.set(KEY_MOVE_TO_TRAY, static_cast<DWORD>(checked));
    }

    // Enable Key Binding
    {
      btn          = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_ENABLE_KEYBIND));
      auto checked = btn ? btn->GetCheck() : false;
      GLOBAL_OPTION.set(KEY_ENABLE_KEYBIND, static_cast<DWORD>(checked));

      if (!checked)
        alt_hotkey_view_ = false;
    }

    // Enable Toggle Key Binding
    {
      btn          = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_ENABLE_TOGGLE_KEYBIND));
      auto checked = btn ? btn->GetCheck() : false;
      GLOBAL_OPTION.set(KEY_ENABLE_TOGGLE_KEYBIND, static_cast<DWORD>(checked));

      if (auto toggle_edit = GetDlgItem(IDC_EDIT_TOGGLE_KEYBIND); toggle_edit)
        toggle_edit->EnableWindow(checked ? TRUE : FALSE);
    }

    RegisterPresetHotkeys();
    RefreshAllPresetButtonCaptions();
    RefreshToggleHotkeyEditText();

    // Enable Mouse Double-Click Tracker
    {
      btn          = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_SET_MOUSE_DBLCLICK_TRACKER));
      auto checked = btn ? btn->GetCheck() : false;
      GLOBAL_OPTION.set(KEY_ENABLE_MOUSE_DBLCLICK_TRACKER, static_cast<DWORD>(checked));
    }

    // Disable with ESC at background
    {
      btn          = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DISABLE_WITH_ESC));
      auto checked = btn ? btn->GetCheck() : false;
      GLOBAL_OPTION.set(KEY_DISABLE_WITH_ESC, static_cast<DWORD>(checked));
    }

    const bool move_enabled = (GLOBAL_OPTION.getInteger(KEY_ENABLE_MOUSE_MOVE_TRACKER) != 0);
    const bool dbl_enabled  = (GLOBAL_OPTION.getInteger(KEY_ENABLE_MOUSE_DBLCLICK_TRACKER) != 0);
    if (move_enabled || dbl_enabled)
    {
      if (mouse_tracker_.Initialize(GetSafeHwnd()))
      {
        mouse_tracker_.SetTriggerOptions(move_enabled, dbl_enabled);
        mouse_tracker_.Active();
      }
    }
    else
    {
      // 체크 해제 시 트래커 기능 자체를 완전히 중지한다.
      mouse_tracker_.Release();
    }

    UpdateEscDisableHotkeyRegistration();
  }
  else
  {
    // Restore setting
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_RESTORE_SETTING));
      btn->SetCheck(GLOBAL_OPTION.getInteger(KEY_RESTORE_SETTING) ? BST_CHECKED : BST_UNCHECKED);
    }

    // Disable Hotkey
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DISABLE_HOTKEY));
      btn->SetCheck(GLOBAL_OPTION.getInteger(KEY_DISABLE_HOTKEY) ? BST_CHECKED : BST_UNCHECKED);
    }

    // Move To Tray
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_MOVE_TO_TRAY));
      btn->SetCheck(GLOBAL_OPTION.getInteger(KEY_MOVE_TO_TRAY) ? BST_CHECKED : BST_UNCHECKED);
    }

    // Enable Key Binding
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_ENABLE_KEYBIND));
      btn->SetCheck(GLOBAL_OPTION.getInteger(KEY_ENABLE_KEYBIND) ? BST_CHECKED : BST_UNCHECKED);
    }

    // Enable Toggle Key Binding
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_ENABLE_TOGGLE_KEYBIND));
      btn->SetCheck(GLOBAL_OPTION.getInteger(KEY_ENABLE_TOGGLE_KEYBIND) ? BST_CHECKED : BST_UNCHECKED);

      if (auto toggle_edit = GetDlgItem(IDC_EDIT_TOGGLE_KEYBIND); toggle_edit)
        toggle_edit->EnableWindow(KeyBinding::IsToggleEnabled() ? TRUE : FALSE);
    }

    // Enable Mouse Double-Click Tracker
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_SET_MOUSE_DBLCLICK_TRACKER));
      btn->SetCheck(GLOBAL_OPTION.getInteger(KEY_ENABLE_MOUSE_DBLCLICK_TRACKER) ? BST_CHECKED : BST_UNCHECKED);
    }

    // Disable with ESC at background
    {
      btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DISABLE_WITH_ESC));
      btn->SetCheck(GLOBAL_OPTION.getInteger(KEY_DISABLE_WITH_ESC) ? BST_CHECKED : BST_UNCHECKED);
    }

    if (!KeyBinding::IsEnabled())
      alt_hotkey_view_ = false;

    RegisterPresetHotkeys();

    const bool move_enabled = (GLOBAL_OPTION.getInteger(KEY_ENABLE_MOUSE_MOVE_TRACKER) != 0);
    const bool dbl_enabled  = (GLOBAL_OPTION.getInteger(KEY_ENABLE_MOUSE_DBLCLICK_TRACKER) != 0);
    if (move_enabled || dbl_enabled)
    {
      if (mouse_tracker_.Initialize(GetSafeHwnd()))
      {
        mouse_tracker_.SetTriggerOptions(move_enabled, dbl_enabled);
        mouse_tracker_.Active();
      }
    }
    else
    {
      // 시작 시 옵션이 꺼져 있으면 인프라를 생성하지 않는다.
      mouse_tracker_.Release();
    }

    UpdateEscDisableHotkeyRegistration();

    RefreshAllPresetButtonCaptions();
    RefreshToggleHotkeyEditText();
  }
}

void CFilterKeySettingDlg::UpdateEscDisableHotkeyRegistration()
{
  const bool enabled = (GLOBAL_OPTION.getInteger(KEY_DISABLE_WITH_ESC) != 0);
  if (enabled)
  {
    if (!bg_esc_watch_active_)
    {
      if (SetTimer(TIMER_BG_ESC_WATCH, 25, nullptr) != 0)
      {
        bg_esc_watch_active_ = true;
        bg_esc_prev_down_    = false;
      }
      else
      {
        TRACE(_T("SetTimer failed. esc disable watcher, error=%lu\n"), GetLastError());
        GLOBAL_OPTION.set(KEY_DISABLE_WITH_ESC, static_cast<DWORD>(false));
        if (auto esc_button = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DISABLE_WITH_ESC)); esc_button)
          esc_button->SetCheck(BST_UNCHECKED);
      }
    }
  }
  else
  {
    if (bg_esc_watch_active_)
    {
      KillTimer(TIMER_BG_ESC_WATCH);
      bg_esc_watch_active_ = false;
      bg_esc_prev_down_    = false;
    }
  }
}

bool CFilterKeySettingDlg::ValidateActiveHotkeysAndAlert() const
{
  CString conflict_message;
  if (KeyBinding::ValidateActiveHotkeys(preset_count_, &conflict_message))
    return true;

  if (!conflict_message.IsEmpty())
    AfxMessageBox(conflict_message);
  return false;
}

void CFilterKeySettingDlg::RefreshToggleHotkeyEditText()
{
  if (auto toggle_edit = GetDlgItem(IDC_EDIT_TOGGLE_KEYBIND); toggle_edit)
  {
    const DWORD hotkey_value = KeyBinding::GetToggleHotkey();
    toggle_edit->SetWindowText(KeyBinding::HotkeyToString(hotkey_value, true));
  }
}

void CFilterKeySettingDlg::UpdateInterface(const int preset)
{
  // 프로그램 타이틀바
  {
    PresetOption option(GLOBAL_OPTION.getInteger(KEY_LAST_PRESET));
    CString      window_title;
    CString      preset_title = option.getString(KEY_PRESET_TITLE);
    window_title.Format(_T("활성화된 프리셋 : %s"), (LPCTSTR)preset_title);
    SetWindowText(window_title);
  }

  // 프리셋 설정 값 로드
  if (PRESET_IS_ON(preset))
  {
    PresetOption option(preset);
    CString      value;

    value.Format(_T("%d"), option.getInteger(KEY_ACCEPT_DELAY));
    GetDlgItem(IDC_EDIT_ACCEPT_DELAY)->SetWindowText(value);

    value.Format(_T("%d"), option.getInteger(KEY_REPEAT_DELAY));
    GetDlgItem(IDC_EDIT_REPEAT_DELAY)->SetWindowText(value);

    value.Format(_T("%d"), option.getInteger(KEY_REPEAT_RATE));
    GetDlgItem(IDC_EDIT_REPEAT_RATE)->SetWindowText(value);
  }

  // 수정 모드 활성화 여부
  GetDlgItem(IDC_CHECK_EDIT_MODE)->EnableWindow(PRESET_IS_ON(preset) ? TRUE : FALSE);
  auto edit_btn = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_EDIT_MODE));
  auto edit_on  = edit_btn ? edit_btn->GetCheck() : false;

  // 프리셋 설정값 UI 활성화 여부
  bool enable = PRESET_IS_ON(preset) && edit_on;
  GetDlgItem(IDC_EDIT_ACCEPT_DELAY)->EnableWindow(enable ? TRUE : FALSE);
  GetDlgItem(IDC_EDIT_REPEAT_DELAY)->EnableWindow(enable ? TRUE : FALSE);
  GetDlgItem(IDC_EDIT_REPEAT_RATE)->EnableWindow(enable ? TRUE : FALSE);
}

void CFilterKeySettingDlg::InitializePresetCount()
{
  int count = static_cast<int>(GLOBAL_OPTION.getInteger(KEY_PRESET_COUNT));
  if (count < PRESET_MIN_COUNT)
    count = PRESET_MIN_COUNT;
  if (count > PRESET_MAX_COUNT)
    count = PRESET_MAX_COUNT;

  preset_count_ = count;
  GLOBAL_OPTION.set(KEY_PRESET_COUNT, static_cast<DWORD>(preset_count_));
}

void CFilterKeySettingDlg::BuildPresetButtons()
{
  CWnd* template_off_button = GetDlgItem(IDC_BTN_PRESET_OFF);
  CWnd* template_p1_button  = GetDlgItem(IDC_BTN_PRESET1);

  if (auto ctrl = GetDlgItem(IDC_BTN_PRESET_OFF); ctrl)
    ctrl->ShowWindow(SW_HIDE);
  if (auto ctrl = GetDlgItem(IDC_BTN_PRESET1); ctrl)
    ctrl->ShowWindow(SW_HIDE);
  if (auto ctrl = GetDlgItem(IDC_BTN_PRESET2); ctrl)
    ctrl->ShowWindow(SW_HIDE);

  CRect base_rect;
  CRect next_rect;
  template_off_button->GetWindowRect(&base_rect);
  ScreenToClient(&base_rect);

  int stride = 32;
  if (template_p1_button)
  {
    template_p1_button->GetWindowRect(&next_rect);
    ScreenToClient(&next_rect);
    stride = next_rect.top - base_rect.top;
    if (stride <= 0)
      stride = base_rect.Height() + 6;
  }

  {
    const int button_height = base_rect.Height();
    int       button_gap    = stride - button_height;

    int min_required_gap = 2;
    if (preset_count_ > 1)
    {
      std::vector<UINT> settings_ids = {
        IDC_CHECK_EDIT_MODE,
        IDC_EDIT_ACCEPT_DELAY,
        IDC_EDIT_REPEAT_DELAY,
        IDC_EDIT_REPEAT_RATE,
        IDC_STATIC_LABEL_ACCEPT,
        IDC_STATIC_LABEL_ACCEPT_MS,
        IDC_STATIC_LABEL_REPEAT_DELAY,
        IDC_STATIC_LABEL_REPEAT_DELAY_MS,
        IDC_STATIC_LABEL_REPEAT_RATE,
        IDC_STATIC_LABEL_REPEAT_RATE_MS,
      };

      bool  cluster_initialized = false;
      CRect settings_cluster;
      for (auto id : settings_ids)
      {
        if (auto ctrl = GetDlgItem(id); ctrl)
        {
          CRect rect;
          ctrl->GetWindowRect(&rect);
          ScreenToClient(&rect);

          if (!cluster_initialized)
          {
            settings_cluster    = rect;
            cluster_initialized = true;
          }
          else
          {
            settings_cluster.UnionRect(settings_cluster, rect);
          }
        }
      }

      if (cluster_initialized)
      {
        const int required_height = settings_cluster.Height();
        const int numerator       = required_height - (preset_count_ * button_height);
        if (numerator > 0)
        {
          min_required_gap = max(min_required_gap, (numerator + (preset_count_ - 2)) / (preset_count_ - 1));
        }
      }
    }

    button_gap = max(min_required_gap, button_gap - 10);
    stride     = button_height + button_gap;
  }

  CFont* template_font = template_off_button ? template_off_button->GetFont() : GetFont();

  preset_buttons_.clear();
  preset_buttons_.reserve(preset_count_);

  for (int preset = PRESET_OFF; preset < preset_count_; ++preset)
  {
    auto  button = std::make_unique<CButton>();
    CRect rect(base_rect.left,
               base_rect.top + stride * preset,
               base_rect.right,
               base_rect.top + stride * preset + base_rect.Height());

    button->Create(_T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                   rect, this, GetPresetButtonControlId(preset));

    if (template_font)
      button->SetFont(template_font);

    preset_buttons_.push_back(std::move(button));
  }

  RefreshAllPresetButtonCaptions();
}

void CFilterKeySettingDlg::LayoutDynamicControls()
{
  if (preset_buttons_.empty())
    return;

  const int group_visual_offset_y = 7;

  std::vector<UINT> centered_settings_ids = {
    IDC_CHECK_EDIT_MODE,
    IDC_EDIT_ACCEPT_DELAY,
    IDC_EDIT_REPEAT_DELAY,
    IDC_EDIT_REPEAT_RATE,
    IDC_STATIC_LABEL_ACCEPT,
    IDC_STATIC_LABEL_ACCEPT_MS,
    IDC_STATIC_LABEL_REPEAT_DELAY,
    IDC_STATIC_LABEL_REPEAT_DELAY_MS,
    IDC_STATIC_LABEL_REPEAT_RATE,
    IDC_STATIC_LABEL_REPEAT_RATE_MS,
  };

  CRect first_button;
  CRect last_button;
  preset_buttons_.front()->GetWindowRect(&first_button);
  preset_buttons_.back()->GetWindowRect(&last_button);
  ScreenToClient(&first_button);
  ScreenToClient(&last_button);

  const int button_top    = first_button.top;
  const int button_bottom = last_button.bottom;

  bool  cluster_initialized = false;
  CRect settings_cluster;
  for (auto id : centered_settings_ids)
  {
    if (auto ctrl = GetDlgItem(id); ctrl)
    {
      CRect rect;
      ctrl->GetWindowRect(&rect);
      ScreenToClient(&rect);

      if (!cluster_initialized)
      {
        settings_cluster    = rect;
        cluster_initialized = true;
      }
      else
      {
        settings_cluster.UnionRect(settings_cluster, rect);
      }
    }
  }

  if (cluster_initialized)
  {
    const int target_center_y  = (button_top + button_bottom) / 2 - group_visual_offset_y;
    const int current_center_y = (settings_cluster.top + settings_cluster.bottom) / 2;
    const int delta_y          = target_center_y - current_center_y;

    for (auto id : centered_settings_ids)
    {
      if (auto ctrl = GetDlgItem(id); ctrl)
      {
        CRect rect;
        ctrl->GetWindowRect(&rect);
        ScreenToClient(&rect);
        ctrl->SetWindowPos(nullptr, rect.left, rect.top + delta_y,
                           0, 0, SWP_NOZORDER | SWP_NOSIZE);
      }
    }
  }

  if (auto settings_group = GetDlgItem(IDC_GROUP_SETTINGS); settings_group)
  {
    CRect group_rect;
    settings_group->GetWindowRect(&group_rect);
    ScreenToClient(&group_rect);

    const int group_top_for_layout = max(0, button_top);
    const int group_top_for_render = max(0, group_top_for_layout - group_visual_offset_y);
    const int group_height         = max(1, button_bottom - group_top_for_render);

    settings_group->SetWindowPos(nullptr,
                                 group_rect.left,
                                 group_top_for_render,
                                 group_rect.Width(),
                                 group_height,
                                 SWP_NOZORDER);

    if (auto edit_mode = GetDlgItem(IDC_CHECK_EDIT_MODE); edit_mode)
    {
      CRect edit_rect;
      edit_mode->GetWindowRect(&edit_rect);
      ScreenToClient(&edit_rect);

      const int group_border_line_y = group_top_for_layout + 2;
      const int edit_top            = group_border_line_y - (edit_rect.Height() / 2);
      edit_mode->SetWindowPos(nullptr,
                              edit_rect.left,
                              edit_top,
                              0, 0,
                              SWP_NOZORDER | SWP_NOSIZE);
    }
  }

  std::vector<UINT> lower_ids = {
    IDC_EDIT_TESTING,
    IDC_CHECK_RESTORE_SETTING,
    IDC_CHECK_MOVE_TO_TRAY,
    IDC_CHECK_DISABLE_HOTKEY,
    IDC_CHECK_ENABLE_KEYBIND,
    IDC_CHECK_SET_MOUSE_DBLCLICK_TRACKER,
    IDC_CHECK_DISABLE_WITH_ESC,
    IDC_CHECK_ENABLE_TOGGLE_KEYBIND,
    IDC_EDIT_TOGGLE_KEYBIND,
  };

  bool  lower_initialized = false;
  CRect lower_cluster;
  for (auto id : lower_ids)
  {
    if (auto ctrl = GetDlgItem(id); ctrl)
    {
      CRect rect;
      ctrl->GetWindowRect(&rect);
      ScreenToClient(&rect);

      if (!lower_initialized)
      {
        lower_cluster     = rect;
        lower_initialized = true;
      }
      else
      {
        lower_cluster.UnionRect(lower_cluster, rect);
      }
    }
  }

  if (lower_initialized)
  {
    int next_top = button_bottom + 10;

    CRect group_rect;
    if (auto settings_group = GetDlgItem(IDC_GROUP_SETTINGS); settings_group)
    {
      settings_group->GetWindowRect(&group_rect);
      ScreenToClient(&group_rect);
      next_top = max(next_top, group_rect.bottom + 10);
    }

    const int lower_delta = next_top - lower_cluster.top;
    for (auto id : lower_ids)
    {
      if (auto ctrl = GetDlgItem(id); ctrl)
      {
        CRect rect;
        ctrl->GetWindowRect(&rect);
        ScreenToClient(&rect);
        ctrl->SetWindowPos(nullptr, rect.left, rect.top + lower_delta,
                           0, 0, SWP_NOZORDER | SWP_NOSIZE);
      }
    }

    CRect dialog_rect;
    GetWindowRect(&dialog_rect);
    CRect client_rect;
    GetClientRect(&client_rect);
    const int frame_extra       = dialog_rect.Height() - client_rect.Height();
    const int bottom_margin     = client_rect.bottom - lower_cluster.bottom;
    const int new_client_height = lower_cluster.bottom + lower_delta + bottom_margin;

    SetWindowPos(nullptr, 0, 0, dialog_rect.Width(),
                 new_client_height + frame_extra,
                 SWP_NOMOVE | SWP_NOZORDER);
  }
}

UINT CFilterKeySettingDlg::GetPresetButtonControlId(const int preset) const
{
  return dynamic_preset_button_base_ + static_cast<UINT>(preset);
}

bool CFilterKeySettingDlg::IsEditModeChecked() const
{
  auto edit_button = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_EDIT_MODE));
  return (edit_button && edit_button->GetCheck() == BST_CHECKED);
}

bool CFilterKeySettingDlg::IsDialogForeground() const
{
  HWND owner = GetSafeHwnd();
  if (!::IsWindow(owner))
    return false;

  HWND fg = ::GetForegroundWindow();
  if (!::IsWindow(fg))
    return false;

  if (fg == owner)
    return true;

  return (::IsChild(owner, fg) == TRUE);
}

bool CFilterKeySettingDlg::SaveCurrentEditingValues(const int target_preset)
{
  if (!PRESET_IS_ON(target_preset))
    return true;

  PresetOption option(target_preset);

  const auto CheckAndSet = [&](int control_id, auto&& key,
                               DWORD min_value, DWORD max_value,
                               LPCTSTR tag) {
    BOOL ok = FALSE;
    UINT u  = GetDlgItemInt(control_id, &ok, FALSE);
    if (!ok)
    {
      AfxMessageBox(_T("숫자 형식이 올바르지 않습니다."));
      GetDlgItem(control_id)->SetFocus();
      return false;
    }

    DWORD v = static_cast<DWORD>(u);
    if (v < min_value || v > max_value)
    {
      CString rangeErrorMessage;
      rangeErrorMessage.Format(_T("%s 값은 %lu에서 %lu 사이여야 합니다."), tag,
                               min_value, max_value);
      AfxMessageBox(rangeErrorMessage);
      GetDlgItem(control_id)->SetFocus();
      return false;
    }

    option.set(key, v);
    return true;
  };

  if (!CheckAndSet(IDC_EDIT_ACCEPT_DELAY, KEY_ACCEPT_DELAY, 0, 2000, _T("Accept Delay")))
    return false;
  if (!CheckAndSet(IDC_EDIT_REPEAT_DELAY, KEY_REPEAT_DELAY, 0, 2000, _T("Repeat Delay")))
    return false;
  if (!CheckAndSet(IDC_EDIT_REPEAT_RATE, KEY_REPEAT_RATE, 0, 1000, _T("Repeat Rate")))
    return false;

  return true;
}

void CFilterKeySettingDlg::OnCommandPresetButton(UINT nID)
{
  const int preset = static_cast<int>(nID - dynamic_preset_button_base_);
  BnClickPreset(preset);
}
