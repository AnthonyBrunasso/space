#pragma once

namespace rgg {

void
DebugPushRect(const Rectf& rect, const v4f& color);

}

namespace mood {

void
RenderCreateEffect(Rectf rect, v4f color, u32 ttl, r32 rotate_delta);

void
CharacterInitialize()
{
}

bool
CharacterUpdate()
{
  ECS_ITR2(itr, kPhysicsComponent, kCharacterComponent);
  while (itr.Next()) {
    physics::Particle2d* particle =
        physics::FindParticle2d(itr.c.physics->particle_id);
    CharacterComponent* c = itr.c.character;
    AnimComponent* anim = ecs::GetAnimComponent(itr.e);
    // Move character.
    if (particle) {
      if (FLAGGED(c->character_flags, kCharacterMove)) {
        if (c->move_dir.x > 0.f) {
          particle->acceleration.x = (c->move_acceleration * c->move_multiplier);
        } else if (c->move_dir.x < 0.f) {
          particle->acceleration.x =
              -(c->move_acceleration * c->move_multiplier);
        }
      }

      // Instantly stop any horizontal acceleration the frame the character
      // stops moving.
      if (FLAGGED(c->prev_character_flags, kCharacterMove) &&
          !FLAGGED(c->character_flags, kCharacterMove)) {
        particle->acceleration.x = 0.f;
      }

      if (anim && FLAGGED(anim->fsm.Flags(), animation::kFSMNodeCantMove)) {
        particle->acceleration.x = 0.f;
        particle->velocity.y = 0.f;
        CBIT(particle->flags, physics::kParticleIgnoreGravity);
      }

      ProjectileWeaponComponent* projectile_weapon =
          ecs::GetProjectileWeaponComponent(itr.e);

      if (projectile_weapon &&
          FLAGGED(c->character_flags, kCharacterFireWeapon)) {
        if (util::FrameCooldownReady(&projectile_weapon->cooldown)) {
          util::FrameCooldownReset(&projectile_weapon->cooldown);
          ProjectileCreate(particle->position + v2f(0.f, 0.f), c->aim_dir,
                           c->entity_id, *projectile_weapon);
        }
      }

      MeleeWeaponComponent* melee_weapon =
          ecs::GetMeleeWeaponComponent(itr.e);

      if (melee_weapon &&
          FLAGGED(c->character_flags, kCharacterAttackMelee)) {
        Rectf attack_area = c.anim->fsm.Frame().rect();
        attack_area.x += c.p->aabb().x;
        attack_area.y += c.p->aabb().y;
        rgg::DebugPushRect(attack_area, rgg::kRed);
      }
      //if (projectile_weapon && FLAGGED(c->character_flags, kCharacterFireSecondary)) {
      //  if (util::FrameCooldownReady(&projectile_weapon->cooldown)) {
      //    util::FrameCooldownReset(&projectile_weapon->cooldown);
      //    ProjectileCreate(particle->position + v2f(0.f, 0.f), c->aim_dir,
      //                     c->entity_id, kProjectileGrenade);
      //  }
      //}
      if (FLAGGED(c->ability_flags, kCharacterAbilityBoost)) {
        if (util::FrameCooldownReady(&kPlayer.boost_cooldown)) {
          util::FrameCooldownReset(&kPlayer.boost_cooldown);
          // Boosting make player invulnerable briefly.
          util::FrameCooldownReset(&kPlayer.player_invulnerable);
          particle->velocity = {};
          particle->acceleration = {};
          particle->force += c->facing * kPlayerBoostForce;
          particle->disable_gravity_ttl = 10;
          c->trail_effect_ttl = 10;
        }
      }
      if (FLAGGED(c->character_flags, kCharacterJump)) {
        if (particle->on_ground) {
          particle->force.y += kJumpForce;
          util::FrameCooldownReset(&c->double_jump_cooldown);
          SBIT(c->character_flags, kCharacterCanDoubleJump);
        }

        if (util::FrameCooldownReady(&c->double_jump_cooldown) &&
            FLAGGED(c->character_flags, kCharacterCanDoubleJump)) {
          particle->force.y += kJumpForce;
          CBIT(c->character_flags, kCharacterCanDoubleJump);
        }
      }
    }

    if (c->trail_effect_ttl > 0) {
      if (c->trail_effect_ttl % 2 == 0) {
        Rectf effect_rect(particle->position, v2f(10.f, 10.f));
        RenderCreateEffect(
            effect_rect, v4f(.4f, 1.f, .4f, 0.3f), 25, math::Random(1.f, 10.f));
      }
      --c->trail_effect_ttl;
    }

    c->aim_dir =
        math::Normalize(math::Rotate(c->aim_dir, c->aim_rotate_delta));

    if (IsPlayer(itr.e)) {
      CharacterComponent* player = c;
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
    }

    if (c->health <= 0.f) {
      if (IsPlayer(itr.e)) {
        return true;
      } else {
        ecs::AssignDeathComponent(itr.e);
        v2f up(0.f, 1.f);
        for (int i = 0; i < 30; ++i) {
          physics::Particle2d* ep =
              physics::CreateParticle2d(particle->position + up * 1.f,
                                        v2f(kParticleWidth, kParticleHeight));
          SBIT(ep->collision_mask, kCollisionMaskCharacter);
          v2f dir = Rotate(up, math::Random(-55.f, 55.f));
          ep->force = dir * math::Random(10000.f, 30000.f);
          SBIT(ep->user_flags, kParticleBlood);
          ep->ttl = kParticleTTL;
        }
      }
    }

    if (particle->velocity.x > 0.f) {
      c->facing.x = 1.f;
      c->aim_dir.x = fabs(c->aim_dir.x);
    }
    if (particle->velocity.x < 0.f) {
      c->facing.x = -1.f;
      c->aim_dir.x = -1.f * fabs(c->aim_dir.x);
    }
    // Particles on the ground with no acceleration should stop immediately.
    // Without this stuff feels kinda floaty.
    if (particle->on_ground && particle->acceleration.x == 0.f) {
      particle->velocity.x = 0.f;
    }

    c->prev_character_flags = c->character_flags;
  }

  return false;
}

}
