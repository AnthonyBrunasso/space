#pragma once

#include "util/misc.cc"

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

void CreateProtoEnumStrings(char**& type_strings, const google::protobuf::EnumDescriptor* desc) {
  type_strings = new char*[desc->value_count()];
  for (s32 i = 0; i < desc->value_count(); ++i) {
    std::string name = desc->FindValueByNumber(i)->name();
    type_strings[i] = new char[name.size()];
    strcpy(type_strings[i], name.data());
  }
}
