#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "memory/memory.cc"

namespace audio {

#define AUDIODEBUG 0

struct Sound {
  r32 length_ms = 0.f;
  u32 bitrate = 0;
  u32 frequency = 0;
  u32 channels = 0;
  u32 size = 0;
  ALenum format;
  ALuint alreference;

  b8 IsValid() const { return size != 0; }
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

b8
LoadWAV(const char* filename, Sound* sound)
{
#pragma pack(push, 1)
  struct WavHeader {
    u8 chunk_id[4];
    u32 chunk_size;
    u8 format[4];
  };
  struct WavChunk {
    u8 chunk_id[4];
    u32 chunk_size;
  };
  struct WavFmt {
    u16 audio_format;
    u16 num_channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
  };
#pragma pack(pop)

  FILE* f = fopen(filename, "rb");
  u32 file_length;

  fseek(f, 0, SEEK_END);
  file_length = ftell(f);
  rewind(f);

  u8* bytes = memory::PushBytes(file_length);

  fread(bytes, file_length, 1, f);
  fclose(f);

#if AUDIODEBUG
  printf("Read file %s size %u\n", filename, file_length);
#endif

  WavHeader* header = (WavHeader*)bytes;
  u32 read = sizeof(WavHeader);

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

  u8* sound_bytes = nullptr;
  while (read < file_length) {
    WavChunk* chunk = (WavChunk*)(&bytes[read]);
#if 0
    printf("Read chunk %.4s bytes %u\n",
           (char*)(chunk->chunk_id), chunk->chunk_size);
#endif
    read += sizeof(WavChunk);
    if (memcmp((char*)(chunk->chunk_id), "fmt", 3) == 0) {
      WavFmt* fmt = (WavFmt*)(&bytes[read]);
#if AUDIODEBUG
      printf("audio_format: %u\n", fmt->audio_format);
      printf("num_channels: %u\n", fmt->num_channels);
      printf("sample_rate: %u\n", fmt->sample_rate);
      printf("byte_rate: %u\n", fmt->byte_rate);
      printf("block_align: %u\n", fmt->block_align);
      printf("bits_per_sample: %u\n", fmt->bits_per_sample);
#endif
      sound->channels = fmt->num_channels;
      sound->bitrate = fmt->bits_per_sample;
      sound->frequency = (r32)fmt->sample_rate;
    } else if (memcmp((char*)(chunk->chunk_id), "data", 4) == 0) {
      sound->size = chunk->chunk_size;
      sound_bytes = &bytes[read];
      break;  // Once we get audio bytes quit.
    } // else - Skip unrecognized chunks.
    read += chunk->chunk_size;
  }

  assert(sound_bytes);

  sound->length_ms = (r32)sound->size / 
      (sound->channels * sound->frequency * (sound->bitrate / 8.f)) * 1000.f;

  if (sound->bitrate == 8) {
    sound->format =
        sound->channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
  } else if (sound->bitrate == 16) {
    sound->format =
        sound->channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
  } else /* Unsupported bitrate */ {
    memory::PopBytes(file_length);
    return false;
  }

  ALuint buffer;
  alGenBuffers((ALuint)1, &buffer);
  alBufferData(buffer, sound->format, sound_bytes, sound->size,
               sound->frequency);
  sound->alreference = buffer;

  ALenum error = alGetError();
  if (error != AL_NO_ERROR) {
    memory::PopBytes(file_length);
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

  memory::PopBytes(file_length);
  return true;
}

}  // namespace audio
