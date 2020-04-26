#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace audio {

#define AUDIODEBUG 1

constexpr uint32_t kMaxSoundFileSize = 12e6;

static uint8_t kBuffer[kMaxSoundFileSize];

struct Sound {
  float length_ms = 0.f;
  uint32_t bitrate = 0;
  uint32_t frequency = 0;
  uint32_t channels = 0;
  uint32_t size = 0;
  ALenum format;
  ALuint alreference;

  bool IsValid() const { return size != 0; }
};

const char*
FormatToString(ALenum format)
{
  switch (format) {
    case AL_FORMAT_STEREO8:  return "AL_FORMAT_STEREO8";
    case AL_FORMAT_MONO8:    return "AL_FORMAT_MONO8";
    case AL_FORMAT_STEREO16: return "AL_FORMAT_STEREO16";
    case AL_FORMAT_MONO16:   return "AL_FORMAT_MONO16";
    default: return "UNKNOWN";
  }
  return "UNKNOWN";
}

bool
LoadWAV(const char* filename, Sound* sound)
{
#pragma pack(push, 1)
  struct WavHeader {
    uint8_t chunk_id[4];
    uint32_t chunk_size;
    uint8_t format[4];
  };
  struct WavChunk {
    uint8_t chunk_id[4];
    uint32_t chunk_size;
  };
  struct WavFmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
  };
#pragma pack(pop)

  FILE* f = fopen(filename, "rb");
  uint32_t file_length;

  fseek(f, 0, SEEK_END);
  file_length = ftell(f);
  assert(file_length < kMaxSoundFileSize);
  rewind(f);

  fread(kBuffer, file_length, 1, f);
  fclose(f);

#if AUDIODEBUG
  printf("Read file %s size %u\n", filename, file_length);
#endif

  WavHeader* header = (WavHeader*)kBuffer;
  uint32_t read = sizeof(WavHeader);

#if 0
  // Should say RIFF
  printf("chunk_id: %.4s\n", (char*)(&header->chunk_id));
  // Think this will be the size of the file for this thing.
  printf("chunk_size: %u\n", header->chunk_size);
  // Should say WAVE
  printf("format: %.4s\n", (char*)(&header->format));
#endif

  assert(memcmp((char*)(&header->chunk_id), "RIFF", 4) == 0);
  assert(memcmp((char*)(&header->format), "WAVE", 4) == 0);

  uint8_t* sound_bytes = nullptr;
  while (read < file_length) {
    WavChunk* chunk = (WavChunk*)(&kBuffer[read]);
#if 0
    printf("Read chunk %.4s bytes %u\n",
           (char*)(chunk->chunk_id), chunk->chunk_size);
#endif
    read += sizeof(WavChunk);
    if (memcmp((char*)(chunk->chunk_id), "fmt", 3) == 0) {
      WavFmt* fmt = (WavFmt*)(&kBuffer[read]);
#if 0
      printf("audio_format: %u\n", fmt->audio_format);
      printf("num_channels: %u\n", fmt->num_channels);
      printf("sample_rate: %u\n", fmt->sample_rate);
      printf("byte_rate: %u\n", fmt->byte_rate);
      printf("block_align: %u\n", fmt->block_align);
      printf("bits_per_sample: %u\n", fmt->bits_per_sample);
#endif
      sound->channels = fmt->num_channels;
      sound->bitrate = fmt->bits_per_sample;
      sound->frequency = (float)fmt->sample_rate;
    } else if (memcmp((char*)(chunk->chunk_id), "data", 4) == 0) {
      sound->size = chunk->chunk_size;
      sound_bytes = &kBuffer[read];
      break;  // Once we get audio bytes quit.
    } // else - Skip unrecognized chunks.
    read += chunk->chunk_size;
  }

  assert(sound_bytes);

  sound->length_ms = (float)sound->size / 
      (sound->channels * sound->frequency * (sound->bitrate / 8.f)) * 1000.f;

  if (sound->bitrate == 8) {
    sound->format =
        sound->channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
  } else if (sound->bitrate == 16) {
    sound->format =
        sound->channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
  } else /* Unsupported bitrate */ {
    return false;
  }

  ALuint buffer;
  alGenBuffers((ALuint)1, &buffer);
  alBufferData(buffer, sound->format, sound_bytes, sound->size,
               sound->frequency);
  sound->alreference = buffer;

  ALenum error = alGetError();
  if (error != AL_NO_ERROR) {
    printf("openal alBufferData error %i\n", error);
    return false;
  }

#if AUDIODEBUG
  printf("Finished reading sound file %s\n", filename);
  printf("length: %.2fms(%.2fs)\n", sound->length_ms, sound->length_ms / 1000.f);
  printf("bitrate: %u\n", sound->bitrate);
  printf("frequency: %u\n", sound->frequency);
  printf("channels: %u\n", sound->channels);
  printf("size: %u\n", sound->size);
  printf("format: %s\n", FormatToString(sound->format));
  printf("alreference: %u\n", sound->alreference);
#endif

  return true;
}

}  // namespace audio
