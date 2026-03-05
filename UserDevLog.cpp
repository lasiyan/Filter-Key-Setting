// clang-format off
#include "pch.h"
#include "UserDevLog.hpp"
#include <cstdarg>
// clang-format on

std::atomic<DevLogSink*> DevLog::sink_ = nullptr;

namespace {
CString BuildTimestampedLine(const CString& text)
{
  SYSTEMTIME st;
  ::GetLocalTime(&st);

  CString line;
  line.Format(_T("[%02d:%02d:%02d.%03d] %s"),
              st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
              (LPCTSTR)text);
  return line;
}
}  // namespace

void DevLog::AttachSink(DevLogSink* sink)
{
  sink_.store(sink);
}

void DevLog::DetachSink(DevLogSink* sink)
{
  auto* current = sink_.load();
  if (current == sink)
    sink_.store(nullptr);
}

bool DevLog::IsEnabled()
{
  return (sink_.load() != nullptr);
}

void DevLog::Write(const CString& text)
{
  auto* sink = sink_.load();
  if (!sink)
    return;

  sink->AppendDevLog(BuildTimestampedLine(text));
}

void DevLog::Writef(LPCTSTR format, ...)
{
  auto* sink = sink_.load();
  if (!sink)
    return;

  va_list args;
  va_start(args, format);

  CString body;
  body.FormatV(format, args);

  va_end(args);

  sink->AppendDevLog(BuildTimestampedLine(body));
}
