#include <cstdio>

#include <windows.h>

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <cstdio>
#include <string.h>

#include "example/audio/wav_loader.cc"

void
ListAudioDevices(const ALCchar* devices)
{
  const ALCchar* device = devices, *next = devices + 1;
  size_t len = 0;

  printf("Device List\n");
  while (device && *device != '\0' && next && *next != '\0') {
    printf("%s\n", device);
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
}

int
main()
{
  ALCdevice* device;
  device = alcOpenDevice(nullptr);
  if (!device) printf("Could not open device.\n");
  ALboolean enumeration;
  enumeration = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE) printf("ALC_ENUMERATION_EXT not present\n");
  ListAudioDevices(alcGetString(nullptr, ALC_DEVICE_SPECIFIER));
  ALCenum error = alGetError();
  if (error != AL_NO_ERROR) {
    printf("openal error\n");
    return 1;
  }
  ALCcontext* context;
  context = alcCreateContext(device, nullptr);
  if (!alcMakeContextCurrent(context)) {
    printf("make context failure.\n");
    return 1;
  }
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
  ALuint buffer;
  alGenBuffers((ALuint)1, &buffer);
  ALsizei size, freq;
  ALenum format;
  ALvoid* data;
  ALboolean loop = AL_FALSE;

  uint8_t* bytes;
  uint32_t byte_count;
  uint32_t sample_rate;

  LoadWAV("example/audio/test.wav", &bytes, &byte_count, &sample_rate);
  printf("Loaded %u bytes sample_rate %u\n", byte_count, sample_rate);

  alBufferData(buffer, AL_FORMAT_STEREO16, bytes, byte_count, sample_rate);
  error = alGetError();
  if (error != AL_NO_ERROR) {
    printf("openal alBufferData error\n");
    return 1;
  }
  alSourcei(source, AL_BUFFER, buffer);
  alSourcePlay(source);
  ALint source_state;
  alGetSourcei(source, AL_SOURCE_STATE, &source_state);
  while (source_state == AL_PLAYING) {
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
  }
  return 0;
}
