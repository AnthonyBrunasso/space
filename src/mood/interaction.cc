#pragma once

namespace mood {

enum SelectionType {
  kSelectionNone,
  // If these numbers change change render.cc call to IsInEditMode.
  kSelectionTile = 1,
  kSelectionCollisionGeometry = 2,
};

struct Selection {
  SelectionType type = kSelectionNone;
  u32 texture_id;
  Rectf subrect;
  SPRITE_LABEL(label_name);
  bool tile_offset = false;
  physics::Particle2d* last_particle = nullptr;
};

struct Interaction {
  u32 terrain_id;
  u32 tiles_id;
  u32 wall_tiles_id;
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
  rgg::TextureInfo info;
  info.min_filter = GL_NEAREST;
  info.mag_filter = GL_NEAREST;
  kInteraction.terrain_id = rgg::LoadTextureAndSprite(
      "asset/firsttry-Sheet.tga", "asset/sheet.anim", info);
  assert(kInteraction.terrain_id);
  kInteraction.tiles_id = rgg::LoadTextureAndSprite(
      "asset/tiles.tga", "asset/tiles.anim", info);
  assert(kInteraction.tiles_id);

  kInteraction.wall_tiles_id = rgg::LoadTextureAndSprite(
      "asset/wall_tiles.tga", "asset/wall_tiles.anim", info);
  assert(kInteraction.wall_tiles_id);

}

void
ProcessPlatformEvent(const PlatformEvent& event, const v2f cursor)
{
  Character* player = Player();
  physics::Particle2d* particle = FindParticle(player);
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
          SBIT(player->character_flags, kCharacterFireWeapon);
        } break;
        case 0 /* ARROW UP */: {
          SBIT(player->character_flags, kCharacterJump);
        } break;
        case 3 /* ARROW RIGHT */: {
          particle->acceleration.x = kPlayerAcceleration;
        } break;
        case 1 /* ARROW DOWN */: {
        } break;
        case 2 /* ARROW LEFT */: {
          particle->acceleration.x = -kPlayerAcceleration;
        } break;
        case 's': {
          SBIT(player->ability_flags, kCharacterAbilityBoost);
          player->ability_dir = math::Normalize(particle->velocity);
        } break;
      }
    } break;
    case KEY_UP: {
      switch (event.key) {
        case 'a': {
          CBIT(player->character_flags, kCharacterFireWeapon);
        } break;
        case 0  /* ARROW UP */: {
          CBIT(player->character_flags, kCharacterJump);
        } break;
        case 3 /* ARROW RIGHT */: {
          particle->acceleration.x = 0.f;
        } break;
        case 1 /* ARROW DOWN */: {
        } break;
        case 2 /* ARROW LEFT */: {
          particle->acceleration.x = 0.f;
        } break;
        case 's': {
          CBIT(player->ability_flags, kCharacterAbilityBoost);
        } break;
      }
    } break;
    case MOUSE_DOWN: {
      imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
      if (kInteraction.selection.type == kSelectionNone ||
          imui::MouseInUI(imui::kEveryoneTag)) break;
      v2f posf;
      u32 texture_id;
      Rectf subrect;
      GetTileEditInfo(&posf, &texture_id, &subrect);
      if (kInteraction.selection.type == kSelectionTile) {
        RenderCreateTexture(
          texture_id,
          Rectf(posf, v2f(subrect.width, subrect.height)), 
          subrect, kInteraction.selection.label_name);
      } else if (kInteraction.selection.type == kSelectionCollisionGeometry) {
        physics::Particle2d* p = physics::CreateInfinteMassParticle2d(
           posf + v2f(subrect.width / 2.f, subrect.height / 2.f),
           v2f(subrect.width, subrect.height));
        SBIT(p->user_flags, kParticleCollider);
        if (subrect.height <= kTileHeight / 2.f) {
          SBIT(p->flags, physics::kParticleResolveCollisionStair);
        }
        kInteraction.selection.last_particle = p;
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
      v2f cdir;
      r32 cmag;
      if (__CalculateStickMovement(event.controller.lstick_x,
                                   event.controller.lstick_y, &cdir, &cmag)) {
        if (FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_X)) {
          SBIT(player->ability_flags, kCharacterAbilityBoost);
          player->ability_dir = cdir;
        }
        if (cdir.x > 0.f) {
          particle->acceleration.x = kPlayerAcceleration * cmag;
        } else if (cdir.x < 0.f) {
          particle->acceleration.x = -kPlayerAcceleration * cmag;
        }
      } else {
        particle->acceleration.x = 0.f;
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
        SBIT(player->character_flags, kCharacterJump);
      } else {
        CBIT(player->character_flags, kCharacterJump);
      }

      if (event.controller.right_trigger) {
        SBIT(player->character_flags, kCharacterFireWeapon);
      } else {
        CBIT(player->character_flags, kCharacterFireWeapon);
      }

      if (event.controller.left_trigger) {
        SBIT(player->character_flags, kCharacterFireSecondary);
      } else {
        CBIT(player->character_flags, kCharacterFireSecondary);
      }
    } break;
    default: break;
  }
}

