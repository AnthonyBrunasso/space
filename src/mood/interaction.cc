#pragma once

namespace mood {

static const u32 kUIBufferSize = 128;
static char kUIBuffer[kUIBufferSize];

enum SelectionType {
  kSelectionNone,
  // If these numbers change change render.cc call to IsInEditMode.
  kSelectionTile = 1,
  kSelectionCollisionGeometry = 2,
  kSelectionSpawner = 3,
  kSelectionObstacle = 4,
};

struct Selection {
  SelectionType type = kSelectionNone;
  u32 texture_id;
  Rectf subrect;
  SPRITE_LABEL(label_name);
  bool tile_offset = false;
  physics::Particle2d* last_particle = nullptr;
  SpawnerType spawner_type = kSpawnerNone;
  ObstacleType obstacle_type = kObstacleNone;
};

struct Interaction {
  Selection selection;
};

static Interaction kInteraction;

b8
IsInEditMode(u32* type)
{
  *type = kInteraction.selection.type;
  return kInteraction.selection.type != kSelectionNone;
}

void
GetTileEditInfo(v2f* pos, u32* texture_id, Rectf* texture_subrect)
{
  const v2f cursor = window::GetCursorPosition();
  PIXEL_ART_OBSERVER();
  v2f clickpos = rgg::CameraRayFromMouseToWorld(cursor, 0.f).xy();
  v2i cpos = WorldToTile(clickpos);
  cpos.x *= kTileWidth;
  cpos.y *= kTileHeight;
  if (kInteraction.selection.tile_offset) {
    cpos.y += kTileHeight / 2.f;
  }
  v2f posf = to_v2f(cpos);
  *pos = posf;
  *texture_id = kInteraction.selection.texture_id;
  *texture_subrect = kInteraction.selection.subrect;
}

// Turns constroller stick values into a facing direction and magnitude.
// Returns true if the stick movement is outside of a fixed value
// "dead zone"
bool
__CalculateStickMovement(
    s16 stick_x, s16 stick_y, v2f* dir, r32* normalized_magnitude)
{
  constexpr r32 kInputDeadzone = 4000.f;
  constexpr r32 kMaxControllerMagnitude = 32767.f;
  v2f stick(stick_x, stick_y);
  r32 magnitude = math::Length(stick);
  *dir = stick / magnitude;
  *normalized_magnitude = 0;
  if (magnitude > kInputDeadzone) {
    if (magnitude > kMaxControllerMagnitude) {
      magnitude = kMaxControllerMagnitude;
    }
    magnitude -= kInputDeadzone;
    *normalized_magnitude =
        magnitude / (kMaxControllerMagnitude - kInputDeadzone);
    
  } else {
    magnitude = 0.0;
    *normalized_magnitude = 0.0;
    return false;
  }
  return true;
}

void
__FileMapCallback(const char* filename)
{
  if (strcmp(filesystem::GetFilenameExtension(filename), "map") != 0) return;
  imui::TextOptions toptions;
  toptions.highlight_color = rgg::kRed;
  if (imui::Text(filename, toptions).clicked) {
    printf("Load Map %s\n", filename);
    MapReload(filename);
  }
}

void
InteractionInitialize()
{

}

void
ProcessPlatformEvent(const PlatformEvent& event, const v2f cursor)
{
  ecs::Entity* pe = Player();
  CharacterComponent* player = ecs::GetCharacterComponent(pe);
  physics::Particle2d* player_particle = nullptr;
  if (player) {
    player_particle = physics::FindParticle2d(
        ecs::GetPhysicsComponent(pe)->particle_id);
  }
  switch(event.type) {
    case KEY_DOWN: {
      switch (event.key) {
        case 27 /* ESC */: {
          exit(1);
        } break;
        case '0': {
          if (kInteraction.selection.last_particle) {
            kInteraction.selection.last_particle->dims.x += kTileHeight / 2.f;
            kInteraction.selection.last_particle->position.x +=
                kTileHeight / 4.f;
            physics::BPUpdateP2d(kInteraction.selection.last_particle);
          }
        } break;
        case '9': {
          if (kInteraction.selection.last_particle) {
            kInteraction.selection.last_particle->dims.x -= kTileHeight / 2.f;
            kInteraction.selection.last_particle->position.x -=
                kTileHeight / 4.f;
            physics::BPUpdateP2d(kInteraction.selection.last_particle);
          }
        } break;
        case 43 /* Plus */: {
          if (kInteraction.selection.last_particle) {
            kInteraction.selection.last_particle->dims.y += kTileHeight / 2.f;
            kInteraction.selection.last_particle->position.y +=
                kTileHeight / 4.f;
            physics::BPUpdateP2d(kInteraction.selection.last_particle);
          }
        } break;
        case 45 /* Minus */: {
          if (kInteraction.selection.last_particle) {
            kInteraction.selection.last_particle->dims.y -= kTileHeight / 2.f;
            kInteraction.selection.last_particle->position.y -=
                kTileHeight / 4.f;
            physics::BPUpdateP2d(kInteraction.selection.last_particle);
          }
        } break;
        case '1': {
          kInteraction.selection.tile_offset =
              !kInteraction.selection.tile_offset;
        } break;
        case 'a': {
          PlayerAttack();
        } break;
        case 0 /* ARROW UP */: {
          PlayerJump();
        } break;
        case 3 /* ARROW RIGHT */: {
          PlayerMove(v2f(kPlayerAcceleration, 0.f), 1.f);
        } break;
        case 1 /* ARROW DOWN */: {
        } break;
        case 2 /* ARROW LEFT */: {
          PlayerMove(v2f(-kPlayerAcceleration, 0.f), 1.f);
        } break;
        case 's': {
          if (player) {
            SBIT(player->ability_flags, kCharacterAbilityBoost);
          }
        } break;
      }
    } break;
    case KEY_UP: {
      switch (event.key) {
        case 'a': {
          PlayerStopAttack();
        } break;
        case 0  /* ARROW UP */: {
          PlayerStopJump();
        } break;
        case 3 /* ARROW RIGHT */: {
          PlayerStopMove();
        } break;
        case 1 /* ARROW DOWN */: {
        } break;
        case 2 /* ARROW LEFT */: {
          PlayerStopMove();
        } break;
        case 's': {
          if (player) {
            CBIT(player->ability_flags, kCharacterAbilityBoost);
          }
        } break;
      }
    } break;
    case MOUSE_DOWN: {
      if (event.button == BUTTON_LEFT) {
        imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
        v2f posf;
        u32 texture_id;
        Rectf subrect;
        GetTileEditInfo(&posf, &texture_id, &subrect);

        if (kInteraction.selection.type == kSelectionNone ||
            imui::MouseInUI(imui::kEveryoneTag)) break;
        switch (kInteraction.selection.type) {
          case kSelectionTile: {
            RenderCreateTexture(
                texture_id,
                Rectf(posf, v2f(subrect.width, subrect.height)), 
                subrect, kInteraction.selection.label_name);
          } break;
          case kSelectionCollisionGeometry: {
            physics::Particle2d* p = physics::CreateInfinteMassParticle2d(
                posf + v2f(subrect.width / 2.f, subrect.height / 2.f),
                v2f(subrect.width, subrect.height));
            SBIT(p->user_flags, kParticleCollider);
            if (subrect.height <= kTileHeight / 2.f) {
              SBIT(p->flags, physics::kParticleResolveCollisionStair);
            }
            kInteraction.selection.last_particle = p;
          } break;
          case kSelectionSpawner: {
            SpawnerCreate(posf + v2f(kTileWidth, kTileHeight) / 2.f,
                          kInteraction.selection.spawner_type);
          } break;
          case kSelectionObstacle: {
            ObstacleComponent* o = ObstacleCreate(
                posf, v2f(5.f, 5.f), kInteraction.selection.obstacle_type);
            kInteraction.selection.last_particle = ecs::GetParticle(o);
          } break;
          default: break;
        }
      } else if (event.button == BUTTON_RIGHT) {
        PIXEL_ART_OBSERVER();
        v2f clickpos = rgg::CameraRayFromMouseToWorld(cursor, 0.f).xy();
        RenderDeleteTexture(clickpos);
      } else if (event.button == BUTTON_MIDDLE) {
        PIXEL_ART_OBSERVER();
        v2f clickpos = rgg::CameraRayFromMouseToWorld(cursor, 0.f).xy();
        // Try to delete a collider.
        for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
          physics::Particle2d* p = &physics::kParticle2d[i];
          ecs::Entity* ent = ecs::FindEntity(p->entity_id);
          if (!math::PointInRect(clickpos, p->aabb())) continue;
          if (FLAGGED(p->user_flags, kParticleCollider)) {
            DeleteParticle2d(p);
            continue;
          }
          if (ent &&
              (ent->Has(kSpawnerComponent) || ent->Has(kObstacleComponent))) {
            ecs::AssignDeathComponent(ent);
          }
        }
      }
    } break;
    case MOUSE_UP: {
      imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
    } break;
    case MOUSE_WHEEL: {
      imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
      kInteraction.selection.tile_offset = !kInteraction.selection.tile_offset;
    } break;
    case XBOX_CONTROLLER: {
      if (!player) break;
      v2f cdir;
      r32 cmag;
      if (__CalculateStickMovement(event.controller.lstick_x,
                                   event.controller.lstick_y, &cdir, &cmag)) {
        if (FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_X)) {
          SBIT(player->ability_flags, kCharacterAbilityBoost);
        }
        PlayerMove(cdir, cmag);
      } else {
        PlayerStopMove();
      }

      if (!FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_X)) {
        CBIT(player->ability_flags, kCharacterAbilityBoost);
      }

