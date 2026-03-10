#pragma once

#include <atomic>

// Sink interface
class DevLogSink
{
 public:
  virtual ~DevLogSink()                          = default;
  virtual void AppendDevLog(const CString& line) = 0;
};

// Static logger
class DevLog
{
 public:
  // Sink management
  static void AttachSink(DevLogSink* sink);
  static void DetachSink(DevLogSink* sink);
  static bool IsEnabled();

  // Output
  static void Write(const CString& text);
  static void Writef(LPCTSTR format, ...);

 private:
  static std::atomic<DevLogSink*> sink_;
};
