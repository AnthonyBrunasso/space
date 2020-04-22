#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// http://soundfile.sapp.org/doc/WaveFormat/

int
main(int argc, char** argv)
{
#pragma pack(push, 1)
  struct WavHeader {
    uint8_t chunk_id[4];
    uint32_t chunk_size;
    uint8_t format[4];
  };
  struct WavFormat {
    uint32_t subchunk1_id;
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
  };
  struct WavData {
    uint32_t subchunk2_id;
    uint32_t subchunk2_size;
    uint8_t* data;
    uint32_t data_size;
  };
#pragma pack(pop)

  FILE* f = fopen("example/audio/test.wav", "rb");
  uint8_t* buffer;
  uint32_t file_length;

  fseek(f, 0, SEEK_END);
  file_length = ftell(f);
  rewind(f);

  buffer = (uint8_t*)malloc(file_length);
  fread(buffer, file_length, 1, f);

  printf("file_length: %u\n", file_length);

  WavHeader* header = (WavHeader*)buffer;

  // Should say RIFF
  printf("chunk_id: %.4s\n", (char*)(&header->chunk_id));
  // Think this will be the size of the file for this thing.
  printf("chunk_size: %u\n", header->chunk_size);
  // Should say WAVE
  printf("format: %.4s\n", (char*)(&header->format));

  WavFormat* format = (WavFormat*)(&buffer[sizeof(WavHeader)]);
  printf("subchunk1_id: %u\n", format->subchunk1_id);
  printf("subchunk1_size: %u\n", format->subchunk1_size);
  printf("audio_format: %u\n", format->audio_format);
  printf("num_channels: %u\n", format->num_channels);
  printf("sample_rate: %u\n", format->sample_rate);
  printf("byte_rate: %u\n", format->byte_rate);
  printf("block_align: %u\n", format->block_align);
  printf("bits_per_sample: %u\n", format->bits_per_sample);

  WavData* wav_data = (WavData*)(&buffer[sizeof(WavHeader) + sizeof(WavFormat)]);

  printf("subchunk2_id: %u\n", wav_data->subchunk2_id);
  printf("subchunk2_size: %u\n", wav_data->subchunk2_size);

  return 0;
}
