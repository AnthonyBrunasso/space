#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common/common.cc"
#include "math/math.cc"
// TODO: Remove this.
#include "memory/memory.cc"

// Engine stuff.
#include "platform/platform.cc"
#include "imgui.h"
#include "gl/gl.cc"

#include "asset/cube.cc"
#include "asset/cone.cc"
#include "asset/sphere.cc"
#include "asset/font.cc"

#include "animation/sprite.cc"
#include "renderer/constants.cc"
// TODO: This is being replaced with imgui.
#include "renderer/shader.h"
// I DO THIS DONT TRY TO DO IT AGAIN IMGUI
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM 1
#include "renderer/imgui_opengl3.cc"
#include "renderer/mesh.cc"
#include "renderer/imgui_impl.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "renderer/imui.cc"

#include "math/math.cc"
#include "util/cooldown.cc"
#include "animation/fsm.cc"
