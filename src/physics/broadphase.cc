#pragma once

// Implement sweep and prune against 2d particles.

struct BP2dCollision {
  Particle2d* p1;
  Particle2d* p2;
  Rectf intersection;
};

DECLARE_ARRAY(BP2dCollision, PHYSICS_PARTICLE_COUNT);

u32
BPGetNextId(u32 id)
{
  if (!id) return kInvalidId;
  Particle2d* p = FindParticle2d(id);
  if (!p) return kInvalidId;
  return p->next_p2d_x;
}

void
BPDeleteP2d(Particle2d* p, u32 i)
{
  // If we are deleting the head adjust the head pointer.
  if (p->id == kPhysics.p2d_head_x) {
    kPhysics.p2d_head_x = BPGetNextId(kPhysics.p2d_head_x);
  }

  // Get the particles next and prev pointers - update their next and prev.
  Particle2d* nextx = FindParticle2d(p->next_p2d_x);
  Particle2d* prevx = FindParticle2d(p->prev_p2d_x);

  if (nextx) {
    if (prevx) nextx->prev_p2d_x = prevx->id;
    else nextx->prev_p2d_x = kInvalidId;
  }

  if (prevx) {
    if (nextx) prevx->next_p2d_x = nextx->id;
    else prevx->next_p2d_x = kInvalidId;
  }

  // Swap the last and and current particle of the used list.
  SwapParticle2d(p->id, kParticle2d[kUsedParticle2d - 1].id);
  ClearParticle2d(kParticle2d[kUsedParticle2d - 1].id);

  if (!kUsedParticle2d) {
    kPhysics.p2d_head_x = kInvalidId;
  }
}

// Moves particle to between prev and next pointers.
void
__MoveP2d(Particle2d* p, Particle2d* prev, Particle2d* next)
{
  // Consider the case where head must be updated.
  // If p was head it is being moved forward. Its next is now head.
  if (p->id == kPhysics.p2d_head_x) {
    kPhysics.p2d_head_x = BPGetNextId(p->id);
  }
  // If p has the smallest x in the list it will become the new head.
  Particle2d* head = FindParticle2d(kPhysics.p2d_head_x);
  if (!head) return;
  if (p->aabb().Min().x < head->aabb().Min().x) {
    kPhysics.p2d_head_x = p->id;
  }
  // Swap all node pointers
  Particle2d* particle_prev = FindParticle2d(p->prev_p2d_x);
  if (particle_prev) particle_prev->next_p2d_x = p->next_p2d_x;
  Particle2d* particle_next = FindParticle2d(p->next_p2d_x);
  if (particle_next) particle_next->prev_p2d_x = p->prev_p2d_x;
  if (prev) prev->next_p2d_x = p->id;
  if (next) next->prev_p2d_x = p->id;
  if (next) p->next_p2d_x = next->id;
  if (prev) p->prev_p2d_x = prev->id;
}

// Updates the particle to be sorted.
void
BPUpdateP2d(Particle2d* p)
{
  // TODO: There is a weird bug with this where if there are only two 
  // particles the next and prev pointers do not properly update. It doesn't
  // cause an issue in functionality that I can discern but leaving this
  // comment for documentation if it ever does.

  // Updates particle position in sorted list if its x is now out of order.
  Particle2d* prev = FindParticle2d(p->prev_p2d_x);
  Particle2d* next = FindParticle2d(p->next_p2d_x);
  // Move the particle forward if needed.
  while (next && p->aabb().Min().x > next->aabb().Min().x) {
    prev = next;
    next = FindParticle2d(next->next_p2d_x);
  }
  if (next && next->id != p->next_p2d_x) {
    __MoveP2d(p, prev, next);
  }
  prev = FindParticle2d(p->prev_p2d_x);
  next = FindParticle2d(p->next_p2d_x);
  // Move the particle backward if needed.
  while (prev && p->aabb().Min().x < prev->aabb().Min().x) {
    next = prev;
    prev = FindParticle2d(prev->prev_p2d_x);
  }
  if (prev && prev->id != p->prev_p2d_x) {
    __MoveP2d(p, prev, next);
  }
}

void
BPInitP2d(Particle2d* particle)
{
  if (!kPhysics.p2d_head_x) {
    kPhysics.p2d_head_x = particle->id;
    return;
  }

  Particle2d* head = FindParticle2d(kPhysics.p2d_head_x);
  if (!head) return;
  if (particle->aabb().Min().x < head->aabb().Min().x) {
    head->prev_p2d_x = particle->id;
    particle->next_p2d_x = head->id;
    kPhysics.p2d_head_x = particle->id;
    return;
  }

  Particle2d* prev = head;
  Particle2d* next = FindParticle2d(head->next_p2d_x);
  while (prev) {
    if (!next) {
      particle->prev_p2d_x = prev->id;
      prev->next_p2d_x = particle->id;
      break;
    }
    if (particle->aabb().Min().x >= prev->aabb().Min().x &&
        particle->aabb().Min().x <= next->aabb().Min().x) {
      particle->prev_p2d_x = prev->id;
      particle->next_p2d_x = next->id;
      next->prev_p2d_x = particle->id;
      prev->next_p2d_x = particle->id;
      break;
    }
    prev = next;
    next = FindParticle2d(next->next_p2d_x);
  }
}

void
BPCalculateCollisions()
{
  kUsedBP2dCollision = 0;
  // Detect all colliding particles.
  Particle2d* p1 = FindParticle2d(kPhysics.p2d_head_x);
  Particle2d* p2 = FindParticle2d(p1->next_p2d_x);
  while (p1->next_p2d_x) {
    if (!p2) {
      p1 = FindParticle2d(p1->next_p2d_x);
      p2 = FindParticle2d(p1->next_p2d_x);
      continue;
    }
    Rectf intersection;
    if (math::IntersectRect(p1->aabb(), p2->aabb(), &intersection)) {
      BP2dCollision* collision = UseBP2dCollision();
      collision->p1 = p1;
      collision->p2 = p2;
      collision->intersection = intersection;
      p2 = FindParticle2d(p2->next_p2d_x);
      continue;
    }
    // If x axis intersects move p2 forward only, this may be a non
    // intersecting y.
    if (p2->aabb().Min().x < p1->aabb().Max().x) {
      p2 = FindParticle2d(p2->next_p2d_x);
    } else {
      p1 = FindParticle2d(p1->next_p2d_x);
      p2 = FindParticle2d(p1->next_p2d_x);
    }
  }
}
