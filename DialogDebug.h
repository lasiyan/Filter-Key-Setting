#pragma once

#include "UserDevLog.hpp"
#include "UserOption.hpp"

class DialogDebug : public CDialogEx, public DevLogSink
{
  DECLARE_DYNAMIC(DialogDebug)

 public:
  explicit DialogDebug(CWnd* pParent = nullptr);
  ~DialogDebug() override;

#ifdef AFX_DESIGN_TIME
  enum
  {
    IDD = IDD_DEBUG_CONSOLE
  };
#endif

 protected:
  void DoDataExchange(CDataExchange* pDX) override;
  BOOL OnInitDialog() override;
  BOOL PreTranslateMessage(MSG* pMsg) override;
  void OnCancel() override;
  void PostNcDestroy() override;

  afx_msg void OnClose();
  afx_msg void OnDestroy();
  afx_msg void OnBnClickedCheckDbgMoveTracker();
  afx_msg void OnBnClickedCheckSyncFilterkey();
  afx_msg void OnBnClickedCheckMuteSound();
  afx_msg void OnBnClickedCheckOffUseWindowsDefault();
  afx_msg void OnBnClickedCheckEnablePresetOsd();
  afx_msg void OnCbnSelchangeComboPresetOsdCorner();
  afx_msg void OnCbnSelchangeComboPresetOsdSize();
  afx_msg void OnBnClickedRadioPresetOsdKeep();
  afx_msg void OnBnClickedRadioPresetOsd3sec();
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnBnClickedBtnDbgApplyPresetCount();

  DECLARE_MESSAGE_MAP()

 public:
  void AppendDevLog(const CString& line) override;

 private:
  void InitializeOptions();
  void NotifyOptionsChanged();

 private:
  CString log_text_;

  static constexpr int kLogMaxChars   = 200000;
  static constexpr int kTrimKeepChars = 120000;
};
