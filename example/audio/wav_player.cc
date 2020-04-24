#include <cstdio>

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <cstdio>
#include <string.h>

#include "audio/audio.cc"
#include "audio/sound.cc"

int
main()
{
  audio::Initialize();

  ALfloat listener_orientation[6] = {0.f, 0.f, 1.f, 0.f, 1.f, 0.f};
  alListener3f(AL_POSITION, 0, 0, 1.f);
  alListener3f(AL_VELOCITY, 0, 0, 0);
  alListenerfv(AL_ORIENTATION, listener_orientation);
  ALuint source;
  alGenSources((ALuint)1, &source);
  alSourcef(source, AL_PITCH, 1);
  alSourcef(source, AL_GAIN, 1);
  alSource3f(source, AL_POSITION, 0, 0, 0);
  alSource3f(source, AL_VELOCITY, 0, 0, 0);
  alSourcei(source, AL_LOOPING, AL_FALSE);
  ALsizei size, freq;
  ALenum format;
  ALboolean loop = AL_FALSE;

  audio::Sound sound;
  if (!audio::LoadWAV("example/audio/test.wav", &sound)) {
    printf("Failed to load sound\n");
    return 1;
  }

  ALCenum error = alGetError();
  if (error != AL_NO_ERROR) {
    printf("openal alBufferData error\n");
    return 1;
  }
  alSourcei(source, AL_BUFFER, sound.alreference);
  alSourcePlay(source);
  ALint source_state;
  alGetSourcei(source, AL_SOURCE_STATE, &source_state);
  while (source_state == AL_PLAYING) {
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
  }
  return 0;
}
