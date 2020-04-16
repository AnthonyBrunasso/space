
#include <cstdint>

void
djb2_hash_more(const uint8_t* bytes, unsigned len, uint64_t *hash)
{
  for (int i = 0; i < len; ++i) {
    *hash = (*hash << 5) + *hash + bytes[i];
  }
}
