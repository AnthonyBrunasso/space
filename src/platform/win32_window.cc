#pragma once

#include "window.h"

#include <cassert>
#include <windows.h>
#include <xinput.h>
#include <gl/gl.h>


static DWORD XInputGetState_Stub(DWORD, XINPUT_STATE*) {
  return 1;
}

static DWORD XInputSetState_Stub(DWORD, XINPUT_VIBRATION*) { return 1;
}

typedef DWORD XInputGetState_Func(DWORD, XINPUT_STATE*);
XInputGetState_Func* __XInputGetState = XInputGetState_Stub;
typedef DWORD XInputSetState_Func(DWORD, XINPUT_VIBRATION*);
XInputSetState_Func* __XInputSetState = XInputSetState_Stub;

namespace window {

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
        const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

// See https://www.opengl.org/registry/specs/ARB/wgl_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
        const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

// See https://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023
#define WGL_SAMPLE_BUFFERS_ARB                    0x2041
#define WGL_SAMPLES_ARB                           0x2042

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

struct Window {
  // Handle to the current window.
  HWND hwnd;
  // Handle to the window device context.
  HDC hdc;
  // This is a handle to the OpenGL rendering context.
  HGLRC hglrc;
  // Whether the window should be closing or not.
  b8 should_close = false;
  // This is a pointer to the current platform event being
  // called with PollEvent. It is unsafe to modify this outside
  // of the context of WindowProc.
  PlatformEvent* platform_event;
};

static Window kWindow;

HMONITOR GetPrimaryMonitorHandle() {
  POINT pt_zero = {};
  return MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
}

void HandleKeyEvent(WPARAM wparam, b8 is_down, PlatformEvent* event) {
  event->type = is_down == kTrue ? KEY_DOWN : KEY_UP;
  if (wparam >= 'A' && wparam <= 'Z') event->key = (u32)wparam + 32;
  else event->key = (u32)wparam;
  switch (wparam) {
    case VK_OEM_PLUS: {
      event->key = 43;
    } break;
    case VK_OEM_MINUS: {
      event->key = 45;
    } break;
  }
}

void HandleMouseEvent(b8 is_down, PlatformEvent* event, PlatformButton button) {
  DWORD message_pos = GetMessagePos();
  POINTS ps = MAKEPOINTS(message_pos);
  POINT p;
  p.x = ps.x; p.y = ps.y;
  ScreenToClient(kWindow.hwnd, &p);
  v2f dims = GetWindowSize();
  event->position = v2f((r32)p.x, dims.y - (r32)p.y);
  event->type = is_down ? MOUSE_DOWN : MOUSE_UP;
  event->button = button;
}

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
  PlatformEvent* platform_event = kWindow.platform_event;
  LRESULT result = 0;

  switch (msg) {
    case WM_CLOSE: {
      kWindow.should_close = true;
    } break;
    case WM_MOVE: {
    } break;
    case WM_DESTROY: {
      PostQuitMessage(0);
    } break;
    case WM_KEYUP: {
      HandleKeyEvent(wparam, false, platform_event);
    } break;
    case WM_KEYDOWN: {
      HandleKeyEvent(wparam, true, platform_event);
    } break;
    case WM_LBUTTONDOWN: {
      HandleMouseEvent(true, platform_event, BUTTON_LEFT);
    } break;
    case WM_LBUTTONUP: {
      HandleMouseEvent(false, platform_event, BUTTON_LEFT);
    } break;
    case WM_RBUTTONDOWN: {
      HandleMouseEvent(true, platform_event, BUTTON_RIGHT);
    } break;
    case WM_RBUTTONUP: {
      HandleMouseEvent(false, platform_event, BUTTON_RIGHT);
    } break;
    case WM_MBUTTONDOWN: {
      HandleMouseEvent(true, platform_event, BUTTON_MIDDLE);
    } break;
    case WM_MBUTTONUP: {
      HandleMouseEvent(false, platform_event, BUTTON_MIDDLE);
    } break;
    case WM_MOUSEMOVE: {
      DWORD message_pos = GetMessagePos();
      POINTS ps = MAKEPOINTS(message_pos);
      POINT p;
      p.x = ps.x; p.y = ps.y;
      ScreenToClient(kWindow.hwnd, &p);
      v2f dims = GetWindowSize();
      platform_event->position = v2f((r32)p.x, dims.y - (r32)p.y);
      platform_event->type = MOUSE_MOVE;
    } break;
    case WM_MOUSEWHEEL: {
      platform_event->wheel_delta =
        (r32)GET_WHEEL_DELTA_WPARAM(wparam) / (r32)WHEEL_DELTA;
      platform_event->type = MOUSE_WHEEL;
    } break;
    case WM_SYSCOMMAND: {
      if (wparam == SC_CLOSE) {
        PostQuitMessage(0);
      }
    } break;
    default: {
      result = DefWindowProcA(window, msg, wparam, lparam); 
    } break;
  }
  return result;
}

