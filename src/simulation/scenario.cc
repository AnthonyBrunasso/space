#pragma once

namespace simulation
{

struct Scenario {
  enum Type {
    kGameScenario,
    kEmptyScenario,
  };
  Type type;

  // Feature enabled in the scenario.
  unsigned auto_move : 1;
  unsigned ship : 1;
  unsigned asteroid : 1;
  unsigned missile : 1;
  unsigned pod : 1;
};

static Scenario kScenario;

void
InitializeScenario(bool reset_features=true)
{
  switch (kScenario.type) {
    case Scenario::kGameScenario: {
      if (reset_features) {
        kScenario.auto_move = 1;
        kScenario.ship = 1;
        kScenario.asteroid = 1;
        kScenario.missile = 1;
        kScenario.pod = 1;
      }
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
      if (reset_features) {
        kScenario.auto_move = 0;
        kScenario.ship = 0;
        kScenario.asteroid = 0;
        kScenario.missile = 0;
        kScenario.pod = 0;
      }
      ClearTilemap();
    } break;
  }
}

void
ResetScenario(bool reset_features=true)
{
  kReadCommand = 0;
  kWriteCommand = 0;
  kUsedAsteroid = 0;
  kUsedPod = 0;
  kUsedUnit = 0;
  kUsedShip = 0;
  kUsedMissile = 0;
  kUsedModule = 0;
  InitializeScenario(reset_features);
}

}  // namespace simulation
