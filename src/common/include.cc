#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "common/common.cc"
#include "math/math.cc"
// TODO: Remove this.
#include "memory/memory.cc"

// Engine stuff.
#include "platform/platform.cc"
#include "imgui.h"

#include "asset/cube.cc"
#include "asset/cone.cc"
#include "asset/sphere.cc"
#include "asset/font.cc"

#include "animation/sprite.cc"
#include "renderer/opengl3_includes.cc"

#include "math/math.cc"
#include "util/cooldown.cc"
#include "animation/fsm.cc"
