#pragma once

enum class PFlags {
  kIgnoreGravity = 0,
  kStaticGeometry = 1,
};

struct PNode {
  v2f pos;
  v2f dims;
  u32 flags;
  // If non-zero this physics node is associated with an entity.
  u32 entity_id = 0;

  void UpdateFromEntity() {
    Entity* entity = EntityGet(entity_id);
    if (!entity) {
      LOG(ERR, "Physics systems has reference to entity %u but is non existent.", entity_id);
      return;
    }
    pos = entity->pos_;
    dims = v2f(16.f, 16.f); // TODO
  }
};

#define PSFLAG(node, flag) SBIT(node.flags, (u32)PFlags::flag);
#define PFLAGGED(node, flag) FLAGGED(node.flags, (u32)PFlags::flag)

std::vector<PNode> kPNodes;

void PAddGeom(const v2f& pos, const v2f& dims) {
  PNode pnode;
  pnode.pos = pos;
  pnode.dims = dims;
  PSFLAG(pnode, kIgnoreGravity);
  PSFLAG(pnode, kStaticGeometry);
  kPNodes.push_back(pnode);
}

void PAddEntity(u32 entity_id) {
  PNode pnode;
  pnode.entity_id = entity_id;
  kPNodes.push_back(pnode);
}

void PUpdate() {
  for (PNode& node : kPNodes) {
    if (node.entity_id) node.UpdateFromEntity();
  }
}

void PDebugRender(s32 scale) {
  for (const PNode& pnode : kPNodes) {
    Rectf scaled_rect(pnode.pos, pnode.dims);
    scaled_rect.x *= scale; 
    scaled_rect.y *= scale; 
    scaled_rect.width *= scale; 
    scaled_rect.height *= scale; 
    if (PFLAGGED(pnode, kStaticGeometry)) {
      rgg::RenderLineRectangle(scaled_rect, rgg::kRed);
    } else if (pnode.entity_id != 0) {
      rgg::RenderLineRectangle(scaled_rect, rgg::kBlue);
    }
  }
}
