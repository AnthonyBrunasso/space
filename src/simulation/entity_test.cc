
#pragma clang diagnostic error "-Wpadded"

#include "entity.cc"

// Type Macros
#define T_FIRST(Type) (k##Type)
#define T_STEP(Type) sizeof(Type)
#define T_END(Type) (k##Type + kUsed##Type)
// Component Macros
#define C_FIRST(Type, ComponentType, component) \
  (ComponentType*)((char*)k##Type + offsetof(Type, component))
// ComponentPointer Macros
#define CP_NEXT(Type, ComponentType, ptr) \
  ptr = (ComponentType*)((char*)ptr + sizeof(Type))
#define CP_INDEX(Type, component, ptr) \
  (Type*)((char*)ptr - offsetof(Type, component)) - k##Type
// TypePointer Macros
#define TP_NEXT(Type, ptr) ptr = (Type*)((char*)ptr + T_STEP(Type))
#define TP_USED(Type, ptr) (void*)ptr < (void*)T_END(Type)
#define TP_INDEX(Type, ptr) ((Type*)ptr - k##Type)

void
TestingIteration()
{
  // Type iteration
  puts("---Unit array iter");
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    printf("%d %p\n", i, unit);
  }

  // Type
  // Alternative
  puts("---Unit macro iter");
  for (Unit* u = T_FIRST(Unit); TP_USED(Unit, u); TP_NEXT(Unit, u)) {
    printf("%ld %p\n", TP_INDEX(Unit, u), u);
  }

  // Macro checks
  Transform* component_ptr = C_FIRST(Unit, Transform, transform);
  TP_USED(Unit, component_ptr);
  CP_NEXT(Unit, Transform, component_ptr);

  // Type, Component
  // Alternative subcomponent
  puts("---Unit Transform iter");
  for (Transform* ut = C_FIRST(Unit, Transform, transform); TP_USED(Unit, ut);
       CP_NEXT(Unit, Transform, ut)) {
    printf("%ld %p\n", CP_INDEX(Unit, transform, ut), ut);
  }

  // Type, Component
  // Alternative subcomponent
  puts("---Unit Command iter");
  for (Command* ut = C_FIRST(Unit, Command, command); TP_USED(Unit, ut);
       CP_NEXT(Unit, Command, ut)) {
    printf("%ld %p\n", CP_INDEX(Unit, command, ut), ut);
  }

  // Type, Component, Component
  // Alternative subcomponent pair
  /*puts("---");
  for (char *m_transform = C_FIRST(Unit, transform),
            *m_command = C_FIRST(Unit, command);
       m_transform < T_END(Unit);
       m_transform += T_STEP(Unit), m_command += T_STEP(Unit)) {
    Transform* c_transform = (Transform*)(m_transform);
    Command* c_command = (Command*)(m_command);
    printf("%ld %p\n", m_transform - (char*)kUnit, c_transform);
    printf("%ld %p\n", m_command - (char*)kUnit, c_command);
  }*/
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
