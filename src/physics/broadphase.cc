#pragma once

// Implement sweep and prune against 2d particles.

struct BP2dCollision {
  Particle2d* p1;
  Particle2d* p2;
};

DECLARE_ARRAY(BP2dCollision, PHYSICS_PARTICLE_COUNT);

void
BPDeleteP2d(Particle2d* p, u32 i)
{
  // If we are deleting the head adjust the head pointer.
  if (p == kPhysics.p2d_head_x) {
    kPhysics.p2d_head_x = kPhysics.p2d_head_x->next_p2d_x;
  }

  // Get the particles next and prev pointers - update their next and prev.
  Particle2d* nextx = p->next_p2d_x;
  Particle2d* prevx = p->prev_p2d_x;

  if (nextx) {
    if (prevx) nextx->prev_p2d_x = prevx;
    else nextx->prev_p2d_x = nullptr;
  }

  if (prevx) {
    if (nextx) prevx->next_p2d_x = nextx;
    else prevx->next_p2d_x = nullptr;
  }

  // Swap the last and and current particle of the used list.
  kParticle2d[i] = kParticle2d[kUsedParticle2d - 1];
  kParticle2d[kUsedParticle2d - 1] = {};

  // This is the case that the fixed pointers actually point to an index
  // that gets swapped.
  if (nextx && nextx->prev_p2d_x &&
      nextx->prev_p2d_x == &kParticle2d[kUsedParticle2d - 1]) {
    nextx->prev_p2d_x = &kParticle2d[i];
  }

  if (prevx && prevx->next_p2d_x &&
      prevx->next_p2d_x == &kParticle2d[kUsedParticle2d - 1]) {
    prevx->next_p2d_x = &kParticle2d[i];
  }

  if (kParticle2d[i].prev_p2d_x) {
    kParticle2d[i].prev_p2d_x->next_p2d_x = &kParticle2d[i];
  }

  --kUsedParticle2d;

  if (!kUsedParticle2d) {
    kPhysics.p2d_head_x = nullptr;
  }
}

// Moves particle to between prev and next pointers.
void
__MoveP2d(Particle2d* p, Particle2d* prev, Particle2d* next)
{
  // Consider the case where head must be updated.
  // If p was head it is being moved forward. Its next is now head.
  if (p == kPhysics.p2d_head_x) {
    kPhysics.p2d_head_x = p->next_p2d_x;
  }
  // If p has the smallest x in the list it will become the new head.
  if (p->aabb().Min().x < kPhysics.p2d_head_x->aabb().Min().x) {
    kPhysics.p2d_head_x = p;
  }
  // Swap all node pointers
  if (p->prev_p2d_x) p->prev_p2d_x->next_p2d_x = p->next_p2d_x;
  if (p->next_p2d_x) p->next_p2d_x->prev_p2d_x = p->prev_p2d_x;
  if (prev) prev->next_p2d_x = p;
  if (next) next->prev_p2d_x = p;
  p->next_p2d_x = next;
  p->prev_p2d_x = prev;
}

// Updates the particle to be sorted.
void
BPUpdateP2d(Particle2d* p)
{
  // Updates particle position in sorted list if its x is now out of order.
  Particle2d* prev = p->prev_p2d_x;
  Particle2d* next = p->next_p2d_x;
  // Move the particle forward if needed.
  while (next && p->aabb().Min().x > next->aabb().Min().x) {
    prev = next;
    next = next->next_p2d_x;
  }
  if (next != p->next_p2d_x) {
    __MoveP2d(p, prev, next);
  }
  prev = p->prev_p2d_x;
  next = p->next_p2d_x;
  // Move the particle backward if needed.
  while (prev && p->aabb().Min().x < prev->aabb().Min().x) {
    next = prev;
    prev = prev->prev_p2d_x;
  }
  if (prev != p->prev_p2d_x) {
    __MoveP2d(p, prev, next);
  }
}

void
BPInitP2d(Particle2d* particle)
{
  if (!kPhysics.p2d_head_x) {
    kPhysics.p2d_head_x = particle;
    return;
  }

  if (particle->aabb().Min().x < kPhysics.p2d_head_x->aabb().Min().x) {
    kPhysics.p2d_head_x->prev_p2d_x = particle;
    particle->next_p2d_x = kPhysics.p2d_head_x;
    kPhysics.p2d_head_x = particle;
    return;
  }

  Particle2d* prev = kPhysics.p2d_head_x;
  Particle2d* next = kPhysics.p2d_head_x->next_p2d_x;
  while (prev) {
    if (!next) {
      particle->prev_p2d_x = prev;
      prev->next_p2d_x = particle;
      break;
    }
    if (particle->aabb().Min().x >= prev->aabb().Min().x &&
        particle->aabb().Min().x <= next->aabb().Min().x) {
      particle->prev_p2d_x = prev;
      particle->next_p2d_x = next;
      next->prev_p2d_x = particle;
      prev->next_p2d_x = particle;
      break;
    }
    prev = next;
    next = next->next_p2d_x;
  }
}

void
BPCalculateCollisions()
{
  kUsedBP2dCollision = 0;
  // Detect all colliding particles.
  Particle2d* p1 = kPhysics.p2d_head_x;
  Particle2d* p2 = p1->next_p2d_x;
  while (p1->next_p2d_x) {
    if (!p2) {
      p1 = p1->next_p2d_x;
      p2 = p1->next_p2d_x;
      continue;
    }
    if (math::IntersectRect(p1->aabb(), p2->aabb())) {
      BP2dCollision* collision = UseBP2dCollision();
      collision->p1 = p1;
      collision->p2 = p2;
      p2 = p2->next_p2d_x;
      continue;
    }
    // If x axis intersects move p2 forward only, this may be a non
    // intersecting y.
    if (p2->aabb().Min().x < p1->aabb().Max().x) {
      p2 = p2->next_p2d_x;
    } else {
      p1 = p1->next_p2d_x;
      p2 = p1->next_p2d_x;
    }
  }
}
