#pragma once

namespace mood {

enum SelectionType {
  kSelectionNone,
  kSelectionTile,
  kSelectionCollisionGeometry,
};

struct Selection {
  SelectionType type = kSelectionNone;
  u32 texture_id;
  Rectf subrect;
  SPRITE_LABEL(label_name);
};

struct Interaction {
  u32 terrain_id;
  Selection selection;
};

static Interaction kInteraction;

static b8 kFreezeGame = false;

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
      PIXEL_ART_OBSERVER();
      v2f clickpos = rgg::CameraRayFromMouseToWorld(cursor, 0.f).xy();
      v2i pos = WorldToTile(clickpos);
      pos.x *= kTileWidth;
      pos.y *= kTileHeight;
      v2f posf = to_v2f(pos);
      if (kInteraction.selection.type == kSelectionTile) {
        RenderCreateTexture(
          kInteraction.selection.texture_id,
          Rectf(posf, v2f(kInteraction.selection.subrect.width,
                          kInteraction.selection.subrect.height)), 
          kInteraction.selection.subrect, kInteraction.selection.label_name);
      } else if (kInteraction.selection.type == kSelectionCollisionGeometry) {
        SBIT(physics::CreateInfinteMassParticle2d(
           posf + v2f(kTileWidth / 2.f, kTileHeight / 2.f),
           v2f(kTileWidth, kTileHeight))->user_flags, kParticleCollider);
      }
    } break;
    case MOUSE_UP: {
      imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
    } break;
    case MOUSE_WHEEL: {
      imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
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
        player->aim_dir = math::Lerp(player->aim_dir, cdir, 0.05f);
        if (player->facing.x > 0.f) {
          r32 angle = atan2(player->aim_dir.y, player->aim_dir.x) * 180.f / PI;
          if (angle > kAimAngleClamp || angle < -kAimAngleClamp) {
            angle = CLAMPF(angle, -kAimAngleClamp, kAimAngleClamp);
            player->aim_dir = math::Rotate(v2f(1.f, 0.f), angle);
          } 
        } else if (player->facing.x < 0.f) {
          r32 angle = math::Atan2(player->aim_dir.y, player->aim_dir.x);
          r32 low = 180.f - kAimAngleClamp;
          r32 high = 180.f + kAimAngleClamp;
          if (angle > kAimAngleClamp || angle < -kAimAngleClamp) {
            angle = CLAMPF(angle, low, high);
            player->aim_dir = math::Rotate(v2f(1.f, 0.f), angle);
          }
        }
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
  imui::HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  imui::Space(imui::kHorizontal, 5.f);
  imui::Text("Items");
  imui::SameLine();
  animation::Sprite* terrain_sprite = rgg::GetSprite(kInteraction.terrain_id);
  animation::SetLabel("grass_left", terrain_sprite);
  if (imui::Texture(32.f, 32.f, kInteraction.terrain_id,
                    animation::Rect(terrain_sprite)).clicked) {
    kInteraction.selection.type = kSelectionTile;
    kInteraction.selection.texture_id = kInteraction.terrain_id;
    kInteraction.selection.subrect = animation::Rect(terrain_sprite);
    strcpy(kInteraction.selection.label_name, "grass_left");
  }
  imui::Space(imui::kHorizontal, 5.f);
  animation::SetLabel("grass_middle", terrain_sprite);
  if (imui::Texture(32.f, 32.f, kInteraction.terrain_id,
                    animation::Rect(terrain_sprite)).clicked) {
    kInteraction.selection.type = kSelectionTile;
    kInteraction.selection.texture_id = kInteraction.terrain_id;
    kInteraction.selection.subrect = animation::Rect(terrain_sprite);
    strcpy(kInteraction.selection.label_name, "grass_middle");
  }

  imui::Space(imui::kHorizontal, 5.f);
  animation::SetLabel("grass_right", terrain_sprite);
  if (imui::Texture(32.f, 32.f, kInteraction.terrain_id,
                    animation::Rect(terrain_sprite)).clicked) {
    kInteraction.selection.type = kSelectionTile;
    kInteraction.selection.texture_id = kInteraction.terrain_id;
    kInteraction.selection.subrect = animation::Rect(terrain_sprite);
    strcpy(kInteraction.selection.label_name, "grass_right");
  }
  imui::NewLine();
  imui::Space(imui::kVertical, 5.f);
  animation::SetLabel("brick", terrain_sprite);
  if (imui::Texture(32.f, 32.f, kInteraction.terrain_id,
                    animation::Rect(terrain_sprite)).clicked) {
    kInteraction.selection.type = kSelectionTile;
    kInteraction.selection.texture_id = kInteraction.terrain_id;
    kInteraction.selection.subrect = animation::Rect(terrain_sprite);
    strcpy(kInteraction.selection.label_name, "brick");
  }
  imui::NewLine();
  imui::Space(imui::kVertical, 5.f);
  if (imui::Button(32.f, 32.f, rgg::kRed).clicked) {
    kInteraction.selection.type = kSelectionCollisionGeometry;
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
