#pragma once

// DialogRename 대화 상자

class DialogRename : public CDialogEx
{
  DECLARE_DYNAMIC(DialogRename)

 public:
  // 표준 생성자입니다.
  DialogRename(CString origin_name, CWnd* pParent = nullptr);
  virtual ~DialogRename();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
  enum
  {
    IDD = IDD_RENAME_PRESET
  };
#endif

 protected:
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV 지원입니다.

  DECLARE_MESSAGE_MAP()

 public:
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  afx_msg void OnEnSetfocusEditChangeName();

 public:
  CString new_name_;
  BOOL    reset_once_;
};
