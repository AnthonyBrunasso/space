#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

namespace audio {

struct Audio {
  ALCdevice* device = nullptr;
  ALCcontext* context = nullptr;
};

static Audio kAudio;

void
ListAudioDevices(const ALCchar* devices)
{
  const ALCchar* device = devices, *next = devices + 1;
  size_t len = 0;

  printf("OpenAL Device List\n");
  while (device && *device != '\0' && next && *next != '\0') {
    printf("%s\n", device);
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
}

bool
Initialize()
{
  kAudio.device = alcOpenDevice(nullptr);
  if (!kAudio.device) {
    printf("Unable to open audio device.\n");
    return false;
  }

  ALboolean enumeration = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE) {
    printf("ALC_ENUMERATION_EXT not present\n");
    return false;
  }

  ListAudioDevices(alcGetString(nullptr, ALC_DEVICE_SPECIFIER));
  ALCenum error = alGetError();
  if (error != AL_NO_ERROR) {
    printf("OpenAL error: %i\n", error);
    return false;
  }

  kAudio.context = alcCreateContext(kAudio.device, nullptr);
  if (!alcMakeContextCurrent(kAudio.context)) {
    printf("alcMakeContextCurrent error\n");
    return false;
  }

  return true;
}

}  // namespace audio
