// clang-format off
#include "pch.h"
#include "DialogDebug.h"
#include "FilterKeySetting.h"
#include "UserPresetOSD.hpp"
#include "afxdialogex.h"
#include <shellapi.h>
// clang-format on

IMPLEMENT_DYNAMIC(DialogDebug, CDialogEx)

DialogDebug::DialogDebug(CWnd* pParent) : CDialogEx(IDD_DEBUG_CONSOLE, pParent)
{
}

DialogDebug::~DialogDebug()
{
}

void DialogDebug::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_EDIT_DEBUG_LOG, log_text_);
}

BEGIN_MESSAGE_MAP(DialogDebug, CDialogEx)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_CHECK_DBG_MOVE_TRACKER, &DialogDebug::OnBnClickedCheckDbgMoveTracker)
ON_BN_CLICKED(IDC_CHECK_SYNC_FILTERKEY, &DialogDebug::OnBnClickedCheckSyncFilterkey)
ON_BN_CLICKED(IDC_CHECK_MUTE_SOUND, &DialogDebug::OnBnClickedCheckMuteSound)
ON_BN_CLICKED(IDC_CHECK_OFF_USE_WINDOWS_DEFAULT, &DialogDebug::OnBnClickedCheckOffUseWindowsDefault)
ON_BN_CLICKED(IDC_CHECK_ENABLE_PRESET_OSD, &DialogDebug::OnBnClickedCheckEnablePresetOsd)
ON_CBN_SELCHANGE(IDC_COMBO_DBG_PRESET_COUNT, &DialogDebug::OnCbnSelchangeComboDbgPresetCount)
ON_CBN_SELCHANGE(IDC_COMBO_PRESET_OSD_CORNER, &DialogDebug::OnCbnSelchangeComboPresetOsdCorner)
ON_CBN_SELCHANGE(IDC_COMBO_PRESET_OSD_SIZE, &DialogDebug::OnCbnSelchangeComboPresetOsdSize)
ON_BN_CLICKED(IDC_RADIO_PRESET_OSD_KEEP, &DialogDebug::OnBnClickedRadioPresetOsdKeep)
ON_BN_CLICKED(IDC_RADIO_PRESET_OSD_3SEC, &DialogDebug::OnBnClickedRadioPresetOsd3sec)
END_MESSAGE_MAP()

BOOL DialogDebug::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  InitializeOptions();
  DevLog::AttachSink(this);
  DevLog::Write(_T("Developer debug window opened"));

  // Disable resizing
  SetWindowLong(GetSafeHwnd(), GWL_STYLE, GetWindowLong(GetSafeHwnd(), GWL_STYLE) & ~WS_SIZEBOX);

  UserPresetOSD::InitializeOptionUI(this);

  // Tooltip
  tooltip_.Initialize(this);

  return TRUE;
}

BOOL DialogDebug::PreTranslateMessage(MSG* pMsg)
{
  tooltip_.RelayEvent(pMsg);

  if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
      (pMsg->wParam == 'A' || pMsg->wParam == 'a'))
  {
    const bool ctrl_pressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    if (ctrl_pressed)
    {
      CWnd* focus = GetFocus();
      if (focus && focus->GetDlgCtrlID() == IDC_EDIT_DEBUG_LOG)
      {
        auto* edit = reinterpret_cast<CEdit*>(GetDlgItem(IDC_EDIT_DEBUG_LOG));
        if (edit)
        {
          edit->SetSel(0, -1);
          return TRUE;
        }
      }
    }
  }

  return CDialogEx::PreTranslateMessage(pMsg);
}

void DialogDebug::OnCancel()
{
  DestroyWindow();
}

void DialogDebug::OnClose()
{
  DestroyWindow();
}

void DialogDebug::OnDestroy()
{
  DevLog::DetachSink(this);

  CWnd* parent = GetParent();
  if (parent && ::IsWindow(parent->GetSafeHwnd()))
    parent->PostMessage(WM_APP + 60, reinterpret_cast<WPARAM>(this), 0);

  CDialogEx::OnDestroy();
}

void DialogDebug::PostNcDestroy()
{
  CDialogEx::PostNcDestroy();
  delete this;
}

