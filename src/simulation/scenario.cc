#pragma once

namespace simulation
{

struct Scenario {
  enum Type {
    kGameScenario,
    kEmptyScenario,
    kAIScenario,
  };
  Type type;
};

static Scenario kScenario;

void
InitializeScenario()
{
  switch (kScenario.type) {
    case Scenario::kGameScenario: {
      v3f pos[] = {v3f(300.f, 300.f, 0.f), v3f(100.f, 130.f, 0),
                   v3f(300.f, 400.f, 0), v3f(650.f, 460.f, 0),
                   v3f(100.f, 577.f, 0.f)};
      const v3f scale = v3f(0.25f, 0.25f, 0.f);
      uint8_t attrib[CREWA_MAX] = {11, 10, 11, 10};
      for (int i = 0; i < ARRAY_LENGTH(pos); ++i) {
        Unit* unit = UseUnit();
        unit->transform.position = pos[i];
        unit->transform.scale = scale;
        memcpy(unit->acurrent, attrib, sizeof(attrib));
        // Everybody is unique!
        unit->kind = i + 1;
      }
      kUnit[4].spacesuit = 1;

      UseShip();
      kShip[0].running = true;
      kShip[0].level = 1;

      InitializeTilemap();
    } break;
    case Scenario::kEmptyScenario: {
      ClearTilemap();
    } break;
    case Scenario::kAIScenario: {
      ClearTilemap();
    } break;
  }
}

void
ResetScenario()
{
  kReadCommand = 0;
  kWriteCommand = 0;
  kUsedAsteroid = 0;
  kUsedPod = 0;
  kUsedUnit = 0;
  kUsedShip = 0;
  kUsedMissile = 0;
  kUsedModule = 0;
  InitializeScenario();
}

}  // namespace simulation
