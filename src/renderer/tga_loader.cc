#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct Kerning {
  int second[16];
  int amount[16];
  int count = 0;
};

// http://tfc.duke.free.fr/coding/tga_specs.pdf
struct FntMetadataRow {
  int id;
  int x;
  int y;
  int width;
  int height;
  int xoffset;
  int yoffset;
  int xadvance;
  Kerning kerning;
};

struct FntMetadata {
  // Just assume ascii. Key corresponds to decimal key code.
  FntMetadataRow rows[256];
  int line_height;
};

FntMetadata
LoadFntMetadata(const char* file)
{
  FILE* fptr = fopen(file, "r");
  char line[128];
  int row_count = 0;
  int i = 0;
  FntMetadata metadata = {};
  while (fgets(line, sizeof(line), fptr)) {
    ++i;
    if (i == 3) {
      metadata.line_height = atoi(&line[18]);
      continue;
    }
    // First 5 lines are header.
    if (i <= 5) continue;
    if (i == 101) continue;
    // TODO: These offsets looks like they are pretty predictable but
    // maybe they're not?? This is quick and works for now so just keep this.

    // Kernings.
    if (i > 100) {
      //printf("%s\n", line);
      int id = atoi(&line[14]);
      FntMetadataRow* row = &metadata.rows[id];
      Kerning* kerning = &row->kerning;
      kerning->second[kerning->count] = atoi(&line[25]);
      kerning->amount[kerning->count] = atoi(&line[36]);
#if 0
      printf("first: %i second: %i amount: %i\n",
             id, kerning->second[kerning->count],
             kerning->amount[kerning->count]);
#endif
      ++kerning->count;
      continue;
    }
    // Character metadata.
    int id = atoi(&line[8]);
    FntMetadataRow* row = &metadata.rows[id];
    row->id = atoi(&line[8]);
    row->x = atoi(&line[15]);
    row->y = atoi(&line[23]);
    row->width = atoi(&line[35]);
    row->height = atoi(&line[48]);
    row->xoffset = atoi(&line[62]);
    row->yoffset = atoi(&line[76]);
    row->xadvance = atoi(&line[91]);
  }
  fclose(fptr);
  return metadata;
}

bool
LoadTGA(const char* file, uint8_t** image_bytes, uint16_t* image_width,
        uint16_t* image_height)
{
#pragma pack(push, 1)
  struct TgaImageSpec {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t image_width;
    uint16_t image_height;
    uint8_t pixel_depth;
    uint8_t image_descriptor;
  };
  struct TgaHeader {
    uint8_t id_length;
    uint8_t color_map_type;
    uint8_t image_type;
    uint8_t color_map_spec[5];
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

  // First load the header.
  TgaHeader* header = (TgaHeader*)buffer;
  // Just don't even support colors.
  assert(header->id_length == 0);
  assert(header->color_map_type == 0);
  // Get the image_spec. This has overall image details.
  TgaImageSpec* image_spec = (TgaImageSpec*)(&buffer[sizeof(TgaHeader)]);
  // Only support 8-bit pixel depths.
  assert(image_spec->pixel_depth == 8);

#if 0
  printf("TGA file: %s header\n", file);
  printf("header->id_length: %i\n", header->id_length);
  printf("header->color_map_type: %i\n", header->color_map_type);
  printf("header->image_type: %i\n", header->image_type);

  printf("TGA file: %s Image Spec\n", file);
  printf("image_spec->x_origin: %i\n", image_spec->x_origin);
  printf("image_spec->y_origin: %i\n", image_spec->y_origin);
  printf("image_spec->image_width: %i\n", image_spec->image_width);
  printf("image_spec->image_height: %i\n", image_spec->image_height);
  printf("image_spec->pixel_depth: %i\n", image_spec->pixel_depth);
  printf("image_spec->image_descriptor: %i\n", image_spec->image_descriptor);
#endif

  // Image bytes sz
  uint32_t image_bytes_size =
      image_spec->image_width * image_spec->image_height;
  *image_width = image_spec->image_width;
  *image_height = image_spec->image_height;
  *image_bytes = (uint8_t*)malloc(image_bytes_size);
  memcpy(*image_bytes, &buffer[sizeof(TgaHeader) + sizeof(TgaImageSpec)],
         image_bytes_size);
  // Free buffer used to read in file.
  free(buffer);
  fclose(fptr);
  return true;
}