bool PollXboxController() {
  PlatformEvent* platform_event = kWindow.platform_event;
  static u32 previous_sequence_num = -1;
  // Get state of controller.
  for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
    XINPUT_STATE state = {};
    // TODO: Takes first connected controller. Perhaps allow multiple
    // controllers.
    if (__XInputGetState(i, &state) == ERROR_SUCCESS) {
      if (state.dwPacketNumber == previous_sequence_num) {
        return false;
      }
      platform_event->type = XBOX_CONTROLLER;
      platform_event->controller.lstick_x = state.Gamepad.sThumbLX;
      platform_event->controller.lstick_y = state.Gamepad.sThumbLY;
      platform_event->controller.rstick_x = state.Gamepad.sThumbRX;
      platform_event->controller.rstick_y = state.Gamepad.sThumbRY;
      platform_event->controller.left_trigger = state.Gamepad.bLeftTrigger;
      platform_event->controller.right_trigger = state.Gamepad.bRightTrigger;
      platform_event->controller.controller_flags = state.Gamepad.wButtons;
      previous_sequence_num = state.dwPacketNumber;
      return true;
    }
  }
  return false;
}

HWND SetupWindow(HINSTANCE inst, const char* name, const CreateInfo& create_info) {
  WNDCLASSA wc = {};
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = inst;
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = 0;
  wc.lpszClassName = name;

  if (!RegisterClassA(&wc)) {
    assert("Failed to register window.");
  }
 
  DWORD window_style = WS_POPUP;
  DWORD window_extended_style = WS_EX_APPWINDOW;
  RECT rect = {};
  rect.right = (LONG)create_info.window_width;
  rect.bottom = (LONG)create_info.window_height;

  if (create_info.fullscreen) {
    DEVMODE screen_settings = {};
    // TODO(abrunasso): This doesn't look nice when it's not the users native
    // resolution. Is there a solution to that?
    screen_settings.dmSize = sizeof(DEVMODE);
    screen_settings.dmPelsWidth = (DWORD)create_info.window_width;
    screen_settings.dmPelsHeight = (DWORD)create_info.window_height;
    screen_settings.dmBitsPerPel = 32;
    screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
    ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);
    window_extended_style |= WS_EX_TOPMOST;
  } else {
    AdjustWindowRectEx(&rect, window_style, false, WS_EX_APPWINDOW);
  }

  s32 x = 0;
  s32 y = 0;
  s32 nw = rect.right - rect.left;
  s32 nh = rect.bottom - rect.top;

  if (!create_info.fullscreen && create_info.window_pos_x == UINT64_MAX &&
      create_info.window_pos_y == UINT64_MAX) {
    // Center the window if the user is not in fullscreen.
    RECT parent_rect;
    GetClientRect(GetDesktopWindow(), &parent_rect);
    x = (parent_rect.right / 2) - (int)(nw / 2.f);
    y = (parent_rect.bottom / 2) - (int)(nh / 2.f);
  } else if (!create_info.fullscreen) {
    if (create_info.window_pos_x != UINT64_MAX) x = (s32)create_info.window_pos_x;
    if (create_info.window_pos_y != UINT64_MAX) y = (s32)create_info.window_pos_y;
  }

  HWND window = CreateWindowExA(WS_EX_APPWINDOW, wc.lpszClassName, name,
                                window_style, x, y, nw, nh, 0, 0, inst, 0);

  if (!window) {
    assert("Failed to create window.");
  }

  
  return window;
}

void InitOpenGLExtensions(void) {
  // https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
  // See "Create a False Context" for why this is necessary.
  WNDCLASSA wc = {};
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = DefWindowProcA;
  wc.hInstance = GetModuleHandle(0);
  wc.lpszClassName = "Dummy_WGL_djuasiodwa";

  if (!RegisterClassA(&wc)) {
    assert("Failed to register dummy OpenGL window.");
  }

  HWND dummy_window = CreateWindowExA(
      0,
      wc.lpszClassName,
      "NULL WINDOW",
      0,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      wc.hInstance,
      0);

  if (!dummy_window) {
    assert("Failed to create dummy OpenGL window.");
  }

  HDC dummy_dc = GetDC(dummy_window);

  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cAlphaBits = 8;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
  if (!pixel_format) {
    assert("Failed to find a suitable pixel format.");
  }
  if (!SetPixelFormat(dummy_dc, pixel_format, &pfd)) {
    assert("Failed to set the pixel format.");
  }

  HGLRC dummy_context = wglCreateContext(dummy_dc);
  if (!dummy_context) {
    assert("Failed to create a dummy OpenGL rendering context.");
  }

  if (!wglMakeCurrent(dummy_dc, dummy_context)) {
    assert("Failed to activate dummy OpenGL rendering context.");
  }

  wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
      "wglCreateContextAttribsARB");
  assert(wglCreateContextAttribsARB);
  wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
      "wglChoosePixelFormatARB");
  assert(wglChoosePixelFormatARB);

  wglMakeCurrent(dummy_dc, 0);
  wglDeleteContext(dummy_context);
  ReleaseDC(dummy_window, dummy_dc);
  DestroyWindow(dummy_window);
}

