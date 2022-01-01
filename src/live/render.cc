namespace live {

void
Render()
{
  live::Grid* grid = live::GridGet(1);
  if (kRenderGridFill) {
    for (live::Cell& cell : grid->storage) {
      if (!cell.entity_ids.empty()) {
        rgg::RenderRectangle(cell.rect(), v4f(.2f, .2f, .2f, .8f));
      }
    }
  }

  rgg::Texture* terrain_texture = rgg::GetTexture(live::kAssets.terrain_texture_id);
  rgg::Texture* character_texture = rgg::GetTexture(live::kAssets.character_texture_id);
  Rectf sbounds = live::ScreenBounds();

  {
    assert(terrain_texture);
    for (const live::Cell& cell : grid->storage) {
      if (!math::IsContainedInRect(cell.rect(), sbounds) && !math::IntersectRect(cell.rect(), sbounds))
        continue;
      if (cell.pos == v2i(0, 0)) {
        live::AssetTerrainRender(terrain_texture, live::GridPosFromXY(cell.pos), live::TerrainAsset::kDirtBottomLeft);
      } else if (cell.pos.y == 0) {
        live::AssetTerrainRender(terrain_texture, live::GridPosFromXY(cell.pos), live::TerrainAsset::kDirtBottomMiddle);
      } else if (cell.pos.x == 0) {
        live::AssetTerrainRender(terrain_texture, live::GridPosFromXY(cell.pos), live::TerrainAsset::kDirtMiddleLeft);
      } else {
        live::AssetTerrainRender(terrain_texture, live::GridPosFromXY(cell.pos), live::TerrainAsset::kDirtMiddleMiddle);
      }
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kZoneComponent);
    while (itr.Next()) {
      PhysicsComponent* physics = itr.c.physics;
      rgg::RenderRectangle(physics->rect(), v4f(0.1f, 0.1f, 0.4f, 0.8f));
    }
  }
  
  {
    ECS_ITR3(itr, kPhysicsComponent, kHarvestComponent, kResourceComponent);
    while (itr.Next()) {
      PhysicsComponent* physics = itr.c.physics;
      ResourceComponent* resource = itr.c.resource;
      if (!math::IsContainedInRect(physics->rect(), sbounds) && !math::IntersectRect(physics->rect(), sbounds))
        continue;

      switch (resource->resource_type) {
        case kLumber:
          //rgg::RenderRectangle(physics->rect(), v4f(0.f, 1.f, 0.f, 1.f));
          live::AssetTerrainRender(terrain_texture, physics->pos, live::TerrainAsset::kTree);
          break;
        case kStone:
          rgg::RenderRectangle(physics->rect(), v4f(.5f, .5f, .5f, 1.f));
          break;
        case kResourceTypeCount:
        default:
          assert(!"Can't render resource type");
      }
    }
  }

  glDisable(GL_BLEND);

  {
    ECS_ITR2(itr, kPhysicsComponent, kBuildComponent);
    while (itr.Next()) {
      PhysicsComponent* physics = itr.c.physics;
      BuildComponent* build = itr.c.build;
      switch (build->structure_type) {
        case kWall:
          rgg::RenderLineRectangle(physics->rect(), v4f(1.f, 1.f, 1.f, 1.f));
          break;
        case kStructureTypeCount:
        default:
          assert(!"Can't render build component");
      }
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kStructureComponent);
    while (itr.Next()) {
      PhysicsComponent* physics = itr.c.physics;
      StructureComponent* structure = itr.c.structure;
      switch (structure->structure_type) {
        case kWall:
          rgg::RenderRectangle(physics->rect(), v4f(.64f, .45f, .28f, 1.f));
          break;
        case kStructureTypeCount:
        default:
          assert(!"Can't render structure component");
      }
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kCharacterComponent);
    while (itr.Next()) {
      PhysicsComponent* character = itr.c.physics;
      r32 half_width = character->rect().width / 2.f;

      /*
      std::vector<v2i> grids = live::GridGetIntersectingCellPos(character);
      for (v2i grid : grids) {
        v2f grid_pos = live::GridPosFromXY(grid);
        rgg::RenderRectangle(Rectf(grid_pos, live::CellDims()), v4f(.2f, .6f, .2f, .8f));
      }*/

      live::AssetCharacterRender(character_texture, character->pos, live::CharacterAsset::kVillager);

      //rgg::RenderCircle(character->pos + v2f(half_width, half_width),
      //                  character->rect().width / 2.f, v4f(1.f, 0.f, 0.f, 1.f));

      if (kRenderCharacterAabb) {
        rgg::RenderLineRectangle(character->rect(), v4f(1.f, 0.f, 0.f, 1.f));
      }
      
      //v2f grid_pos;
      //if (live::GridClampPos(character->pos, &grid_pos)) {
      //  rgg::RenderRectangle(Rectf(grid_pos, live::CellDims()), v4f(.2f, .2f, .2f, .8f));
      //}
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kResourceComponent);
    while (itr.Next()) {
      // The resource should not be rendered if this is a tree for instance.
      if (GetHarvestComponent(itr.e)) continue;
      PhysicsComponent* physics = itr.c.physics;
      ResourceComponent* resource = itr.c.resource;
      Rectf rect = physics->rect();
      switch (resource->resource_type) {
        case kLumber:
          rgg::RenderTriangle(rect.Center(), rect.width / 2.f, v4f(.1f, .5f, .1f, 1.f));
          break;
        case kStone:
          rgg::RenderTriangle(rect.Center(), rect.width / 2.f, v4f(.5f, .5f, .5f, 1.f));
          break;
        case kResourceTypeCount:
        default:
          assert(!"Can't render resource type");
      }
    }
  }

  glEnable(GL_BLEND);

  if (kRenderGrid) {
    // TODO: Implement an active grid which is the current view I think.
    live::Grid* grid = live::GridGet(1);
    r32 grid_width = grid->width * live::kCellWidth;
    r32 grid_height = grid->height * live::kCellHeight;
    for (s32 x = 0; x < grid->width; ++x) {
      v2f start(x * live::kCellWidth, 0);
      v2f end(x * live::kCellWidth, grid_height);
      rgg::RenderLine(start, end, v4f(1.f, 1.f, 1.f, .15f));
    }
    for (s32 y = 0; y < grid->height; ++y) {
      v2f start(0, y * live::kCellHeight);
      v2f end(grid_width, y * live::kCellHeight);
      rgg::RenderLine(start, end, v4f(1.f, 1.f, 1.f, .15f));
    }
  }

}

}
