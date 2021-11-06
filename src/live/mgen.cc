// namespace live {

void
GenTrees(v2f min, v2f max, u32 grid_id)
{
  Rectf rect(min, max);
  for (int i = 0; i < 128; ++i) { 
    v2f p = math::RandomPointInRect(rect);
    //printf("x:%.2f y:%.2f w:%.2f h:%.2f\n", rect.x,  rect.y, rect.width, rect.height);
    //printf("x:%.2f,y:%.2f\n", p.x, p.y);
    SimCreateHarvest(kLumber, p, grid_id, kSecsToHarvestLumber);
  }
}

// }