HGLRC InitOpenGL(HDC real_dc) {
  InitOpenGLExtensions();

  int pixel_format_attribs[] = {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
      WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
      WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
      WGL_COLOR_BITS_ARB,     32,
      WGL_DEPTH_BITS_ARB,     24,
      WGL_STENCIL_BITS_ARB,    8,
      WGL_SAMPLE_BUFFERS_ARB,  1, // Number of buffers (must be 1 at time of writing)
      WGL_SAMPLES_ARB,         0, // Number of samples
      0
  };

  int pixel_format;
  UINT num_formats;
  wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
  if (!num_formats) {
    assert("Failed to set the OpenGL 3.3 pixel format.");
  }

  PIXELFORMATDESCRIPTOR pfd;
  DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
  if (!SetPixelFormat(real_dc, pixel_format, &pfd)) {
    assert("Failed to set the OpenGL 3.3 pixel format.");
  }

  // Specify that we want to create an OpenGL 3.3 core profile context
  int gl33_attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0,
  };

  HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
  if (!gl33_context) {
    assert("Failed to create OpenGL 3.3 context.");
  }

  if (!wglMakeCurrent(real_dc, gl33_context)) {
    assert("Failed to activate OpenGL 3.3 rendering context.");
  }

  return gl33_context;
}


void SetupXboxController() {
  HMODULE xinput_library = LoadLibraryW(L"xinput1_4.dll");
  if (!xinput_library) {
    xinput_library = LoadLibraryW(L"xinput9_1_0.dll");
  }
  if (!xinput_library) {
    return;
  }
  __XInputGetState = (XInputGetState_Func*)GetProcAddress(xinput_library, "XInputGetState");
  __XInputSetState = (XInputSetState_Func*)GetProcAddress(xinput_library, "XInputSetState");
}

int Create(const char* name, int width, int height, b8 fullscreen) { 
  // TODO: Remove this implementation when other platforms move to new api.
  CreateInfo info;
  info.window_width = width;
  info.window_height = height;
  info.fullscreen = fullscreen;
  kWindow.hwnd = SetupWindow(GetModuleHandle(0), name, info);
  kWindow.hdc = GetDC(kWindow.hwnd);
  kWindow.hglrc = InitOpenGL(kWindow.hdc);
  SetupGLFunctions();
  SetupXboxController();

  ShowWindow(kWindow.hwnd, 1);
  UpdateWindow(kWindow.hwnd);
  return 1;
}

int Create(const char* name, const CreateInfo& create_info) {
  kWindow.hwnd = SetupWindow(GetModuleHandle(0), name, create_info);
  kWindow.hdc = GetDC(kWindow.hwnd);
  kWindow.hglrc = InitOpenGL(kWindow.hdc);
  SetupGLFunctions();
  SetupXboxController();

  ShowWindow(kWindow.hwnd, 1);
  UpdateWindow(kWindow.hwnd);

  return 1;
}

b8 PollEvent(PlatformEvent* event) {
  *event = {};
  kWindow.platform_event = event;

  MSG msg;
  if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      kWindow.should_close = true;
    } else {
      TranslateMessage(&msg);
      // This dispatches messages to the WindowProc function.
      DispatchMessageA(&msg);
    }
    return true;
  }

  if (PollXboxController()) {
    return true;
  }

#if 0
  // Keep cursor in screen.
  RECT crect;
  GetClientRect(kWindow.hwnd, &crect);
  ClientToScreen(kWindow.hwnd, (POINT*)(&crect.left));
  ClientToScreen(kWindow.hwnd, (POINT*)(&crect.right));
  ClipCursor(&crect);
#endif

  return false;
}

void SwapBuffers() {
  SwapBuffers(kWindow.hdc);
}

b8 ShouldClose() {
  return kWindow.should_close;
}

v2f GetWindowSize() {
  RECT rect;
  GetClientRect(kWindow.hwnd, &rect);
  return v2f((r32)rect.right - rect.left, (r32)rect.bottom - rect.top);
}

v2f GetPrimaryMonitorSize() {
  return v2f((r32)GetSystemMetrics(SM_CXSCREEN), (r32)GetSystemMetrics(SM_CYSCREEN));
}

v2f GetCursorPosition() {
  POINT cursor;
  GetCursorPos(&cursor);
  ScreenToClient(kWindow.hwnd, &cursor);
  v2f dims = GetWindowSize();
  return v2f((r32)cursor.x, dims.y - (r32)cursor.y);
}

void SetCursorPosition(v2f pos) {
  //v2f dims = GetWindowSize();
  SetCursorPos((s32)pos.x, (s32)pos.y);
}

const char* GetBinaryPath() {
  static bool kDoOnce = true;
  static char kStrBinPath[256];
  if (kDoOnce) {
    static TCHAR kBinPath[256];
    GetCurrentDirectory(256, kBinPath);
    size_t n = 0;
    wcstombs_s(&n, kStrBinPath, 256, kBinPath, 256);
    kDoOnce = false;
  }
  return &kStrBinPath[0];
}

}  // namespace window
