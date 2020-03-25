#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Usage: 
// tgatoc file.tga file.fnt src/asset/font.cc Font

struct Kerning {
  int second[20];
  int amount[20];
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
  Kerning kerning = {};
};

struct FntMetadata {
  // Just assume ascii. Key corresponds to decimal key code.
  FntMetadataRow rows[256];
  int line_height;
};

/*
static int arr[2] = {1, 2};

struct Foo {
  int x;
  int n;
  int* arr;
};

static Foo foo[3] = {
  {1, 2, &arr[0]},
};
*/

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
      assert(row->id != 0 && row->id == id);
      Kerning* kerning = &row->kerning;
      kerning->second[kerning->count] = atoi(&line[25]);
      kerning->amount[kerning->count] = atoi(&line[36]);
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

int
main(int argc, char** argv)
{
  assert(argc == 5);
  uint8_t* data;
  uint16_t image_width;
  uint16_t image_height;
  assert(LoadTGA(argv[1], &data, &image_width, &image_height));
  printf("Loaded %s size %i bytes...\n", argv[1], image_width * image_height);
  FntMetadata meta = LoadFntMetadata(argv[2]);

  FILE* fout = fopen(argv[3], "w");
  char* n = argv[4];
  fprintf(fout, "#pragma once\n\n");
  fprintf(fout, "namespace rgg {\n\n");
  fprintf(fout, R"(
struct FontMetadataRow {
  int id;
  int x;
  int y;
  int width;
  int height;
  int xoffset;
  int yoffset;
  int xadvance;
  Kerning kerning = {};
};)");

  fprintf(fout, "\n\n");
  fprintf(fout, "static int k%sWidth = %i\n\n", n, image_width);
  fprintf(fout, "static int k%sHeight = %i\n\n", n, image_height);
  fprintf(fout, "static uint8_t k%sData[%i] = {",
          n, image_width * image_height);
  for (int i = 0; i < image_width * image_height; ++i) {
    fprintf(fout, "%i,", data[i]);
  }
  fprintf(fout, "};\n\n");
  fprintf(fout, "static FontMetadataRow[256] = {\n");
  for (int i = 0; i < 256; ++i) {
    auto& m = meta.rows[i];
    fprintf(fout, "  {%i, %i, %i, %i, %i, %i, %i, %i}\n",
            m.id, m.x, m.y, m.width, m.height, m.xoffset, m.xadvance);
  }
  fprintf(fout, "}\n\n");
  fprintf(fout, "}  // namespace rgg");


  return 0;
}
