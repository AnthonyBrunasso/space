#pragma once

#include "math/vec.h"

// Cross-platform window / opengl context abstraction. The purpose
// of these functions are to setup a window on the screen and setup
// the opengl context.
//
// Notably this module creates a single window and does not intend
// on supporting making multiple.

enum PlatformEventType {
  NOT_IMPLEMENTED,  // Event does not have translation implemented yet.
  MOUSE_DOWN,
  MOUSE_UP,
  MOUSE_WHEEL,
  KEY_DOWN,
  KEY_UP,
  MOUSE_POSITION,
  XBOX_CONTROLLER,
};

enum PlatformButton {
  BUTTON_UNKNOWN,
  BUTTON_LEFT = 1,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
};

enum ControllerButton {
  XBOX_CONTROLLER_UP = 0,
  XBOX_CONTROLLER_DOWN = 1,
  XBOX_CONTROLLER_LEFT = 2,
  XBOX_CONTROLLER_RIGHT = 3,

  XBOX_CONTROLLER_START = 4,
  XBOX_CONTROLLER_BACK = 5,

  XBOX_CONTROLLER_LEFT_THUMB = 6,
  XBOX_CONTROLLER_RIGHT_THUMB = 7,

  XBOX_CONTROLLER_LEFT_SHOULDER = 8,
  XBOX_CONTROLLER_RIGHT_SHOULDER = 9,

  XBOX_CONTROLLER_A = 12,
  XBOX_CONTROLLER_B = 13,
  XBOX_CONTROLLER_X = 14,
  XBOX_CONTROLLER_Y = 15,
};

struct ControllerState {
  // Left stick x / y.
  s16 lstick_x;
  s16 lstick_y;
  // Right stick x / y.
  s16 rstick_x;
  s16 rstick_y;
  // Whether the left and right trigger are pressed.
  b8 left_trigger;
  b8 right_trigger;
  // bitfield containing fields from ControllerButton enum.
  u16 controller_flags;
};

struct PlatformEvent {
  // Type of event.
  PlatformEventType type;
  // Screen space the event took place in.
  v2f position;
  // Event Detail
  union {
    r32 wheel_delta;
    char key;
    PlatformButton button;
    ControllerState controller;
  };
};

namespace window
{

struct CreateInfo {
  u64 window_width = 1920;
  u64 window_height = 1080;
  // If left as UINT64_MAX let the platform decide where the window should go.
  u64 window_pos_x = UINT64_MAX;
  u64 window_pos_y = UINT64_MAX;
  b8 fullscreen = false;
};

int Create(const char* name, s32 width, s32 height, b8 fullscreen);

int Create(const char* name, const CreateInfo& create_info);

// Returns true if an event existed. False otherwise.
// Fully poll this queue at the top of each game loop.
b8 PollEvent(PlatformEvent* event);

void SwapBuffers();

b8 ShouldClose();

v2f GetWindowSize();

v2f GetPrimaryMonitorSize();

v2f GetCursorPosition();

const char* GetBinaryPath();

}  // namespace window
