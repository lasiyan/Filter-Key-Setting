#pragma once

#include "UserDefine.hpp"

bool InitializeOptionValues();

class OptionBase
{
 protected:
  explicit OptionBase(const CString& section);

 public:
  virtual ~OptionBase();

  template <typename T>
  bool set(const CString& key, const T& value, bool if_not_exist = false);
  template <typename T>
  bool setInit(const CString& key, const T& value);

  DWORD   getInteger(const CString& key, DWORD default_value = 0);
  CString getString(const CString& key, const CString& default_value = CString());

 protected:
  HKEY    registry_      = NULL;
  CString section_       = CString();
  DWORD   dwDisposition_ = 0x00000000;
};

class GlobalOption : public OptionBase
{
 public:
  static GlobalOption& instance()
  {
    static GlobalOption instance;
    return instance;
  }

 private:
  GlobalOption();
  ~GlobalOption()                              = default;
  GlobalOption(const GlobalOption&)            = delete;
  GlobalOption& operator=(const GlobalOption&) = delete;
  GlobalOption(GlobalOption&&)                 = delete;
  GlobalOption& operator=(GlobalOption&&)      = delete;
};
#define GLOBAL_OPTION (GlobalOption::instance())

class PresetOption : public OptionBase
{
  static CString SECTION(int number);

 public:
  static CString TAG(const int number);

  PresetOption(const int number);  // Start from 1
};