      if (__CalculateStickMovement(event.controller.rstick_x,
                                   event.controller.rstick_y, &cdir, &cmag)) {
        if (cdir.y > 0.f) {
          player->aim_rotate_delta = cmag * kAimSensitivity;
        } else if (cdir.y < 0.f) {
          player->aim_rotate_delta = -cmag * kAimSensitivity;
        }
        if (player->facing.x < 0.f) player->aim_rotate_delta *= -1.f;
      } else {
        player->aim_rotate_delta = 0.f;
      }

      if (FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_A)) {
        PlayerJump();
      } else {
        PlayerStopJump();
      }

      if (event.controller.right_trigger) {
        PlayerAttack();
        //SBIT(player->character_flags, kCharacterFireWeapon);
      } else {
        PlayerStopAttack();
        //CBIT(player->character_flags, kCharacterFireWeapon);
      }

      if (event.controller.left_trigger) {
        SBIT(player->character_flags, kCharacterFireSecondary);
      } else {
        CBIT(player->character_flags, kCharacterFireSecondary);
      }

      if (FLAGGED(event.controller.controller_flags,
                  XBOX_CONTROLLER_RIGHT_SHOULDER)) {
        PlayerNextWeapon();
      }

      if (FLAGGED(event.controller.controller_flags,
                  XBOX_CONTROLLER_LEFT_SHOULDER)) {
        PlayerPrevWeapon();
      }
    } break;
    default: break;
  }
}

