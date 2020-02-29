// TODO (AN): Reconsider file structure.
// This file provides access to hardware intrinsics.
// The platform layer provides software alternatives which we don't care about.
// ASM would allow simplification and relocation to common/
#pragma once

#include <cstdint>

#ifdef _WIN32
#include <intrin.h>
#else
// The recommended include is heavy: compile time
// <x86intrin.h>
#define __X86INTRIN_H
#include <bmiintrin.h>
#include <immintrin.h>
#include <lzcntintrin.h>
#include <popcntintrin.h>
#endif

#ifdef _WIN32
// TODO: enforce target arch?
#define TARGET(x)
#else
// target cpu requirement
#define TARGET(x) __attribute__((target(x)))
#endif

#define RDRND_RETRY_LIMIT 3

// Reset lowest set bit to 0
inline uint64_t TARGET("bmi") BLSR(uint64_t f)
{
  return _blsr_u64(f);
}

// Right 0s: Trailing zero count
inline uint64_t TARGET("bmi") TZCNT(uint64_t f)
{
  return _tzcnt_u64(f);
}

// Left 0s: Leading zero count
inline uint64_t TARGET("lzcnt") LZCNT(uint64_t f)
{
  return _lzcnt_u64(f);
}

// 1s Population count
inline uint64_t TARGET("popcnt") POPCNT(uint64_t f)
{
  return _mm_popcnt_u64(f);
}

// Bits in 'a' are cleared in 'f'
// Logically: ~a & f
inline uint64_t TARGET("bmi") ANDN(uint64_t a, uint64_t f)
{
  return _andn_u64(a, f);
}

inline int TARGET("rdrnd") RDRND(unsigned long long *p)
{
  for (int i = 0; i < RDRND_RETRY_LIMIT; ++i)
    if (_rdrand64_step(p)) return 1;
  return 0;
}
