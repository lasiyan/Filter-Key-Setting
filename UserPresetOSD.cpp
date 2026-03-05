// clang-format off
#include "pch.h"
#include "UserPresetOSD.hpp"
#include "resource.h"
// clang-format on

namespace {
class PresetOSDWindow : public CWnd
{
 public:
  bool EnsureCreated()
  {
    if (::IsWindow(GetSafeHwnd()))
      return true;

    CString class_name = AfxRegisterWndClass(0);
    return CreateEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                    class_name,
                    _T("PresetOSD"),
                    WS_POPUP,
                    CRect(0, 0, 1, 1),
                    nullptr,
                    0);
  }

  void ShowText(const CString&        text,
                BYTE                  alpha,
                UserPresetOSD::Corner corner,
                int                   size_percent,
                UINT                  auto_hide_ms)
  {
    if (!EnsureCreated())
      return;

    text_ = text;

    const RECT target_work = ResolveTargetWorkRect();
    const int  work_h      = target_work.bottom - target_work.top;
    const int  target_h    = max(30, (work_h * size_percent) / 100);
    const int  font_px     = max(12, (target_h * 70) / 100);

    EnsureFont(font_px);

    CClientDC dc(this);
    HGDIOBJ   old_font = dc.SelectObject(&font_);

    CRect calc(0, 0, 0, 0);
    dc.DrawText(text_, &calc, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);

    dc.SelectObject(old_font);

    const int pad_x = max(4, font_px / 5);
    const int width = calc.Width() + pad_x * 2;
    const int hgt   = target_h;
    POINT     pt    = ResolveCornerPoint(target_work, width, hgt, corner);

    SetWindowPos(&CWnd::wndTopMost, pt.x, pt.y, width, hgt,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    ::SetLayeredWindowAttributes(GetSafeHwnd(), 0, alpha, LWA_ALPHA);

    KillTimer(1);
    if (auto_hide_ms > 0)
      SetTimer(1, auto_hide_ms, nullptr);
    Invalidate(FALSE);
    UpdateWindow();
  }

  void UpdateAlpha(BYTE alpha)
  {
    if (!::IsWindow(GetSafeHwnd()))
      return;

    ::SetLayeredWindowAttributes(GetSafeHwnd(), 0, alpha, LWA_ALPHA);
    Invalidate(FALSE);
    UpdateWindow();
  }

  void ReapplyCurrent(BYTE                  alpha,
                      UserPresetOSD::Corner corner,
                      int                   size_percent,
                      UINT                  auto_hide_ms,
                      bool                  show_if_hidden)
  {
    if (!show_if_hidden && !IsShowing())
      return;

    if (text_.IsEmpty())
      return;

    ShowText(text_, alpha, corner, size_percent, auto_hide_ms);
  }

  bool IsShowing() const
  {
    return (::IsWindow(GetSafeHwnd()) && ::IsWindowVisible(GetSafeHwnd()));
  }

 protected:
  afx_msg void OnPaint()
  {
    CPaintDC dc(this);
    CRect    rc;
    GetClientRect(&rc);

    dc.FillSolidRect(&rc, RGB(22, 22, 22));
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(255, 70, 70));
    dc.SelectObject(&font_);

    dc.DrawText(text_, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
  }

  afx_msg void OnTimer(UINT_PTR id)
  {
    if (id == 1)
    {
      KillTimer(1);
      ShowWindow(SW_HIDE);
      return;
    }

    CWnd::OnTimer(id);
  }

  DECLARE_MESSAGE_MAP()

 private:
  void EnsureFont(int pixel_height)
  {
    if (font_height_ == pixel_height && font_.GetSafeHandle())
      return;

    if (font_.GetSafeHandle())
      font_.DeleteObject();

    LOGFONT lf  = {};
    lf.lfHeight = -pixel_height;
    lf.lfWeight = FW_BOLD;
    _tcscpy_s(lf.lfFaceName, _T("Segoe UI"));

    font_.CreateFontIndirect(&lf);
    font_height_ = pixel_height;
  }

  static RECT ResolveTargetWorkRect()
  {
    MONITORINFO mi  = { sizeof(mi) };
    HMONITOR    mon = nullptr;

    if (auto* main_wnd = AfxGetMainWnd(); main_wnd)
    {
      const HWND main_hwnd = main_wnd->GetSafeHwnd();
      if (::IsWindow(main_hwnd) && ::IsWindowVisible(main_hwnd) && !::IsIconic(main_hwnd))
        mon = ::MonitorFromWindow(main_hwnd, MONITOR_DEFAULTTOPRIMARY);
    }

    if (!mon)
    {
      POINT cursor = {
        0,
      };
      ::GetCursorPos(&cursor);
      mon = ::MonitorFromPoint(cursor, MONITOR_DEFAULTTOPRIMARY);
    }

    GetMonitorInfo(mon, &mi);
    return mi.rcWork;
  }

  static POINT ResolveCornerPoint(const RECT& work, int width, int height, UserPresetOSD::Corner corner)
  {
    constexpr int margin = 0;

    POINT pt{};
    switch (corner)
    {
      case UserPresetOSD::Corner::TopLeft:
        pt.x = work.left + margin;
        pt.y = work.top + margin;
        break;
      case UserPresetOSD::Corner::TopRight:
        pt.x = work.right - width - margin;
        pt.y = work.top + margin;
        break;
      case UserPresetOSD::Corner::BottomLeft:
        pt.x = work.left + margin;
        pt.y = work.bottom - height - margin;
        break;
      case UserPresetOSD::Corner::BottomRight:
      default:
        pt.x = work.right - width - margin;
        pt.y = work.bottom - height - margin;
        break;
    }

    return pt;
  }

 private:
  CString text_;
  CFont   font_;
  int     font_height_ = 0;
};

BEGIN_MESSAGE_MAP(PresetOSDWindow, CWnd)
ON_WM_PAINT()
ON_WM_TIMER()
END_MESSAGE_MAP()

PresetOSDWindow& OSDWindow()
{
  static PresetOSDWindow wnd;
  return wnd;
}

int ClampPosition(int value)
{
  if (value < static_cast<int>(UserPresetOSD::Corner::TopLeft))
    return static_cast<int>(UserPresetOSD::Corner::TopLeft);
  if (value > static_cast<int>(UserPresetOSD::Corner::BottomRight))
    return static_cast<int>(UserPresetOSD::Corner::BottomRight);
  return value;
}

int ClampAlpha(int value)
{
  if (value < 0)
    return 0;
  if (value > 255)
    return 255;
  return value;
}

int ClampSizePercent(int value)
{
  if (value < 1)
    return 1;
  if (value > 10)
    return 10;
  return value;
}
}  // namespace

