#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>

#include "common/common.cc"
#include "math/math.cc"
// TODO: Remove this.
#include "memory/memory.cc"

// Engine stuff.
#include "platform/platform.cc"
#include "imgui.h"

// TODO: Remove cube, cone and sphere probably.
#include "asset/cube.cc"
#include "asset/cone.cc"
#include "asset/sphere.cc"
#include "asset/font.cc"

#include "renderer/opengl3_includes.cc"
#include "2d/anim2d.cc"
#include "2d/map2d.cc"
#include "2d/entity2d.cc"

#include "math/math.cc"
#include "util/cooldown.cc"
