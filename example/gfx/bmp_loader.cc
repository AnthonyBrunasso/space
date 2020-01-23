#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

  
bool
LoadBMP(const char* file, uint8_t** image_bytes,
        uint16_t* image_width, uint16_t* image_height)
{
#pragma pack(push,1)
  struct BMPHeader {
    uint8_t signature[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
  };

  struct BMPInfoHeader {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t plane;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    uint32_t colors_used;
    uint32_t important_colors;
  };
#pragma pack(pop)
  FILE* fptr;
  uint8_t* buffer;
  uint32_t file_length;

  fptr = fopen(file, "rb");
  fseek(fptr, 0, SEEK_END);
  file_length = ftell(fptr);
  rewind(fptr);
  buffer = (uint8_t*)malloc(file_length);
  fread(buffer, file_length, 1, fptr);

  BMPHeader* header = (BMPHeader*)buffer;
  BMPInfoHeader* info_header = (BMPInfoHeader*)(&buffer[sizeof(BMPHeader)]);

#if 1
  printf("Loading img %s\n", file);
  printf("Header\n");
  printf("signature: %.*s\n", 2, (char*)header->signature);
  printf("file_size: %i\n", header->file_size);
  printf("reserved: %i\n", header->reserved);
  printf("data_offset: %i\n", header->data_offset);

  printf("Info Header\n");
  printf("size: %i\n", info_header->size);
  printf("width: %i\n", info_header->width);
  printf("height: %i\n", info_header->height);
  printf("plane: %i\n", info_header->plane);
  printf("bpp: %i\n", info_header->bpp);
  printf("compression: %i\n", info_header->compression);
  printf("image_size: %i\n", info_header->image_size);
  printf("horizontal_resolution: %i\n", info_header->horizontal_resolution);
  printf("vertical_resolution: %i\n", info_header->vertical_resolution);
  printf("colors_used: %i\n", info_header->colors_used);
  printf("important_colors: %i\n", info_header->important_colors);
#endif
  assert(memcmp(header->signature, "BM", 2) == 0);
  assert(info_header->bpp == 32);
  assert(info_header->compression == 3);

  *image_bytes = (uint8_t*)malloc(info_header->image_size);
  memcpy(*image_bytes, &buffer[sizeof(BMPHeader) + sizeof(BMPInfoHeader)],
         info_header->image_size);
#if 1
  for (int i = 0; i < 2 * (info_header->width * 4); i += 4) {
  // Top Left.
  printf("i:%i r: %i g: %i b: %i a: %i\n",
         i / 4,
         (*image_bytes)[i],
         (*image_bytes)[i + 1],
         (*image_bytes)[i + 2],
         (*image_bytes)[i + 3]);
  }
#endif
  return true;
}


int main() {
  uint8_t* image_bytes;
  uint16_t width, height;
  LoadBMP("example/gfx/test.bmp", &image_bytes, &width, &height);
  return 0;
}
