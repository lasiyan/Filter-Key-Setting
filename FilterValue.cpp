
// clang-format off
#include "pch.h"
#include "FilterValue.h"
#include <memory>
// clang-format on

namespace FilterKey {

bool Container::initialize()
{
  bool res = true;

  // Make Common option
  if (Container::exist(USER_REGISTRY_PATH) == false) {
    Container preset(USER_REGISTRY_PATH);

    res &= preset.set(KEY_RESTORE_SETTING, DWORD(1));
    res &= preset.set(KEY_DISABLE_HOTKEY, DWORD(0));
    res &= preset.set(KEY_FAST_APPLY, DWORD(0));
    res &= preset.set(KEY_LAST_PRESET, DWORD(0));
  }

  // Make Preset 1
  if (Container::exist(PRESET::ONE) == false) {
    Container preset(PRESET::ONE);

    res &= preset.set(KEY_PRESET_TITLE, CString(_T("켜기")));
    res &= preset.set(KEY_ACCEPT_DELAY, DWORD(0));
    res &= preset.set(KEY_REPEAT_DELAY, DWORD(90));
    res &= preset.set(KEY_REPEAT_RATE, DWORD(14));
    res &= preset.set(KEY_FILTER_FLAG, DWORD(CUSTOM_FILTER_FLAG));
  }

  // Make Preset 2
  if (Container::exist(PRESET::TWO) == false) {
    Container preset(PRESET::TWO);

    res &= preset.set(KEY_PRESET_TITLE, CString(_T("프리셋 2")));
    res &= preset.set(KEY_ACCEPT_DELAY, DWORD(0));
    res &= preset.set(KEY_REPEAT_DELAY, DWORD(500));
    res &= preset.set(KEY_REPEAT_RATE, DWORD(33));
    res &= preset.set(KEY_FILTER_FLAG, DWORD(CUSTOM_FILTER_FLAG));
  }

  return (res ? TRUE : FALSE);
}

CString Container::section(PRESET preset)
{
  CString section;
  section.Format(_T("%s\\Preset%d"), USER_REGISTRY_PATH, preset);
  return section;
}

bool Container::exist(PRESET preset)
{
  return Container::exist(Container::section(preset));
}

bool Container::exist(CString section)
{
  HKEY hKey = NULL;
  auto res  = RegOpenKeyEx(REG_TARGET, section, 0L, KEY_ALL_ACCESS, &hKey);
  RegCloseKey(hKey);

  if (res == ERROR_SUCCESS) {
    if (hKey != NULL) {
      return true;
    }
  }
  return false;
}

Container::Container(CString section)
{
  section_ = section;

  RegCreateKeyEx(REG_TARGET, section_, 0, NULL, REG_OPTION_NON_VOLATILE,
                 KEY_ALL_ACCESS, NULL, &registry_, &dwDisposition_);
}

Container::Container(PRESET preset)
{
  number_  = static_cast<int>(preset);
  section_ = Container::section(preset);

  RegCreateKeyEx(REG_TARGET, section_, 0, NULL, REG_OPTION_NON_VOLATILE,
                 KEY_ALL_ACCESS, NULL, &registry_, &dwDisposition_);
}

Container::~Container()
{
  if (registry_ != NULL) {
    RegCloseKey(registry_);
  }
}

template <typename T>
bool Container::set(CString key, const T value)
{
  if (std::is_same<T, CString>()) {
    CString str;
    str.Format(_T("%s"), value);

    return (RegSetValueEx(registry_, key, 0, REG_SZ, (CONST BYTE*)(LPCTSTR)str,
                          (DWORD)(_tcslen(str) + 1) * sizeof(TCHAR)) ==
            ERROR_SUCCESS);
  }
  else if (std::is_same<T, int>()) {
    return (RegSetValueEx(registry_, key, 0, REG_DWORD, (CONST BYTE*)&value,
                          sizeof(value)) == ERROR_SUCCESS);
  }
  else if (std::is_same<T, DWORD>()) {
    return (RegSetValueEx(registry_, key, 0, REG_DWORD, (CONST BYTE*)&value,
                          sizeof(value)) == ERROR_SUCCESS);
  }
  return false;
}

DWORD Container::getInteger(CString key)
{
  DWORD value = 0x00000000;
  DWORD size  = sizeof(value);

  RegQueryValueEx(registry_, key, NULL, NULL, (LPBYTE)&value, &size);
  return value;
}

CString Container::getString(CString key)
{
  CString value;
  DWORD   data_size = 0;

  if (RegQueryValueEx(registry_, key, NULL, NULL, NULL, &data_size) ==
      ERROR_SUCCESS) {
    auto data = new TCHAR[data_size];
    RegQueryValueEx(registry_, key, NULL, NULL, (LPBYTE)data, &data_size);
    value = reinterpret_cast<TCHAR*>(data);
    delete[] data;
  }
  return value;
}

}  // namespace FilterKey