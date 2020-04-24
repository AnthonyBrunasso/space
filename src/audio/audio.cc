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
  float pitch = 1.f;
  float gain = 1.f;
  v3f position = {};
  v3f velocity = {};
  bool looping = 0.f;
};

DECLARE_ARRAY(Source, 32);
DECLARE_HASH_ARRAY(Sound, 16);

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

void
Reset()
{
  // Cleanup sources.
  for (int i = 0; i < kUsedSource;) {
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

uint32_t
LoadSound(const char* filename)
{
  if (kUsedSound >= kMaxSound) return kInvalidId;
  Sound* sound = UseSound();
  if (!LoadWAV(filename, sound)) return kInvalidId;
  return sound->id;
}

void
PlaySound(uint32_t id)
{
  Source* source = UseSource();
  alGenSources((ALuint)1, &source->alreference);
  alSourcef(source->alreference, AL_PITCH, source->pitch);
  alSourcef(source->alreference, AL_GAIN, source->gain);
  alSourcefv(source->alreference, AL_POSITION, &source->position.x);
  alSourcefv(source->alreference, AL_VELOCITY, &source->velocity.x);
  alSourcei(source->alreference, AL_LOOPING, source->looping);
  ALCenum error = alGetError();
  Sound* sound = FindSound(id);
  if (!sound) return; 
  alSourcei(source->alreference, AL_BUFFER, sound->alreference);
  alSourcePlay(source->alreference);
}

}  // namespace audio
