#pragma once

std::vector<std::string> GetEntityBlueprints() {
  std::vector<std::string> blueprints;
  filesystem::WalkDirectory(kEntitiesDir, [&](const char* filename, bool is_dir) {
    if (strcmp(filename, ".") == 0) return;
    if (strcmp(filename, "..") == 0) return;
    assert(is_dir == false); // Entities can't have a subdir
    blueprints.push_back(filesystem::JoinPath(kEntitiesDir, filename));
  });
  return blueprints;
}
