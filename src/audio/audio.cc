#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include "common/common.cc"
#include "math/vec.h"
#include "sound.cc"

namespace audio {

struct Audio {
  ALCdevice* device = nullptr;
  ALCcontext* context = nullptr;
};

static Audio kAudio;

struct Source {
  ALuint alreference;
  r32 pitch = 1.f;
  r32 gain = 1.f;
  v3f position = {};
  v3f velocity = {};
  b8 looping = false;
};

DECLARE_ARRAY(Source, 32);

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

b8
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

void
Cleanup()
{
  // Cleanup sources.
  for (s32 i = 0; i < kUsedSource;) {
    Source* source = &kSource[i];
    if (source) {
      ALint source_state;
      alGetSourcei(source->alreference, AL_SOURCE_STATE, &source_state);
      if (source_state == AL_STOPPED) {
        alDeleteSources(1, &source->alreference);
        CompressSource(i);
        continue;
      }
    }
    ++i;
  }
}

void
SetListener(const v3f& position, const v3f& velocity, const v3f& facing,
            const v3f& up)
{
  ALfloat listener_orientation[6] = {
    facing.x, facing.y, facing.z, up.x, up.y, up.z};
  alListener3f(AL_POSITION, position.x, position.y, position.z);
  alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
  alListenerfv(AL_ORIENTATION, listener_orientation);
}

void
PlaySound(const Sound& sound, const Source& init_source)
{
  if (!sound.IsValid()) return;
  Source* source = UseSource();
  *source = init_source;
  alGenSources((ALuint)1, &source->alreference);
  alSourcef(source->alreference, AL_PITCH, source->pitch);
  alSourcef(source->alreference, AL_GAIN, source->gain);
  alSourcefv(source->alreference, AL_POSITION, &source->position.x);
  alSourcefv(source->alreference, AL_VELOCITY, &source->velocity.x);
  alSourcei(source->alreference, AL_LOOPING, source->looping);
  alSourcei(source->alreference, AL_BUFFER, sound.alreference);
  alSourcePlay(source->alreference);
}

}  // namespace audio
