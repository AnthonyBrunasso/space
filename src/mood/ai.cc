#pragma once

#include "mood/entity.cc"
#include "physics/physics.cc"

namespace mood {

void
AICreate(v2f pos)
{
  physics::Particle2d* particle = physics::CreateParticle2d(pos, v2f(5.f, 5.f)); 
  SBIT(particle->user_flags, kPhysicsCharacter);
  Character* c = UseCharacter();
  c->particle_id = particle->id;
}

// TODO: Remove this.
void
AIDelete(u32 particle_id)
{
  for (s32 i = 0; i < kUsedCharacter; ++i) {
    Character* character = &kCharacter[i];
    //printf("DELETE %i\n", particle_id);
    if (character->particle_id == particle_id) {
      //printf("REQUEST DELETE CHARACTER: %i\n", i);
      SBIT(character->flags, kCharacterDeath);
      return;
    }
  }
}

void
AIUpdate()
{
  for (s32 i = 0; i < kUsedCharacter;) {
    Character* character = &kCharacter[i];

    if (FLAGGED(character->flags, kCharacterDeath)) {
      //printf("DO DELETE CHARACTER: %i\n", i);
      physics::DeleteParticle2d(character->particle_id);
      CompressCharacter(i);
      continue;
    }

    ++i;
  }
}

}