void
EntityViewer(v2f screen)
{
  static char kUIBuffer[64];
  static b8 enable = false;
  static v2f pos(screen.x - 315, screen.y);
  imui::PaneOptions options;
  options.width = options.max_width = 315.f;
  imui::Begin("Entity Viewer", imui::kEveryoneTag, options, &pos, &enable);
  for (u32 i = 0; i < kUsedEntity; ++i) {
    Entity* e = &kEntity[i];
    imui::Indent(0);
    snprintf(kUIBuffer, sizeof(kUIBuffer), "Entity %u", e->id);
    imui::Text(kUIBuffer);
    imui::Indent(2);
    physics::Particle2d* p = physics::FindParticle2d(e->particle_id);
    snprintf(kUIBuffer, sizeof(kUIBuffer), "Particle %u", p->id);
    imui::Text(kUIBuffer);
    imui::Indent(4);
    snprintf(kUIBuffer, sizeof(kUIBuffer), "pos %.2f %.2f", p->position.x,
             p->position.y);
    imui::Text(kUIBuffer);
    snprintf(kUIBuffer, sizeof(kUIBuffer), "dims %.2f %.2f", p->dims.x,
             p->dims.y);
    imui::Text(kUIBuffer);
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
  if (!enable) kInteraction.selection.type = kSelectionNone;
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
  if (imui::Text("Render AABB", toptions).clicked) {
    kRenderAabb = !kRenderAabb;
  }
  imui::Space(imui::kHorizontal, 5.f);
  imui::Checkbox(16.f, 16.f, &kRenderAabb);
  imui::NewLine();
  imui::Space(imui::kHorizontal, 5.f);
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
  animation::Sprite* terrain_sprite = rgg::GetSprite(kInteraction.terrain_id);
  for (int i = 0; i < terrain_sprite->label_size; ++i) {
    animation::SetLabel(i, terrain_sprite);
    if (imui::Texture(32.f, 32.f, kInteraction.terrain_id,
                      animation::Rect(terrain_sprite)).clicked) {
      kInteraction.selection.type = kSelectionTile;
      kInteraction.selection.texture_id = kInteraction.terrain_id;
      kInteraction.selection.subrect = animation::Rect(terrain_sprite);
      strcpy(kInteraction.selection.label_name,
             animation::LabelName(terrain_sprite));
    }
    imui::Space(imui::kHorizontal, 5.f);
  }
  imui::NewLine();
  imui::SameLine();
  animation::Sprite* tiles_sprite = rgg::GetSprite(kInteraction.tiles_id);
  for (int i = 0; i < tiles_sprite->label_size; ++i) {
    animation::SetLabel(i, tiles_sprite);
    if (imui::Texture(32.f, 32.f, kInteraction.tiles_id,
                      animation::Rect(tiles_sprite)).clicked) {
      kInteraction.selection.type = kSelectionTile;
      kInteraction.selection.texture_id = kInteraction.tiles_id;
      kInteraction.selection.subrect = animation::Rect(tiles_sprite);
      strcpy(kInteraction.selection.label_name,
             animation::LabelName(tiles_sprite));
    }
    imui::Space(imui::kHorizontal, 5.f);
  }
  imui::NewLine();
  imui::SameLine();
  animation::Sprite* wall_tiles_sprite =
      rgg::GetSprite(kInteraction.wall_tiles_id);
  for (int i = 0; i < wall_tiles_sprite->label_size; ++i) {
    animation::SetLabel(i, wall_tiles_sprite);
    if (imui::Texture(32.f, 32.f, kInteraction.wall_tiles_id,
                      animation::Rect(wall_tiles_sprite)).clicked) {
      kInteraction.selection.type = kSelectionTile;
      kInteraction.selection.texture_id = kInteraction.wall_tiles_id;
      kInteraction.selection.subrect = animation::Rect(wall_tiles_sprite);
      strcpy(kInteraction.selection.label_name,
             animation::LabelName(wall_tiles_sprite));
    }
    imui::Space(imui::kHorizontal, 5.f);
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
  imui::Space(imui::kHorizontal, 5.f);
  imui::HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  imui::Space(imui::kVertical, 5.f);
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
  imui::End();
}

}