void
CharacterComponentUI(CharacterComponent* comp)
{
  snprintf(kUIBuffer, kUIBufferSize, "facing: %.2f %.2f",
           comp->facing.x, comp->facing.y);
  imui::Text(kUIBuffer);
  imui::SameLine();
  imui::Text("character_flags: ");
  imui::Bitfield8(comp->character_flags);
  imui::NewLine();
  imui::SameLine();
  imui::Text("ability_flags: ");
  imui::Bitfield8(comp->ability_flags);
  imui::NewLine();
  snprintf(kUIBuffer, kUIBufferSize, "trail_effect_ttl: %u",
           comp->trail_effect_ttl);
  imui::Text(kUIBuffer);
  snprintf(kUIBuffer, kUIBufferSize, "health: %.2f", comp->health);
  imui::Text(kUIBuffer);
  snprintf(kUIBuffer, kUIBufferSize, "max_health: %.2f", comp->max_health);
  imui::Text(kUIBuffer);
  snprintf(kUIBuffer, kUIBufferSize, "aim_dir: %.2f %.2f", comp->aim_dir.x,
           comp->aim_dir.y);
  imui::Text(kUIBuffer);
  snprintf(kUIBuffer, kUIBufferSize, "aim_rotate_delta: %.2f",
           comp->aim_rotate_delta);
  imui::Text(kUIBuffer);
  snprintf(kUIBuffer, kUIBufferSize, "anim_frame: %u", comp->anim_frame);
  imui::Text(kUIBuffer);
  snprintf(kUIBuffer, kUIBufferSize, "move_dir: %.2f %.2f", comp->move_dir.x,
           comp->move_dir.y);
  imui::Text(kUIBuffer);
  snprintf(kUIBuffer, kUIBufferSize, "move_multiplier: %.2f",
           comp->move_multiplier);
  imui::Text(kUIBuffer);
  snprintf(kUIBuffer, kUIBufferSize, "move_acceleration: %.2f",
           comp->move_acceleration);
  imui::Text(kUIBuffer);
}

