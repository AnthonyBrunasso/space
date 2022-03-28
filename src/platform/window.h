#pragma once

// Cross-platform window / opengl context abstraction. The purpose
// of these functions are to setup a window on the screen and setup
// the opengl context.
//
// Notably this module creates a single window and does not intend
// on supporting making multiple.

constexpr u32 KEY_ESC = 65307;
constexpr u32 KEY_A = 97;
constexpr u32 KEY_B = 98;
constexpr u32 KEY_C = 99;
constexpr u32 KEY_D = 100;
constexpr u32 KEY_E = 101;
constexpr u32 KEY_F = 102;
constexpr u32 KEY_G = 103;
constexpr u32 KEY_H = 104;
constexpr u32 KEY_I = 105;
constexpr u32 KEY_J = 106;
constexpr u32 KEY_K = 107;
constexpr u32 KEY_L = 108;
constexpr u32 KEY_M = 109;
constexpr u32 KEY_N = 110;
constexpr u32 KEY_O = 111;
constexpr u32 KEY_P = 112;
constexpr u32 KEY_Q = 113;
constexpr u32 KEY_R = 114;
constexpr u32 KEY_S = 115;
constexpr u32 KEY_T = 116;
constexpr u32 KEY_U = 117;
constexpr u32 KEY_V = 118;
constexpr u32 KEY_W = 119;
constexpr u32 KEY_X = 120;
constexpr u32 KEY_Y = 121;
constexpr u32 KEY_Z = 122;
constexpr u32 KEY_RETURN = 65293;
constexpr u32 KEY_TAB = 65289;
constexpr u32 KEY_HOME = 65360;
constexpr u32 KEY_END = 65367;
constexpr u32 KEY_DEL = 65535;
constexpr u32 KEY_BACKSPACE = 65288;
constexpr u32 KEY_SPACE = 32;
constexpr u32 KEY_COMMA = 44;
constexpr u32 KEY_PERIOD = 46;
constexpr u32 KEY_SLASH = 47;
constexpr u32 KEY_SEMICOLON = 59;
constexpr u32 KEY_EQUALS = 61;
#ifdef _WIN32
constexpr u32 KEY_ARROW_UP = 0;
constexpr u32 KEY_ARROW_RIGHT = 3;
constexpr u32 KEY_ARROW_DOWN = 1;
constexpr u32 KEY_ARROW_LEFT = 2;
#else
constexpr u32 KEY_ARROW_UP = 65362;
constexpr u32 KEY_ARROW_RIGHT = 65363; 
constexpr u32 KEY_ARROW_DOWN = 65364;
constexpr u32 KEY_ARROW_LEFT = 65361;
#endif
constexpr u32 KEY_NUMPAD_UP = 65431;
constexpr u32 KEY_NUMPAD_RIGHT = 65432;
constexpr u32 KEY_NUMPAD_DOWN = 65433;
constexpr u32 KEY_NUMPAD_LEFT = 65430;

enum PlatformEventType {
  NOT_IMPLEMENTED,  // Event does not have translation implemented yet.
  MOUSE_DOWN,
  MOUSE_UP,
  MOUSE_WHEEL,
  MOUSE_MOVE,
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
    u32 key;
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

void SetCursorPosition();

const char* GetBinaryPath();

}  // namespace window