void DialogDebug::AppendDevLog(const CString& line)
{
  if (!::IsWindow(GetSafeHwnd()))
    return;

  CEdit* edit = reinterpret_cast<CEdit*>(GetDlgItem(IDC_EDIT_DEBUG_LOG));
  if (!edit)
    return;

  CString current;
  edit->GetWindowText(current);

  if (!current.IsEmpty())
    current += _T("\r\n");
  current += line;

  if (current.GetLength() > kLogMaxChars)
    current = current.Right(kTrimKeepChars);

  edit->SetWindowText(current);
  edit->SetSel(current.GetLength(), current.GetLength());
}

void DialogDebug::InitializeOptions()
{
  auto* move = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DBG_MOVE_TRACKER));
  if (move)
  {
    const bool checked = (GLOBAL_OPTION.getInteger(KEY_ENABLE_MOUSE_MOVE_TRACKER) != 0);
    move->SetCheck(checked ? BST_CHECKED : BST_UNCHECKED);
  }

  auto* combo = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_COMBO_DBG_PRESET_COUNT));
  if (combo)
  {
    preset_count_combo_updating_ = true;

    combo->ResetContent();
    for (int value = PRESET_MIN_COUNT; value <= PRESET_MAX_COUNT; ++value)
    {
      CString item;
      item.Format(_T("%d"), value);
      combo->AddString(item);
    }

    int current_count = static_cast<int>(GLOBAL_OPTION.getInteger(KEY_PRESET_COUNT));
    if (current_count < PRESET_MIN_COUNT)
      current_count = PRESET_MIN_COUNT;
    if (current_count > PRESET_MAX_COUNT)
      current_count = PRESET_MAX_COUNT;

    combo->SetCurSel(current_count - PRESET_MIN_COUNT);

    preset_count_combo_updating_ = false;
  }

  auto* sync = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_SYNC_FILTERKEY));
  if (sync)
  {
    const bool checked = (GLOBAL_OPTION.getInteger(KEY_SYNC_FILTERKEY, 0) != 0);
    sync->SetCheck(checked ? BST_CHECKED : BST_UNCHECKED);
  }

  auto* mute = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_MUTE_SOUND));
  if (mute)
  {
    const bool checked = (GLOBAL_OPTION.getInteger(KEY_MUTE_SOUND, 0) != 0);
    mute->SetCheck(checked ? BST_CHECKED : BST_UNCHECKED);
  }

  auto* off_behavior = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_OFF_USE_WINDOWS_DEFAULT));
  if (off_behavior)
  {
    const bool checked = (GLOBAL_OPTION.getInteger(KEY_OFF_USE_WINDOWS_DEFAULT, 1) != 0);
    off_behavior->SetCheck(checked ? BST_CHECKED : BST_UNCHECKED);
  }
}

void DialogDebug::NotifyOptionsChanged()
{
  CWnd* parent = GetParent();
  if (parent && ::IsWindow(parent->GetSafeHwnd()))
    parent->PostMessage(WM_APP + 61, 0, 0);
}

void DialogDebug::OnBnClickedCheckDbgMoveTracker()
{
  auto*      button  = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_DBG_MOVE_TRACKER));
  const bool checked = (button && button->GetCheck() == BST_CHECKED);

  GLOBAL_OPTION.set(KEY_ENABLE_MOUSE_MOVE_TRACKER, static_cast<DWORD>(checked));
  DevLog::Writef(_T("Option changed: move tracker = %d"), checked ? 1 : 0);
  NotifyOptionsChanged();
}

void DialogDebug::OnBnClickedCheckSyncFilterkey()
{
  auto*      button  = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_SYNC_FILTERKEY));
  const bool checked = (button && button->GetCheck() == BST_CHECKED);

  GLOBAL_OPTION.set(KEY_SYNC_FILTERKEY, static_cast<DWORD>(checked));
  DevLog::Writef(_T("Option changed: sync filterkey = %d"), checked ? 1 : 0);
  NotifyOptionsChanged();
}

void DialogDebug::OnBnClickedCheckMuteSound()
{
  auto*      button  = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_MUTE_SOUND));
  const bool checked = (button && button->GetCheck() == BST_CHECKED);

  GLOBAL_OPTION.set(KEY_MUTE_SOUND, static_cast<DWORD>(checked));
  DevLog::Writef(_T("Option changed: mute sound = %d"), checked ? 1 : 0);
  NotifyOptionsChanged();
}

