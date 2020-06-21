#include "window.h"

#include <cassert>
#include <windows.h>
#include <xinput.h>
#include <gl/gl.h>

// https://www.khronos.org/registry/OpenGL/api/GL/glext.h

// GL types.
typedef size_t GLsizeiptr;
typedef char GLchar;
typedef float GLfloat;

// GL functions loaded using wglGetProcAddress.
typedef void glGenBuffers_Func(GLsizei, GLuint*);
glGenBuffers_Func* glGenBuffers;
typedef void glBindBuffer_Func(GLenum, GLuint);
glBindBuffer_Func* glBindBuffer;
typedef void glBufferData_Func(GLenum, GLsizeiptr, const void*, GLenum);
glBufferData_Func* glBufferData;
typedef void glGenVertexArrays_Func(GLsizei, GLuint*);
glGenVertexArrays_Func* glGenVertexArrays;
typedef void glBindVertexArray_Func(GLuint);
glBindVertexArray_Func* glBindVertexArray;
typedef void glEnableVertexAttribArray_Func(GLuint);
glEnableVertexAttribArray_Func* glEnableVertexAttribArray;
typedef void glVertexAttribPointer_Func(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
glVertexAttribPointer_Func* glVertexAttribPointer;
typedef GLuint glCreateShader_Func(GLenum);
glCreateShader_Func* glCreateShader;
typedef void glShaderSource_Func(GLuint, GLsizei, const GLchar* const*, const GLint*);
glShaderSource_Func* glShaderSource;
typedef void glCompileShader_Func(GLuint);
glCompileShader_Func* glCompileShader;
typedef GLuint glCreateProgram_Func(void);
glCreateProgram_Func* glCreateProgram;
typedef void glAttachShader_Func(GLuint, GLuint);
glAttachShader_Func* glAttachShader;
typedef void glLinkProgram_Func(GLuint);
glLinkProgram_Func* glLinkProgram;
typedef void glUseProgram_Func(GLuint);
glUseProgram_Func* glUseProgram;
typedef GLint glGetUniformLocation_Func(GLuint, const GLchar*);
glGetUniformLocation_Func* glGetUniformLocation;
typedef void glUniformMatrix4fv_Func(GLint, GLsizei, GLboolean, const GLfloat*);
glUniformMatrix4fv_Func* glUniformMatrix4fv;
typedef void glGetShaderInfoLog_Func(GLuint, GLsizei, GLsizei*, GLchar*);
glGetShaderInfoLog_Func* glGetShaderInfoLog;
typedef void glGetProgramInfoLog_Func(GLuint, GLsizei, GLsizei*, GLchar*);
glGetProgramInfoLog_Func* glGetProgramInfoLog;
typedef void glGetShaderiv_Func(GLuint, GLenum, GLint*);
glGetShaderiv_Func* glGetShaderiv;
typedef void glGetProgramiv_Func(GLuint, GLenum, GLint*);
glGetProgramiv_Func* glGetProgramiv;
typedef void glGetActiveAttrib_Func(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
glGetActiveAttrib_Func* glGetActiveAttrib;
typedef GLint glGetAttribLocation_Func(GLuint, const GLchar*);
glGetAttribLocation_Func* glGetAttribLocation;
typedef void glGetActiveUniform_Func(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
glGetActiveUniform_Func* glGetActiveUniform;
typedef void glUniform4f_Func(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
glUniform4f_Func* glUniform4f;
typedef void glUniform3f_Func(GLint, GLfloat, GLfloat, GLfloat);
glUniform3f_Func* glUniform3f;
typedef void glActiveTexture_Func(GLenum);
glActiveTexture_Func* glActiveTexture;
typedef void glUniform1i_Func(GLint, GLint);
glUniform1i_Func* glUniform1i;
typedef void glDeleteShader_Func(GLuint);
glDeleteShader_Func* glDeleteShader;
typedef void glGenFramebuffers_Func(GLsizei, GLuint*);
glGenFramebuffers_Func* glGenFramebuffers;
typedef void glBindFramebuffer_Func(GLenum, GLuint);
glBindFramebuffer_Func* glBindFramebuffer;
typedef void glFramebufferTexture_Func(GLenum, GLenum, GLuint, GLint);
glFramebufferTexture_Func* glFramebufferTexture;
typedef void glDrawBuffers_Func(GLsizei, const GLenum*);
glDrawBuffers_Func* glDrawBuffers;
typedef void glUniform1f_Func(GLint, GLfloat);
glUniform1f_Func* glUniform1f;
typedef void glGenerateMipmap_Func(GLenum);
glGenerateMipmap_Func* glGenerateMipmap;

// GL defines.
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_BOOL                           0x8B56
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
#define GL_SAMPLER_CUBE                   0x8B60
#define GL_SAMPLER_2D_SHADOW              0x8B62
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_FRAMEBUFFER                    0x8D40
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1

static DWORD XInputGetState_Stub(DWORD, XINPUT_STATE*)
{
  printf("Calling stub...\n");
  return 1;
}

static DWORD XInputSetState_Stub(DWORD, XINPUT_VIBRATION*)
{
  return 1;
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

void
HandleKeyEvent(WPARAM wparam, b8 is_down, PlatformEvent* event)
{
  event->type = is_down == kTrue ? KEY_DOWN : KEY_UP;
  if (wparam >= 'A' && wparam <= 'Z') event->key = wparam + 32;
  else event->key = wparam;
}

void
HandleMouseEvent(b8 is_down, PlatformEvent* event, PlatformButton button)
{
  DWORD message_pos = GetMessagePos();
  POINTS ps = MAKEPOINTS(message_pos);
  POINT p;
  p.x = ps.x; p.y = ps.y;
  ScreenToClient(kWindow.hwnd, &p);
  v2f dims = GetWindowSize();
  event->position = v2f(p.x, dims.y - p.y);
  event->type = is_down ? MOUSE_DOWN : MOUSE_UP;
  event->button = button;
}

LRESULT CALLBACK
WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
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

bool
PollXboxController()
{
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

HWND
SetupWindow(HINSTANCE inst, const char* name, const CreateInfo& create_info)
{
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
  rect.right = create_info.window_width;
  rect.bottom = create_info.window_height;

  if (create_info.fullscreen) {
    DEVMODE screen_settings = {};
    // TODO(abrunasso): This doesn't look nice when it's not the users native
    // resolution. Is there a solution to that?
    screen_settings.dmSize = sizeof(DEVMODE);
    screen_settings.dmPelsWidth = create_info.window_width;
    screen_settings.dmPelsHeight = create_info.window_height;
    screen_settings.dmBitsPerPel = 32;
    screen_settings.dmFields = DM_BITSPERPEL| DM_PELSWIDTH| DM_PELSHEIGHT;
    ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);
    window_extended_style |= WS_EX_TOPMOST;
  } else {
    AdjustWindowRectEx(&rect, window_style, false, WS_EX_APPWINDOW);
  }

  int x = 0;
  int y = 0;
  int nw = rect.right - rect.left;
  int nh = rect.bottom - rect.top;

  if (!create_info.fullscreen && create_info.window_pos_x == UINT64_MAX &&
      create_info.window_pos_y == UINT64_MAX) {
    // Center the window if the user is not in fullscreen.
    RECT parent_rect;
    GetClientRect(GetDesktopWindow(), &parent_rect);
    x = (parent_rect.right / 2) - (nw / 2.f);
    y = (parent_rect.bottom / 2) - (nh / 2.f);
  } else if (!create_info.fullscreen) {
    if (create_info.window_pos_x != UINT64_MAX) x = create_info.window_pos_x;
    if (create_info.window_pos_y != UINT64_MAX) y = create_info.window_pos_y;
  }

  HWND window = CreateWindowExA(WS_EX_APPWINDOW, wc.lpszClassName, name,
                                window_style, x, y, nw, nh, 0, 0, inst, 0);

  if (!window) {
    assert("Failed to create window.");
  }

  
  return window;
}

void
InitOpenGLExtensions(void)
{
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

HGLRC
InitOpenGL(HDC real_dc)
{
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
      WGL_SAMPLES_ARB,         4, // Number of samples
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

void*
GetGLFunction(const char* name)
{
  void *p = (void *)wglGetProcAddress(name);
  if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) ||
               (p == (void*)0x3) || (p == (void*)-1)) {
    static HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }
  assert(p);
  return p;
}

void
SetupGLFunctions() {
  glGenBuffers = (glGenBuffers_Func*)GetGLFunction("glGenBuffers");
  glBindBuffer = (glBindBuffer_Func*)GetGLFunction("glBindBuffer");
  glBufferData = (glBufferData_Func*)GetGLFunction("glBufferData");
  glGenVertexArrays = (glGenVertexArrays_Func*)GetGLFunction("glGenVertexArrays");
  glBindVertexArray = (glBindVertexArray_Func*)GetGLFunction("glBindVertexArray");
  glEnableVertexAttribArray = (glEnableVertexAttribArray_Func*)GetGLFunction("glEnableVertexAttribArray");
  glVertexAttribPointer = (glVertexAttribPointer_Func*)GetGLFunction("glVertexAttribPointer");
  glCreateShader = (glCreateShader_Func*)GetGLFunction("glCreateShader");
  glShaderSource = (glShaderSource_Func*)GetGLFunction("glShaderSource");
  glCompileShader = (glCompileShader_Func*)GetGLFunction("glCompileShader");
  glCreateProgram = (glCreateProgram_Func*)GetGLFunction("glCreateProgram");
  glAttachShader = (glAttachShader_Func*)GetGLFunction("glAttachShader");
  glLinkProgram = (glLinkProgram_Func*)GetGLFunction("glLinkProgram");
  glUseProgram = (glUseProgram_Func*)GetGLFunction("glUseProgram");
  glGetUniformLocation  = (glGetUniformLocation_Func*)GetGLFunction("glGetUniformLocation");
  glUniformMatrix4fv = (glUniformMatrix4fv_Func*)GetGLFunction("glUniformMatrix4fv");
  glGetShaderInfoLog = (glGetShaderInfoLog_Func*)GetGLFunction("glGetShaderInfoLog");
  glGetProgramInfoLog = (glGetProgramInfoLog_Func*)GetGLFunction("glGetProgramInfoLog");
  glGetShaderiv = (glGetShaderiv_Func*)GetGLFunction("glGetShaderiv");
  glGetProgramiv = (glGetProgramiv_Func*)GetGLFunction("glGetProgramiv");
  glGetActiveAttrib = (glGetActiveAttrib_Func*)GetGLFunction("glGetActiveAttrib");
  glGetAttribLocation = (glGetAttribLocation_Func*)GetGLFunction("glGetAttribLocation");
  glGetActiveUniform = (glGetActiveUniform_Func*)GetGLFunction("glGetActiveUniform");
  glUniform4f = (glUniform4f_Func*)GetGLFunction("glUniform4f");
  glUniform3f = (glUniform3f_Func*)GetGLFunction("glUniform3f");
  glActiveTexture = (glActiveTexture_Func*)GetGLFunction("glActiveTexture");
  glUniform1i = (glUniform1i_Func*)GetGLFunction("glUniform1i");
  glDeleteShader = (glDeleteShader_Func*)GetGLFunction("glDeleteShader");
  glGenFramebuffers = (glGenFramebuffers_Func*)GetGLFunction("glGenFramebuffers");
  glBindFramebuffer = (glBindFramebuffer_Func*)GetGLFunction("glBindFramebuffer");
  glFramebufferTexture = (glFramebufferTexture_Func*)GetGLFunction("glFramebufferTexture");
  glDrawBuffers = (glDrawBuffers_Func*)GetGLFunction("glDrawBuffers");
  glUniform1f = (glUniform1f_Func*)GetGLFunction("glUniform1f");
  glGenerateMipmap = (glGenerateMipmap_Func*)GetGLFunction("glGenerateMipmap");
}

void
SetupXboxController()
{
  HMODULE xinput_library = LoadLibrary(L"xinput1_4.dll");
  if (!xinput_library) {
    xinput_library = LoadLibrary(L"xinput9_1_0.dll");
  }
  if (!xinput_library) {
    return;
  }
  printf("Loaded xinput\n");
  __XInputGetState = (XInputGetState_Func*)GetProcAddress(xinput_library, "XInputGetState");
  __XInputSetState = (XInputSetState_Func*)GetProcAddress(xinput_library, "XInputSetState");
}

int
Create(const char* name, int width, int height, b8 fullscreen)
{ 
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

int
Create(const char* name, const CreateInfo& create_info)
{
  printf("Create %lu %lu\n", create_info.window_width, create_info.window_height);
  kWindow.hwnd = SetupWindow(GetModuleHandle(0), name, create_info);
  kWindow.hdc = GetDC(kWindow.hwnd);
  kWindow.hglrc = InitOpenGL(kWindow.hdc);
  SetupGLFunctions();
  SetupXboxController();

  ShowWindow(kWindow.hwnd, 1);
  UpdateWindow(kWindow.hwnd);

  return 1;
}

b8
PollEvent(PlatformEvent* event)
{
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

void
SwapBuffers()
{
  SwapBuffers(kWindow.hdc);
}

b8
ShouldClose()
{
  return kWindow.should_close;
}

v2f
GetWindowSize()
{
  RECT rect;
  GetClientRect(kWindow.hwnd, &rect);
  return v2f((r32)rect.right - rect.left, (r32)rect.bottom - rect.top);
}


v2f
GetCursorPosition()
{
  POINT cursor;
  GetCursorPos(&cursor);
  ScreenToClient(kWindow.hwnd, &cursor);
  v2f dims = GetWindowSize();
  return v2f((r32)cursor.x, dims.y - (r32)cursor.y);
}
}  // namespace window
