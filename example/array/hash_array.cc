// This hash map implementation is meant to give quick iteration on types
// using the same layout as common/array but also keep a secondary array
// used explicitly for hashing to indices of that array using an id.
//
// The implementation allows quick removal from EntityRegistry as well
// as quick lookup via FindType() with the overhead of a secondary array.
//
// If this implementation is to be used EntityRegistry will need to know of
// hash types so it can update the hash entries.

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cmath>

#define N 10
#define H_N (uint32_t)(N * 1.3f)
//#define H_N 10

constexpr uint32_t kInvalidEntry = 0;
constexpr uint32_t kInvalidType = 0;

static uint32_t kAutoIncrementId = 1;
static uint32_t kUsedUnit = 0;

struct Unit {
  uint32_t id;
  uint32_t hash_idx; // Back pointer to hash map for quick removals of Unit.
};

// Hash entry meant to work with existing EntityRegistry system.
// EntityRegistry deletes an element by swapping the last with the element
// being deleted for quick deletion.
//
// Example -
//
// Unit  = [1,0 3,2 2,1] : id,hash_idx
// Entry = [1,0 2,2 3,1] : id,idx
//
// Delete id 3
// Unit  = [1,0 2,1]
// Entry = [1,0 2,1 0,0] : id,idx
//
// Note now Entry contains an invalid id,idx - 3,1 combo. This entry will be
// considered free by IsEmptyEntry and reused the next time an id hashes to
// this idx.
struct HashEntry {
  uint32_t id;
  uint32_t idx;
};

void
UnsetEntry(HashEntry* entry)
{
  entry->id = 0;
  entry->idx = 0;
}

static Unit unit[N];
static HashEntry entry[H_N];

uint32_t
Hash(uint32_t i)
{
  return (i * 2654435761 % H_N); 
}

bool
IsEmptyEntry(HashEntry entry)
{
  return entry.idx == kInvalidEntry || unit[entry.idx].id != entry.id;
}

// Use linear probe.
HashEntry*
FindEmptyEntry(uint32_t hash, uint32_t* hash_idx)
{
  *hash_idx = hash;
  HashEntry* hash_entry = &entry[*hash_idx];
  while (!IsEmptyEntry(*hash_entry)) {
    printf("%i collides\n");
    *hash_idx += 1;
    *hash_idx = *hash_idx % H_N;
    hash_entry = &entry[*hash_idx];
  }
  return hash_entry;
}

Unit*
UseUnit()
{
  if (kUsedUnit >= N) return nullptr;
  Unit* u = &unit[kUsedUnit++];
  u->id = rand() % 1000;
  uint32_t hash = Hash(u->id);
  printf("Use Unit id %i\n", u->id);
  HashEntry* hash_entry = FindEmptyEntry(hash, &u->hash_idx);
  hash_entry->id = u->id;
  hash_entry->idx = kUsedUnit - 1;
  return u;
}

Unit*
FindUnit(uint32_t id)
{
  uint32_t hash = Hash(id);
  // Unit exists at hash or linear probe until it's found. Likely it's nearby.
  Unit* u = &unit[hash];
  uint32_t n = 1;
  while (u->id != id) {
    if (n >= H_N) return nullptr;
    ++hash;
    hash = hash % H_N;
    u = &unit[hash];
    ++n;
  }
  return u;
}

void
DeleteUnit(uint32_t id)
{
  Unit* u = FindUnit(id);
  Unit* lu = &unit[kUsedUnit - 1];
  // Hash idx of last unit swap
  entry[lu->hash_idx].idx = entry[u->hash_idx].idx;
  entry[lu->hash_idx].id = lu->id;
  entry[u->hash_idx] = {0, 0};
  // Swap unit in unit array.
  *u = *lu;
  kUsedUnit--;
  unit[kUsedUnit] = {0, 0};
}

void
DebugPrint()
{
  printf("Unit - ");

  for (int i = 0; i < N; ++i) {
    printf("%i/%i/%i ", i, unit[i].id, unit[i].hash_idx);
  }

  printf("\nHash - ");

  for (int i = 0; i < H_N; ++i) {
    printf("%i/%i/%i ", i, entry[i].id, entry[i].idx);
  }

  printf("\n\n");
}

void
DebugPrintUnit(uint32_t id)
{
  Unit* u = FindUnit(id);
  printf("%i/%i\n", u->id, u->hash_idx);
}

int
main()
{
  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();


  UseUnit();
  DebugPrint();


  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();

  UseUnit();
  DebugPrint();

  DeleteUnit(544);
  DebugPrint();


  //for (int i = 1; i <= N; ++i) {
  //  DebugPrintUnit(i);
  //}
  //DebugPrintUnit(930);
  //DebugPrintUnit(709);
  //DebugPrintUnit(272);
  //DebugPrintUnit(73);

  return 0;
}