void
EntityViewer(v2f screen)
{
  static u64 kEntityMask[ENTITY_COUNT];
  static b8 enable = false;
  static v2f pos(screen.x - 315, screen.y);
  imui::TextOptions toptions;
  toptions.highlight_color = rgg::kRed;
  imui::PaneOptions options;
  options.width = options.max_width = 315.f;
  options.max_height = 800.f;
  imui::Begin("Entity Viewer", imui::kEveryoneTag, options, &pos, &enable);
  for (u32 i = 0; i < ecs::kUsedEntity; ++i) {
    ecs::Entity* e = &ecs::kEntity[i];
    snprintf(kUIBuffer, kUIBufferSize, "Entity %u", e->id);
    if (imui::Text(kUIBuffer, toptions).highlighted) {
      physics::Particle2d* particle = ecs::GetParticle(e);
      if (particle) {
        rgg::DebugPushRect(particle->aabb(), rgg::kRed);
      }
    }
    imui::Indent(2);
    for (u32 j = 0; j < kComponentCount; ++j) {
      if (e->Has(j)) {
        if (imui::Text(TypeName(j), toptions).clicked) {
          FBIT(kEntityMask[i], j);
        }
        if (FLAGGED(kEntityMask[i], j)) {
          imui::Indent(4);
          switch ((TypeId)j) {
            case kPhysicsComponent: {
            } break;
            case kAIComponent: {
            } break;
            case kCharacterComponent: {
              CharacterComponentUI(ecs::GetCharacterComponent(e));
            } break;
            case kDeathComponent: {
            } break;
            case kProjectileComponent: {
            } break;
            case kObstacleComponent: {
            } break;
            case kSpawnerComponent: {
            } break;
            case kProjectileWeaponComponent: {
            } break;
            default: assert(!"Add component...");
          }
          imui::Indent(2);
        }
      }
    }
    imui::Indent(0);
  }
  imui::End();
}

