
#include <cstdint>

void
djb2_hash_more(const u8* bytes, unsigned len, u64* hash)
{
  for (s32 i = 0; i < len; ++i) {
    *hash = (*hash << 5) + *hash + bytes[i];
  }
}
