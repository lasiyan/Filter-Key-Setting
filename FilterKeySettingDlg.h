
// FilterKeySettingDlg.h : header file
//

#pragma once

#include "FilterKeySettingDefine.h"
#include "FilterValue.h"

// CFilterKeySettingDlg dialog
class CFilterKeySettingDlg : public CDialogEx {
  // Construction
 public:
  CFilterKeySettingDlg(CWnd* pParent = nullptr);  // standard constructor
  ~CFilterKeySettingDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_FILTERKEYSETTING_DIALOG };
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
  DECLARE_MESSAGE_MAP()

  afx_msg void OnBnClickedPresetOff();
  afx_msg void OnBnClickedPreset1();
  afx_msg void OnBnClickedPreset2();
  afx_msg void OnBnClickedApply();
  afx_msg void OnBnClickedCheckFastApply();

  afx_msg void OnEnSetFocusTesting();
  afx_msg void OnEnKillFocusTesting();

  /////////////////////////////////////////////
  void PopupRenameDialog();

  void ApplyFilterKey(BOOL alert = TRUE);
  void UpdateInterface();
  void UpdateFilterKey();
  void UpdateOption(BOOL write = TRUE);

  virtual BOOL PreTranslateMessage(MSG* pMsg);

 private:
  FilterKey::Container* global_config_ = nullptr;
  FilterKey::Container* preset_        = nullptr;
  bool                  press_ctrl_    = false;
  bool                  fast_apply_    = false;

 public:
  afx_msg void OnBnClickedCheckRestoreSetting();
  afx_msg void OnBnClickedCheckDisableHotkey();
};
