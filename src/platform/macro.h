#pragma once

#ifdef __cplusplus
#define EXTERN(x) \
  extern "C" {    \
  x;              \
  }
#else
#define EXTERN(x) x
#endif

#define MIN(x, y) (y ^ ((x ^ y) & -(x < y)))
#define MAX(x, y) (x ^ ((x ^ y) & -(x < y)))
#define CLAMP(x, min, max) MIN(MAX(x, min), max)
#define CLAMPF(x, low, high) ((x < low) ? low : (( x > high ) ? high : x))

#if _WIN32
#define ALIGNAS(n) alignas(n)
#define INLINE __forceinline
#else
// ALIGN macro taken in macos.
#define ALIGNAS(n) __attribute__((aligned(n)))
#define INLINE __attribute__((always_inline)) inline
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))
#endif

#define FLAG(x) (1<<x)

// System memory block
#define PAGE (4 * 1024)

#define DJB2_CONST 5381

