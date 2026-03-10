#pragma once

#include "UserDefine.hpp"

namespace UserPresetOSD {

// Display corner position
enum class Corner : int
{
  TopLeft = 0,
  TopRight,
  BottomLeft,
  BottomRight,
};

// Initialization
void InitializeOptionDefaults();

// Display
void ShowPresetIndex(int preset_index);
void RefreshDisplay();

// Value clamping helpers
int ClampPosition(int value);
int ClampAlpha(int value);
int ClampSizePercent(int value);

}  // namespace UserPresetOSD
