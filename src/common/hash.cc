
#include <cstdint>

void
djb2_hash_more(const u8* bytes, u32 len, u64* hash)
{
  for (u32 i = 0; i < len; ++i) {
    *hash = (*hash << 5) + *hash + bytes[i];
  }
}