void DialogDebug::OnBnClickedCheckOffUseWindowsDefault()
{
  auto*      button  = reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_OFF_USE_WINDOWS_DEFAULT));
  const bool checked = (button && button->GetCheck() == BST_CHECKED);

  GLOBAL_OPTION.set(KEY_OFF_USE_WINDOWS_DEFAULT, static_cast<DWORD>(checked));
  DevLog::Writef(_T("Option changed: off use windows default = %d"), checked ? 1 : 0);
  NotifyOptionsChanged();
}

void DialogDebug::OnBnClickedCheckEnablePresetOsd()
{
  UserPresetOSD::SaveOptionFromUI(this);
  DevLog::Writef(_T("Option changed: preset osd = %d"),
                 GLOBAL_OPTION.getInteger(KEY_ENABLE_PRESET_OSD, 0) ? 1 : 0);
  NotifyOptionsChanged();
}

void DialogDebug::OnCbnSelchangeComboPresetOsdCorner()
{
  UserPresetOSD::SaveOptionFromUI(this);
  NotifyOptionsChanged();
}

void DialogDebug::OnBnClickedRadioPresetOsdKeep()
{
  UserPresetOSD::SaveOptionFromUI(this);
  NotifyOptionsChanged();
}

void DialogDebug::OnBnClickedRadioPresetOsd3sec()
{
  UserPresetOSD::SaveOptionFromUI(this);
  NotifyOptionsChanged();
}

void DialogDebug::OnCbnSelchangeComboPresetOsdSize()
{
  UserPresetOSD::SaveOptionFromUI(this);
  NotifyOptionsChanged();
}

void DialogDebug::OnCbnSelchangeComboDbgPresetCount()
{
  if (preset_count_combo_updating_)
    return;

  auto* combo = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_COMBO_DBG_PRESET_COUNT));
  if (!combo)
    return;

  const int selected = combo->GetCurSel();
  if (selected == CB_ERR)
    return;

  int current_count = static_cast<int>(GLOBAL_OPTION.getInteger(KEY_PRESET_COUNT, PRESET_MIN_COUNT));
  if (current_count < PRESET_MIN_COUNT)
    current_count = PRESET_MIN_COUNT;
  if (current_count > PRESET_MAX_COUNT)
    current_count = PRESET_MAX_COUNT;

  const int selected_count = PRESET_MIN_COUNT + selected;
  if (selected_count == current_count)
    return;

  CString message;
  message.Format(_T("프리셋 개수를 %d개로 변경하려면 프로그램 재시작이 필요합니다.\r\n"
                    "지금 다시 시작하시겠습니까?"),
                 selected_count);

  const int answer = AfxMessageBox(message, MB_ICONQUESTION | MB_YESNO);
  if (answer != IDYES)
  {
    preset_count_combo_updating_ = true;
    combo->SetCurSel(current_count - PRESET_MIN_COUNT);
    preset_count_combo_updating_ = false;
    return;
  }

  GLOBAL_OPTION.set(KEY_PRESET_COUNT, static_cast<DWORD>(selected_count));
  DevLog::Writef(_T("Option changed: preset_count = %d (restart accepted)"), selected_count);

  TCHAR exe_path[MAX_PATH] = {};
  if (::GetModuleFileName(nullptr, exe_path, _countof(exe_path)) == 0)
  {
    AfxMessageBox(_T("실행 파일 경로를 확인할 수 없어 재시작하지 못했습니다."));
    return;
  }

  HINSTANCE launch_result = ::ShellExecute(GetSafeHwnd(), _T("open"), exe_path, nullptr, nullptr, SW_SHOWNORMAL);
  if (reinterpret_cast<INT_PTR>(launch_result) <= 32)
  {
    AfxMessageBox(_T("프로그램 재시작에 실패했습니다."));
    return;
  }

  if (CWnd* parent = GetParent(); parent && ::IsWindow(parent->GetSafeHwnd()))
  {
    parent->PostMessage(WM_CLOSE);
  }
  else
  {
    AfxGetMainWnd()->PostMessage(WM_CLOSE);
  }
}

void DialogDebug::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);

  if (pScrollBar && pScrollBar->GetDlgCtrlID() == IDC_SLIDER_PRESET_OSD_ALPHA)
  {
    UserPresetOSD::SaveOptionFromUI(this);
    NotifyOptionsChanged();
  }
}
