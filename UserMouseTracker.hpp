#pragma once

#include <windows.h>

#include <atomic>

#define TIMER_MOUSE_MOVE_WATCH (WM_USER + 1)

enum class MouseTrackerTrigger : WPARAM
{
  MoveDistance = 1,  // Triggered by accumulated movement threshold
  DoubleClick  = 2,  // Triggered by double-click detection
};

class MouseTracker
{
 public:
  MouseTracker();
  ~MouseTracker();

  bool Initialize(HWND  owner_wnd,
                  DWORD interval_ms           = 50,
                  DWORD move_threshold_px     = 2,
                  DWORD distance_target_px    = 120,
                  DWORD double_click_ms       = 350,
                  DWORD double_click_range_px = 12);
  void Release();

  bool Active();
  void Deactive();
  void SetTriggerOptions(bool enable_move, bool enable_double_click);

 private:
  static VOID CALLBACK TimerProc(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
  void                 Tick();

  void      ResetTracking();
  bool      IsOwnerForeground() const;
  ULONGLONG NowMs() const;

 private:
  // lifetime / infra
  HWND              owner_wnd_;     // Owner window handle to receive messages
  HANDLE            timer_queue_;   // Timer queue handle
  HANDLE            timer_handle_;  // Periodic tick timer handle
  std::atomic<bool> initialized_;   // Initialize/Release lifecycle state
  std::atomic<bool> active_;        // Whether timer is currently running

  // config
  DWORD interval_ms_;            // Polling interval (ms)
  DWORD move_threshold_px_;      // Min axis delta treated as movement (px)
  DWORD distance_target_px_;     // Accumulated distance to fire move trigger (px)
  DWORD double_click_ms_;        // Max interval between two clicks (ms)
  DWORD double_click_range_px_;  // Max position delta between two clicks (px)
  DWORD arm_delay_ms_;           // Delay after background transition before tracking (ms)

  // tracking state
  bool              has_last_cursor_pos_;          // Whether previous cursor position is valid
  POINT             last_cursor_pos_;              // Cursor position at previous tick
  ULONGLONG         moved_distance_px_;            // Accumulated movement distance (Manhattan)
  bool              last_left_button_down_;        // Left button state at previous tick
  bool              has_last_click_;               // Whether previous click info is valid
  POINT             last_click_pos_;               // Previous click position
  ULONGLONG         last_click_ms_;                // Previous click timestamp (ms)
  ULONGLONG         trigger_cooldown_until_ms_;    // Cooldown end timestamp to avoid retrigger (ms)
  ULONGLONG         background_since_ms_;          // Timestamp when app entered background (ms)
  std::atomic<bool> enable_move_trigger_;          // Enable movement trigger
  std::atomic<bool> enable_double_click_trigger_;  // Enable double-click trigger
  bool              in_background_;                // Whether tracker currently sees background state
};
