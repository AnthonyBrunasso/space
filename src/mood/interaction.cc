#pragma once

namespace mood {

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
        case 32 /* SPACE */: {
          SBIT(player->character_flags, kCharacterFireWeapon);
        } break;
        case 'j': {
          SBIT(player->ability_flags, kCharacterAbilityBoost);
          player->ability_dir = math::Normalize(particle->velocity);
        } break;
        case 'a': {
          particle->acceleration.x = -kPlayerAcceleration;
        } break;
        case 's': {
        } break;
        case 'w': {
          SBIT(player->character_flags, kCharacterJump);
        } break;
        case 'd': {
          particle->acceleration.x = kPlayerAcceleration;
        } break;
      }
    } break;
    case KEY_UP: {
      switch (event.key) {
        case 32: {
          CBIT(player->character_flags, kCharacterFireWeapon);
        } break;
        case 'j': {
          CBIT(player->ability_flags, kCharacterAbilityBoost);
        } break;
        case 'a': {
          particle->acceleration.x = 0.f;
        } break;
        case 's': {
        } break;
        case 'w': {
          CBIT(player->character_flags, kCharacterJump);
        } break;
        case 'd': {
          particle->acceleration.x = 0.f;
        } break;
      }
    } break;
    case MOUSE_DOWN: {
      imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
      if (!imui::MouseInUI(cursor, imui::kEveryoneTag)) {
        physics::Particle2d* p = physics::CreateParticle2d(
            rgg::CameraRayFromMouseToWorld(cursor, 0.f).xy(),
            v2f(5.f, 5.f));
        p->inverse_mass = 0.f;
      }
    } break;
    case MOUSE_UP: {
      imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
    } break;
    case MOUSE_WHEEL: {
      imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
    } break;
    case XBOX_CONTROLLER: {
      // TODO: Calculate controller deadzone with min magnitude.
      constexpr r32 kInputDeadzone = 4000.f;
      constexpr r32 kMaxControllerMagnitude = 32767.f;
      v2f stick(event.controller.lstick_x, event.controller.lstick_y);
      r32 magnitude = math::Length(stick);
      v2f nstick = stick / magnitude;
      r32 normalized_magnitude = 0;
      if (magnitude > kInputDeadzone) {
        if (magnitude > kMaxControllerMagnitude) {
          magnitude = kMaxControllerMagnitude;
        }
        magnitude -= kInputDeadzone;
        normalized_magnitude =
            magnitude / (kMaxControllerMagnitude - kInputDeadzone);
        if (nstick.x > 0.f) {
          particle->acceleration.x =
              kPlayerAcceleration * normalized_magnitude;
        } else if (nstick.x < 0.f) {
          particle->acceleration.x =
              -kPlayerAcceleration * normalized_magnitude;
        }
      } else {
        magnitude = 0.0;
        normalized_magnitude = 0.0;
        particle->acceleration.x = 0.f;
      }

      if (magnitude > 0.f &&
          FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_X)) {
        SBIT(player->ability_flags, kCharacterAbilityBoost);
        player->ability_dir = nstick;
      } else {
        CBIT(player->ability_flags, kCharacterAbilityBoost);
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
  };
  imui::End();
}

}