namespace UserPresetOSD {

void InitializeOptionDefaults()
{
  GLOBAL_OPTION.setInit(KEY_ENABLE_PRESET_OSD, false);
  GLOBAL_OPTION.setInit(KEY_PRESET_OSD_CORNER, static_cast<DWORD>(Corner::BottomRight));
  GLOBAL_OPTION.setInit(KEY_PRESET_OSD_ALPHA, static_cast<DWORD>(220));
  GLOBAL_OPTION.setInit(KEY_PRESET_OSD_KEEP_VISIBLE, false);
  GLOBAL_OPTION.setInit(KEY_PRESET_OSD_SIZE, static_cast<DWORD>(5));
}

void InitializeOptionUI(CWnd* owner)
{
  if (!owner)
    return;

  if (auto* check = reinterpret_cast<CButton*>(owner->GetDlgItem(IDC_CHECK_ENABLE_PRESET_OSD)); check)
    check->SetCheck(GLOBAL_OPTION.getInteger(KEY_ENABLE_PRESET_OSD, 0) ? BST_CHECKED : BST_UNCHECKED);

  if (auto* combo = reinterpret_cast<CComboBox*>(owner->GetDlgItem(IDC_COMBO_PRESET_OSD_CORNER)); combo)
  {
    combo->ResetContent();
    combo->AddString(_T("좌상단"));
    combo->AddString(_T("우상단"));
    combo->AddString(_T("좌하단"));
    combo->AddString(_T("우하단"));

    const int idx = ClampPosition(static_cast<int>(GLOBAL_OPTION.getInteger(
        KEY_PRESET_OSD_CORNER, static_cast<DWORD>(Corner::BottomRight))));
    combo->SetCurSel(idx);
  }

  if (auto* slider = reinterpret_cast<CSliderCtrl*>(owner->GetDlgItem(IDC_SLIDER_PRESET_OSD_ALPHA)); slider)
  {
    slider->SetRange(0, 255);
    slider->SetTicFreq(15);
    slider->SetPos(
        ClampAlpha(static_cast<int>(GLOBAL_OPTION.getInteger(KEY_PRESET_OSD_ALPHA, 220))));
  }

  if (auto* combo = reinterpret_cast<CComboBox*>(owner->GetDlgItem(IDC_COMBO_PRESET_OSD_SIZE)); combo)
  {
    combo->ResetContent();
    for (int value = 1; value <= 10; ++value)
    {
      CString item;
      item.Format(_T("%d"), value);
      combo->AddString(item);
    }

    const int size_percent = ClampSizePercent(
        static_cast<int>(GLOBAL_OPTION.getInteger(KEY_PRESET_OSD_SIZE, 5)));
    combo->SetCurSel(size_percent - 1);
  }

  const bool keep = (GLOBAL_OPTION.getInteger(KEY_PRESET_OSD_KEEP_VISIBLE, 0) != 0);
  if (auto* keep_radio = reinterpret_cast<CButton*>(owner->GetDlgItem(IDC_RADIO_PRESET_OSD_KEEP)); keep_radio)
    keep_radio->SetCheck(keep ? BST_CHECKED : BST_UNCHECKED);
  if (auto* auto_radio = reinterpret_cast<CButton*>(owner->GetDlgItem(IDC_RADIO_PRESET_OSD_3SEC)); auto_radio)
    auto_radio->SetCheck(keep ? BST_UNCHECKED : BST_CHECKED);
}

void LayoutOptionUI(CWnd* owner)
{
  UNREFERENCED_PARAMETER(owner);
}

void SaveOptionFromUI(CWnd* owner)
{
  if (!owner)
    return;

  bool enabled = false;
  if (auto* check = reinterpret_cast<CButton*>(owner->GetDlgItem(IDC_CHECK_ENABLE_PRESET_OSD)); check)
    enabled = (check->GetCheck() == BST_CHECKED);

  int corner = static_cast<int>(Corner::BottomRight);
  if (auto* combo = reinterpret_cast<CComboBox*>(owner->GetDlgItem(IDC_COMBO_PRESET_OSD_CORNER)); combo)
  {
    const int sel = combo->GetCurSel();
    if (sel != CB_ERR)
      corner = ClampPosition(sel);
  }

  int alpha = 220;
  if (auto* slider = reinterpret_cast<CSliderCtrl*>(owner->GetDlgItem(IDC_SLIDER_PRESET_OSD_ALPHA)); slider)
    alpha = ClampAlpha(slider->GetPos());

  bool keep_visible = false;
  if (auto* keep_radio = reinterpret_cast<CButton*>(owner->GetDlgItem(IDC_RADIO_PRESET_OSD_KEEP)); keep_radio)
    keep_visible = (keep_radio->GetCheck() == BST_CHECKED);

  int size_percent = 5;
  if (auto* combo = reinterpret_cast<CComboBox*>(owner->GetDlgItem(IDC_COMBO_PRESET_OSD_SIZE)); combo)
  {
    const int sel = combo->GetCurSel();
    if (sel != CB_ERR)
      size_percent = sel + 1;
  }
  size_percent = ClampSizePercent(size_percent);

  GLOBAL_OPTION.set(KEY_ENABLE_PRESET_OSD, static_cast<DWORD>(enabled));
  GLOBAL_OPTION.set(KEY_PRESET_OSD_CORNER, static_cast<DWORD>(corner));
  GLOBAL_OPTION.set(KEY_PRESET_OSD_ALPHA, static_cast<DWORD>(alpha));
  GLOBAL_OPTION.set(KEY_PRESET_OSD_KEEP_VISIBLE, static_cast<DWORD>(keep_visible));
  GLOBAL_OPTION.set(KEY_PRESET_OSD_SIZE, static_cast<DWORD>(size_percent));

  if (!enabled)
  {
    if (OSDWindow().IsShowing())
      OSDWindow().ShowWindow(SW_HIDE);
    return;
  }

  const UINT hide_ms = keep_visible ? 0u : 3000u;
  OSDWindow().ReapplyCurrent(
      static_cast<BYTE>(alpha), static_cast<Corner>(corner), size_percent, hide_ms, keep_visible);
}

void ShowPresetIndex(int preset_index)
{
  if (GLOBAL_OPTION.getInteger(KEY_ENABLE_PRESET_OSD, 0) == 0)
    return;

  const int corner = ClampPosition(static_cast<int>(GLOBAL_OPTION.getInteger(
      KEY_PRESET_OSD_CORNER, static_cast<DWORD>(Corner::BottomRight))));
  const int alpha  = ClampAlpha(static_cast<int>(GLOBAL_OPTION.getInteger(KEY_PRESET_OSD_ALPHA, 220)));

  const bool keep_visible = (GLOBAL_OPTION.getInteger(KEY_PRESET_OSD_KEEP_VISIBLE, 0) != 0);
  const UINT hide_ms      = keep_visible ? 0u : 3000u;
  const int  size_percent = ClampSizePercent(
      static_cast<int>(GLOBAL_OPTION.getInteger(KEY_PRESET_OSD_SIZE, 5)));

  if (preset_index < 0)
    preset_index = 0;
  if (preset_index > 99)
    preset_index = 99;

  CString text;
  text.Format(_T("%d"), preset_index);

  OSDWindow().ShowText(
      text, static_cast<BYTE>(alpha), static_cast<Corner>(corner), size_percent, hide_ms);
}

}  // namespace UserPresetOSD
