#ifndef __FILTER_VALUE_REGISTRY_HPP__
#define __FILTER_VALUE_REGISTRY_HPP__

#include <winreg.h>

#include "FilterKeySettingDefine.h"

#define REG_TARGET         (HKEY_CURRENT_USER)
#define USER_REGISTRY_PATH ("Software\\FilterKeyHelper")

// Common
#define KEY_RESTORE_SETTING  ("restore_setting")
#define KEY_DISABLE_HOTKEY   ("disable_hotkey")
#define KEY_LAST_PRESET      ("last_preset_number")
#define KEY_FAST_APPLY       ("fast_apply")

#define KEY_PRESET_TITLE     ("preset_title")
#define KEY_ACCEPT_DELAY     ("accept_delay")
#define KEY_REPEAT_DELAY     ("repeat_delay")
#define KEY_REPEAT_RATE      ("repeat_rate")
#define KEY_FILTER_FLAG      ("filter_flags")

#define DEFAULT_ACCEPT_DELAY (1000)
#define DEFAULT_REPEAT_DELAY (1000)
#define DEFAULT_REPEAT_RATE  (500)
#define WINDOW_FILTER_FLAG   (126)
#define CUSTOM_FILTER_FLAG   (35)

namespace FilterKey {

class Container {
 public:
  static bool    initialize();
  static CString section(PRESET preset);
  static bool    exist(PRESET preset);
  static bool    exist(CString section);

 public:
  Container(CString section);  // Global
  Container(PRESET preset);
  ~Container();

 public:
  template <typename T>
  bool set(CString key, const T value);

  DWORD   getInteger(CString key);
  CString getString(CString key);

  int number() { return number_; }

 private:
  HKEY    registry_      = NULL;
  CString section_       = "";
  DWORD   dwDisposition_ = 0x00000000;
  int     number_        = 0;
};

}  // namespace FilterKey

#endif  // __FILTER_VALUE_REGISTRY_HPP__
