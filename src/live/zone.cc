// namespace live {

b8
ZoneHasResourceForPickup(ZoneComponent* zone, ResourceComponent** resource, ResourceType resource_type)
{
  Entity* zone_entity = FindEntity(zone->entity_id);
  assert(zone_entity);
  PhysicsComponent* zone_physics = GetPhysicsComponent(zone_entity);
  assert(zone_physics);
  Grid* grid = GridGet(zone_physics->grid_id);
  assert(grid);
  std::vector<v2i> grid_cells = GridGetIntersectingCellPos(zone_physics);
  for (v2i cell : grid_cells) {
    // printf("Looking at cell %i %i\n", cell.x, cell.y);
    Cell* gcell = grid->Get(cell);
    assert(gcell != nullptr);
    // If the cell does not have a resource or building entity.
    for (u32 entity_id : gcell->entity_ids) {
      Entity* entity = FindEntity(entity_id);
      if (entity->Has(kResourceComponent) && !entity->Has(kOrderComponent)) {
        ResourceComponent* entity_resource = GetResourceComponent(entity);
        if (entity_resource->resource_type == resource_type) {
          *resource = entity_resource;
          return true;
        }
      }
    }
  }
  return false;
}

b8
ZoneHasCapacity(ZoneComponent* zone)
{
  for (const ZoneCell& zcell : zone->zone_cells) {
    if (!zcell.reserved) return true;
  }
  return false;
}

// }
