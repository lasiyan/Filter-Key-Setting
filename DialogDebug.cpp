// clang-format off
#include "pch.h"
#include "DialogDebug.h"
#include "FilterKeySetting.h"
#include "UserPresetOSD.hpp"
#include "afxdialogex.h"
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
ON_CBN_SELCHANGE(IDC_COMBO_PRESET_OSD_CORNER, &DialogDebug::OnCbnSelchangeComboPresetOsdCorner)
ON_CBN_SELCHANGE(IDC_COMBO_PRESET_OSD_SIZE, &DialogDebug::OnCbnSelchangeComboPresetOsdSize)
ON_BN_CLICKED(IDC_RADIO_PRESET_OSD_KEEP, &DialogDebug::OnBnClickedRadioPresetOsdKeep)
ON_BN_CLICKED(IDC_RADIO_PRESET_OSD_3SEC, &DialogDebug::OnBnClickedRadioPresetOsd3sec)
ON_BN_CLICKED(IDC_BTN_DBG_APPLY_PRESET_COUNT, &DialogDebug::OnBnClickedBtnDbgApplyPresetCount)
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

  return TRUE;
}

BOOL DialogDebug::PreTranslateMessage(MSG* pMsg)
{
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

void DialogDebug::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);

  if (pScrollBar && pScrollBar->GetDlgCtrlID() == IDC_SLIDER_PRESET_OSD_ALPHA)
  {
    UserPresetOSD::SaveOptionFromUI(this);
    NotifyOptionsChanged();
  }
}

void DialogDebug::OnBnClickedBtnDbgApplyPresetCount()
{
  auto* combo = reinterpret_cast<CComboBox*>(GetDlgItem(IDC_COMBO_DBG_PRESET_COUNT));
  if (!combo)
    return;

  int selected = combo->GetCurSel();
  if (selected == CB_ERR)
  {
    AfxMessageBox(_T("프리셋 개수를 먼저 선택하세요."));
    return;
  }

  const int preset_count = PRESET_MIN_COUNT + selected;
  GLOBAL_OPTION.set(KEY_PRESET_COUNT, static_cast<DWORD>(preset_count));

  CString msg;
  msg.Format(_T("프리셋 개수 %d개로 저장되었습니다. 적용하려면 프로그램을 다시 시작하세요."), preset_count);
  AfxMessageBox(msg);

  DevLog::Writef(_T("Option changed: preset_count = %d (restart required)"), preset_count);
}
