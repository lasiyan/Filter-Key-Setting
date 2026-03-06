#pragma once

#include <mmsystem.h>

#include "UserOption.hpp"
#include "resource.h"

#pragma comment(lib, "winmm.lib")

namespace UserPlaySound {

inline bool TryPlayEmbeddedWav(const int preset)
{
  const int resource_id = PRESET_IS_OFF(preset) ? IDR_NOTIFY_OFF_WAV : IDR_NOTIFY_ON_WAV;

  HMODULE module = AfxGetResourceHandle();
  HRSRC   res    = ::FindResource(module, MAKEINTRESOURCE(resource_id), _T("WAVE"));
  if (!res)
    res = ::FindResource(module, MAKEINTRESOURCE(resource_id), _T("WAV"));
  if (!res)
    return false;

  HGLOBAL loaded = ::LoadResource(module, res);
  if (!loaded)
    return false;

  const void* data = ::LockResource(loaded);
  const DWORD size = ::SizeofResource(module, res);
  if (!data || size == 0)
    return false;

  return (::PlaySound(reinterpret_cast<LPCTSTR>(data),
                      nullptr,
                      SND_MEMORY | SND_ASYNC | SND_NODEFAULT) == TRUE);
}

inline void PlayPresetAppliedSound(const int preset)
{
  if (GLOBAL_OPTION.getInteger(KEY_MUTE_SOUND, 0) != 0)
    return;

  if (!TryPlayEmbeddedWav(preset))
    ::MessageBeep(MB_ICONERROR);
}

}  // namespace UserPlaySound
