// clang-format off
#pragma once

// Players in one game
#define MAX_PLAYER 2

// Invalid offset into an array type 
// Equivalent to memset(0xff) for up to 64 bits
constexpr uint64_t kInvalidIndex = -1;

// Invalid unique identifier ('id') for assigning stable identification numbers
// Equivalent to memset(0x00) for up to 64 bits
// Allows for boolean tests: (id ? "exists: "not-exists")
constexpr uint64_t kInvalidId = 0;
