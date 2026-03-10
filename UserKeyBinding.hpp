#pragma once

#include "UserOption.hpp"

namespace KeyBinding {

// Hotkey ID constants
constexpr int HOTKEY_ID_BASE   = 0x1200;
constexpr int TOGGLE_HOTKEY_ID = 0x12F0;

// Hotkey pack/unpack helpers (DWORD = [modifiers:16 | vk:16])
inline UINT  HotkeyVK(DWORD hotkey) { return static_cast<UINT>(hotkey & 0xFFFF); }
inline UINT  HotkeyModifiers(DWORD hotkey) { return static_cast<UINT>((hotkey >> 16) & 0xFFFF); }
inline DWORD ComposeHotkey(UINT vk, UINT modifiers)
{
  return (static_cast<DWORD>(modifiers) << 16) | static_cast<DWORD>(vk);
}

// Validation
struct HotkeyValidation
{
  bool    ok;
  CString error_message;
};

HotkeyValidation ValidateHotkeyCandidate(int preset_count, DWORD hotkey_value,
                                         int exclude_preset, bool is_toggle);

// Global state query
bool  IsEnabled();
bool  IsToggleEnabled();
DWORD GetToggleHotkey();

// String conversion
CString HotkeyToString(DWORD hotkey_value, bool with_not_set_placeholder = false);

// Modifier helpers
bool IsModifierVirtualKey(UINT vk);
UINT CurrentModifiers();

// Duplicate detection
bool FindDuplicatePresetHotkey(int target_preset, int preset_count,
                               DWORD hotkey_value, int* duplicate_preset);
bool FindDuplicateActiveHotkeyCandidate(int      preset_count,
                                        DWORD    hotkey_value,
                                        int      ignore_preset,
                                        bool     ignore_toggle,
                                        CString* duplicate_target_desc);
bool ValidateActiveHotkeys(int preset_count, CString* conflict_message);
bool IsActiveHotkeyAssigned(UINT vk, UINT modifiers, int preset_count);

// Toggle
int ResolveToggleTargetPreset(int current_preset, int last_on_preset, int preset_count);

// Registration
void RegisterPresetHotkeys(HWND hWnd, int preset_count,
                           bool registered_flags[PRESET_MAX_COUNT],
                           int  hotkey_id_base = HOTKEY_ID_BASE);
void UnregisterPresetHotkeys(HWND hWnd, int preset_count,
                             bool registered_flags[PRESET_MAX_COUNT],
                             int  hotkey_id_base = HOTKEY_ID_BASE);
void RegisterConfiguredHotkeys(HWND hWnd, int preset_count,
                               bool  registered_flags[PRESET_MAX_COUNT],
                               bool* toggle_registered,
                               int   hotkey_id_base   = HOTKEY_ID_BASE,
                               int   toggle_hotkey_id = TOGGLE_HOTKEY_ID);
void UnregisterConfiguredHotkeys(HWND hWnd, int preset_count,
                                 bool  registered_flags[PRESET_MAX_COUNT],
                                 bool* toggle_registered,
                                 int   hotkey_id_base   = HOTKEY_ID_BASE,
                                 int   toggle_hotkey_id = TOGGLE_HOTKEY_ID);

}  // namespace KeyBinding
