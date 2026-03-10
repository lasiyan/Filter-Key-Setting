#pragma once

#include "UserDefine.hpp"

// Preset value data
struct PresetValues
{
  DWORD accept_delay;
  DWORD repeat_delay;
  DWORD repeat_rate;
};

// Validation result
struct PresetValidation
{
  bool    ok;
  CString field_name;
  CString error_message;
};

namespace PresetService {

// Activation
bool ActivatePreset(int preset, int* out_last_on = nullptr);

// Value query / validation / save
PresetValues     GetPresetValues(int preset);
PresetValidation ValidateValues(const PresetValues& values);
bool             SavePresetValues(int preset, const PresetValues& values, bool* changed = nullptr);

// Preset count
int ResolvePresetCount();

}  // namespace PresetService
