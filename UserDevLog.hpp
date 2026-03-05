#pragma once

#include <atomic>

class DevLogSink
{
 public:
  virtual ~DevLogSink()                          = default;
  virtual void AppendDevLog(const CString& line) = 0;
};

class DevLog
{
 public:
  static void AttachSink(DevLogSink* sink);
  static void DetachSink(DevLogSink* sink);
  static bool IsEnabled();

  static void Write(const CString& text);
  static void Writef(LPCTSTR format, ...);

 private:
  static std::atomic<DevLogSink*> sink_;
};
