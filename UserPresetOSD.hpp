#pragma once

#include "UserOption.hpp"

namespace UserPresetOSD {

enum class Corner : int
{
  TopLeft = 0,
  TopRight,
  BottomLeft,
  BottomRight,
};

void InitializeOptionDefaults();
void InitializeOptionUI(CWnd* owner);
void LayoutOptionUI(CWnd* owner);
void SaveOptionFromUI(CWnd* owner);
void ShowPresetIndex(int preset_index);

}  // namespace UserPresetOSD
