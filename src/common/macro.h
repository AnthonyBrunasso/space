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
#define CLAMPF(x, low, high) ((x < low) ? low : ((x > high) ? high : x))

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

#define FLAG(x) (1 << x)
#define FLAGGED(value, bit) ((value & FLAG(bit)) != 0)

// System memory block
#define PAGE (4 * 1024)

#define DJB2_CONST 5381

// Compile time check that can be performed in an expression
#define STATIC_ASSERT(cond) (sizeof(char [-1 * !(cond)]))

// Array bucket
#define POWEROF2(value) ((value & value-1) == 0)
#define MOD_BUCKET(value, max) (value % (max + STATIC_ASSERT(POWEROF2(max))))

// Non-branching equivalence to: (condition) ? a : b
#define TERNARY(condition, a, b) (((a)*(condition)) + ((b)*!(condition)))
