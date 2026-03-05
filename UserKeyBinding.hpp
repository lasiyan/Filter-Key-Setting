#pragma once

#include "UserOption.hpp"

namespace KeyBinding {

constexpr int HOTKEY_ID_BASE   = 0x1200;
constexpr int TOGGLE_HOTKEY_ID = 0x12F0;

bool IsEnabled();
bool IsToggleEnabled();

DWORD GetToggleHotkey();

CString HotkeyToString(DWORD hotkey_value, bool with_not_set_placeholder = false);

bool IsModifierVirtualKey(UINT vk);
UINT CurrentModifiers();

bool FindDuplicatePresetHotkey(int target_preset, int preset_count,
                               DWORD hotkey_value, int* duplicate_preset);

bool FindDuplicateActiveHotkeyCandidate(int      preset_count,
                                        DWORD    hotkey_value,
                                        int      ignore_preset,
                                        bool     ignore_toggle,
                                        CString* duplicate_target_desc);

bool ValidateActiveHotkeys(int preset_count, CString* conflict_message);
bool IsActiveHotkeyAssigned(UINT vk, UINT modifiers, int preset_count);

int ResolveToggleTargetPreset(int current_preset, int last_on_preset, int preset_count);

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
