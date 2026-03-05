
// FilterKeySettingDlg.h : header file
//

#pragma once

#include <memory>
#include <vector>

#include "FilterKeySettingDefine.h"
#include "UserMouseTracker.hpp"
#include "UserOption.hpp"

class DialogDebug;

// CFilterKeySettingDlg dialog
class CFilterKeySettingDlg : public CDialogEx
{
  // Construction
 public:
  CFilterKeySettingDlg(CWnd* pParent = nullptr);  // standard constructor
  ~CFilterKeySettingDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
  enum
  {
    IDD = IDD_FILTERKEYSETTING_DIALOG
  };
#endif

 protected:
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

  // Implementation
 protected:
  HICON m_hIcon;

  // Generated message map functions
  virtual BOOL    OnInitDialog();
  afx_msg void    OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void    OnDestroy();
  afx_msg void    OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
  afx_msg void    OnSize(UINT nType, int cx, int cy);
  afx_msg void    OnTimer(UINT_PTR nIDEvent);
  afx_msg LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnMouseTrackerTriggered(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnDebugDialogClosed(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnDebugOptionsChanged(WPARAM wParam, LPARAM lParam);
  DECLARE_MESSAGE_MAP()

  virtual BOOL PreTranslateMessage(MSG* pMsg);

  afx_msg void OnCommandPresetButton(UINT nID);

  afx_msg void OnBnClickedCheckEditMode();
  afx_msg void OnBnClickedCheckRestoreSetting();
  afx_msg void OnBnClickedCheckDisableHotkey();
  afx_msg void OnBnClickedCheckMoveToTray();
  afx_msg void OnBnClickedCheckEnableKeybind();
  afx_msg void OnBnClickedCheckEnableToggleKeybind();
  afx_msg void OnBnClickedCheckSetMouseDblclickTracker();
  afx_msg void OnBnClickedCheckDisableWithEsc();

  afx_msg void OnEnSetFocusTesting();
  afx_msg void OnEnKillFocusTesting();
  afx_msg void OnEnSetFocusToggleKeybind();

  /////////////////////////////////////////////
  void BnClickPreset(const int preset);
  void PopupRenameDialog(const int preset);
  void PopupKeyBindingDialog(const int preset);
  void PopupToggleKeyBindingDialog();
  void ActivePreset(const int preset, BOOL alert, BOOL beep = FALSE);
  void ResetEditMode();
  bool ValidateActiveHotkeysAndAlert() const;
  void RefreshToggleHotkeyEditText();
  void UpdateEscDisableHotkeyRegistration();

  void UpdateOption(BOOL write = TRUE);
  void UpdateInterface(const int preset);
  bool AddTrayIcon();
  void RemoveTrayIcon();
  void RestoreFromTray();
  void RegisterPresetHotkeys();
  void UnregisterPresetHotkeys();
  void RefreshPresetButtonCaption(const int preset);
  void RefreshAllPresetButtonCaptions();
  void InitializePresetCount();
  void OpenDeveloperDebugDialog();
  void BuildPresetButtons();
  void LayoutDynamicControls();
  UINT GetPresetButtonControlId(const int preset) const;
  bool IsEditModeChecked() const;
  bool IsDialogForeground() const;
  bool SaveCurrentEditingValues(const int target_preset);

 private:
  int last_selected_      = PRESET_OFF;
  int preset_before_edit_ = PRESET_OFF;
  int preset_count_       = 0;
  int last_on_preset_     = 1;

  // System Tray Icon
  static constexpr UINT     WM_TRAYICON_MSG                      = WM_APP + 1;
  static constexpr UINT     WM_DEV_DEBUG_CLOSED                  = WM_APP + 60;
  static constexpr UINT     WM_DEV_DEBUG_OPTIONS_CHANGED         = WM_APP + 61;
  static constexpr UINT_PTR TIMER_BG_ESC_WATCH                   = 0x12F1;
  static constexpr UINT     TRAY_ICON_ID                         = 1;
  NOTIFYICONDATA            tray_icon_data_                      = {};
  bool                      tray_icon_added_                     = false;
  bool                      hotkey_registered_[PRESET_MAX_COUNT] = {
    false,
  };
  bool                                  toggle_hotkey_registered_   = false;
  static constexpr UINT                 dynamic_preset_button_base_ = 2000;
  std::vector<std::unique_ptr<CButton>> preset_buttons_;
  bool                                  alt_hotkey_view_              = false;
  bool                                  opening_toggle_hotkey_dialog_ = false;
  bool                                  bg_esc_watch_active_          = false;
  bool                                  bg_esc_prev_down_             = false;
  MouseTracker                          mouse_tracker_;
  DialogDebug*                          debug_dialog_ = nullptr;
};
