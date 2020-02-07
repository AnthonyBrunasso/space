
#pragma clang diagnostic error "-Wpadded"

#include "entity.cc"

void
TestingIteration()
{
  // Type iteration
  puts("---");
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    printf("%d %p\n", i, unit);
  }

  // Type
  // Alternative
  puts("---");
  for (char* mem = (char*)kUnit; mem < (char*)(kUnit + kUsedUnit);
       mem += sizeof(Unit)) {
    Unit* unit = (Unit*)mem;
    printf("%ld %p\n", mem - (char*)kUnit, unit);
  }

  // Type, Component
  // Alternative subcomponent
  puts("---");
  for (char* mem = (char*)kUnit + offsetof(Unit, transform);
       mem < (char*)(kUnit + kUsedUnit); mem += sizeof(Unit)) {
    Transform* c_transform = (Transform*)(mem);
    printf("%ld %p\n", mem - (char*)kUnit, c_transform);
  }

  // Type, Component, Component
  // Alternative subcomponent pair
  puts("---");
  for (char *m_transform = (char*)kUnit + offsetof(Unit, transform),
            *m_command = (char*)kUnit + offsetof(Unit, command);
       m_transform < (char*)(kUnit + kUsedUnit);
       m_transform += sizeof(Unit), m_command += sizeof(Unit)) {
    Transform* c_transform = (Transform*)(m_transform);
    Command* c_command = (Command*)(m_command);
    printf("%ld %p\n", m_transform - (char*)kUnit, c_transform);
    printf("%ld %p\n", m_command - (char*)kUnit, c_command);
  }
}

int
main()
{
  for (int i = 0; i < 5; ++i) {
    UseUnit();
  }

  TestingIteration();

  return 0;
}