void
MapEditor(v2f screen)
{
  static char kUIBuffer[64];
  static b8 enable = false;
  static v2f pos(screen.x - 615, screen.y);
  imui::PaneOptions options;
  options.width = options.max_width = 315.f;
  imui::Begin("Map Editor", imui::kEveryoneTag, options, &pos, &enable);
  if (enable) {
    kRenderSpawner = true;
    kRenderAabb = true;
  }
  if (!enable) {
    kInteraction.selection = {};
    kRenderSpawner = false;
    kRenderAabb = false;
  }
  imui::SameLine();
  imui::Width(160.f);
  imui::TextOptions toptions;
  toptions.highlight_color = rgg::kRed;
  if (imui::Text("Freeze game", toptions).clicked) {
    kFreezeGame = !kFreezeGame;
  }
  imui::Space(imui::kHorizontal, 5.f);
  imui::Checkbox(16.f, 16.f, &kFreezeGame);
  imui::NewLine();
  imui::SameLine();
  imui::Width(160.f);
  if (imui::Text("Render Grid", toptions).clicked) {
    kRenderGrid = !kRenderGrid;
  }
  imui::Space(imui::kHorizontal, 5.f);
  imui::Checkbox(16.f, 16.f, &kRenderGrid);
  imui::NewLine();
  imui::SameLine();
  imui::Width(160.f);
  if (imui::Text("Enemies Enabled", toptions).clicked) {
    kEnableEnemies = !kEnableEnemies;
  }
  imui::Space(imui::kHorizontal, 5.f);
  imui::Checkbox(16.f, 16.f, &kEnableEnemies);
  imui::NewLine();
  imui::HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  imui::Space(imui::kHorizontal, 5.f);
  imui::Text("Items");
  imui::SameLine();
  animation::Sprite* terrain_sprite = rgg::GetSprite(kRender.terrain_id);
  for (int i = 0; i < terrain_sprite->label_size; ++i) {
    animation::SetLabel(i, terrain_sprite);
    if (imui::Texture(32.f, 32.f, kRender.terrain_id,
                      animation::Rect(terrain_sprite)).clicked) {
      kInteraction.selection.type = kSelectionTile;
      kInteraction.selection.texture_id = kRender.terrain_id;
      kInteraction.selection.subrect = animation::Rect(terrain_sprite);
      strcpy(kInteraction.selection.label_name,
             animation::LabelName(terrain_sprite));
    }
    imui::Space(imui::kHorizontal, 5.f);
  }
  imui::NewLine();
  imui::SameLine();
  animation::Sprite* tiles_sprite = rgg::GetSprite(kRender.tiles_id);
  for (int i = 0; i < tiles_sprite->label_size; ++i) {
    animation::SetLabel(i, tiles_sprite);
    if (imui::Texture(32.f, 32.f, kRender.tiles_id,
                      animation::Rect(tiles_sprite)).clicked) {
      kInteraction.selection.type = kSelectionTile;
      kInteraction.selection.texture_id = kRender.tiles_id;
      kInteraction.selection.subrect = animation::Rect(tiles_sprite);
      strcpy(kInteraction.selection.label_name,
             animation::LabelName(tiles_sprite));
    }
    imui::Space(imui::kHorizontal, 5.f);
  }
  imui::NewLine();
  imui::Space(imui::kVertical, 5.f);
  imui::SameLine();
  animation::Sprite* wall_tiles_sprite =
      rgg::GetSprite(kRender.wall_tiles_id);
  for (int i = 0; i < wall_tiles_sprite->label_size; ++i) {
    animation::SetLabel(i, wall_tiles_sprite);
    if (imui::Texture(32.f, 32.f, kRender.wall_tiles_id,
                      animation::Rect(wall_tiles_sprite)).clicked) {
      kInteraction.selection.type = kSelectionTile;
      kInteraction.selection.texture_id = kRender.wall_tiles_id;
      kInteraction.selection.subrect = animation::Rect(wall_tiles_sprite);
      strcpy(kInteraction.selection.label_name,
             animation::LabelName(wall_tiles_sprite));
    }
    imui::Space(imui::kHorizontal, 5.f);
  }
  imui::NewLine();
  imui::SameLine();
  animation::Sprite* character_sprite = rgg::GetSprite(kRender.character_id);
  if (imui::Texture(32.f, 32.f, kRender.character_id,
                    animation::Rect(character_sprite)).clicked) {
    kInteraction.selection.type = kSelectionSpawner;
    kInteraction.selection.texture_id = kRender.character_id;
    kInteraction.selection.subrect = animation::Rect(character_sprite);
    kInteraction.selection.spawner_type = kSpawnerPlayer;
  }
  imui::Space(imui::kHorizontal, 5.f);
  animation::Sprite* snail_sprite = rgg::GetSprite(kRender.snail_id);
  if (imui::Texture(32.f, 32.f, kRender.snail_id,
                    animation::Rect(snail_sprite)).clicked) {
    kInteraction.selection.type = kSelectionSpawner;
    kInteraction.selection.texture_id = kRender.snail_id;
    kInteraction.selection.subrect = animation::Rect(snail_sprite);
    kInteraction.selection.spawner_type = kSpawnerSnail;
  }
  imui::Space(imui::kHorizontal, 5.f);
  if (imui::ButtonCircle(16.f, rgg::kRed).clicked) {
    kInteraction.selection.type = kSelectionSpawner;
    kInteraction.selection.spawner_type = kSpawnerFlying;
  }
  imui::NewLine();
  imui::Space(imui::kVertical, 5.f);
  if (imui::Button(32.f, 32.f, rgg::kRed).clicked) {
    kInteraction.selection.type = kSelectionCollisionGeometry;
    kInteraction.selection.subrect = Rectf(0.f, 0.f, 16.f, 16.f);
  }
  imui::Space(imui::kVertical, 5.f);
  if (imui::Button(32.f, 16.f, rgg::kGreen).clicked) {
    kInteraction.selection.type = kSelectionCollisionGeometry;
    kInteraction.selection.subrect = Rectf(0.f, 0.f, 16.f, 9.f);
  }
  imui::Space(imui::kVertical, 5.f);
  if (imui::Button(32.f, 16.f, rgg::kBlue).clicked) {
    kInteraction.selection.type = kSelectionCollisionGeometry;
    kInteraction.selection.subrect = Rectf(0.f, 0.f, 16.f, 7.f);
  }
  imui::Space(imui::kVertical, 5.f);
  imui::HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  imui::Text("Obstacles");
  imui::Indent(2);
  if (imui::Text("Boost", toptions).clicked) {
    kInteraction.selection.type = kSelectionObstacle;
    kInteraction.selection.obstacle_type = kObstacleBoost;
  }
  imui::Indent(0);
  imui::HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  imui::Space(imui::kVertical, 5.f);
  if (imui::Text("New Map", toptions).clicked) {
    MapGenerateUniqueName();
    MapCreateEmpty(kCurrentMapName);
    kReloadGame = true;
  }
  if (imui::Text("Save Map", toptions).clicked) {
    MapSave("asset/test.map");
  }
  static b8 show_load_maps = false;
  if (imui::Text("Load Map", toptions).clicked) {
    show_load_maps = !show_load_maps;
  }
  if (show_load_maps) {
    imui::Indent(1);
    filesystem::WalkDirectory("asset/", __FileMapCallback);
    imui::Indent(0);
  }
  if (imui::Text("Clear Map", toptions).clicked) {
    kReloadGame = true;
    strcpy(kCurrentMapName, "");
  }
  imui::End();
}

}
