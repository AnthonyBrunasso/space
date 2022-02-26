#define GL_GLEXT_PROTOTYPES
#include <X11/Xlib.h>

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"
#include "GL/glext.h"

#include "window.h"

namespace window
{

static r32 kWidthPixels;
static r32 kHeightPixels;
static r32 kMouseX;
static r32 kMouseY;
static Display* kDisplay;
static s32 kWindowId;
static EGLDisplay kEglDisplay;
static EGLSurface kEglSurface;


s32 x11_error_handler(Display*, XErrorEvent*) {
  return 0;
}

s32 x11_ioerror_handler(Display*) {
  return 0;
}

s32 Create(const char* name, s32 width, s32 height, b8 fullscreen) {
  if (kWindowId) return 0;

  kDisplay = XOpenDisplay(NULL);
  s32 screen = DefaultScreen(kDisplay);
  Window root_window = RootWindow(kDisplay, screen);

  s32 egl_major = 0;
  s32 egl_minor = 0;
  kEglDisplay = eglGetDisplay(kDisplay);
  s32 egl_version_ok = eglInitialize(kEglDisplay, &egl_major, &egl_minor);
  if (!egl_version_ok) {
    return 0;
  }

  // egl config
  s32 egl_config_count;
  if (!eglGetConfigs(kEglDisplay, NULL, 0, &egl_config_count)) {
    return 0;
  }

#define MAX_EGL_CONFIG 40
  if (egl_config_count > MAX_EGL_CONFIG) {
    egl_config_count = MAX_EGL_CONFIG;
  }

  EGLConfig egl_config[MAX_EGL_CONFIG];
  if (!eglGetConfigs(kEglDisplay, egl_config, egl_config_count,
                     &egl_config_count)) {
    return 0;
  }

  s32 selected_config = 0;
  for (s32 i = 0; i < egl_config_count; ++i) {
    s32 color_type;
    s32 renderable_type;
#define EGL_READ_CONFIG(attrib, x) \
  eglGetConfigAttrib(kEglDisplay, egl_config[i], attrib, x);
    EGL_READ_CONFIG(EGL_COLOR_BUFFER_TYPE, &color_type);
    EGL_READ_CONFIG(EGL_RENDERABLE_TYPE, &renderable_type);

    s32 red_bits;
    s32 green_bits;
    s32 blue_bits;
    eglGetConfigAttrib(kEglDisplay, egl_config[i], EGL_RED_SIZE, &red_bits);
    eglGetConfigAttrib(kEglDisplay, egl_config[i], EGL_GREEN_SIZE, &green_bits);
    eglGetConfigAttrib(kEglDisplay, egl_config[i], EGL_BLUE_SIZE, &blue_bits);

    s32 alpha_bits;
    s32 depth_bits;
    s32 stencil_bits;
    eglGetConfigAttrib(kEglDisplay, egl_config[i], EGL_ALPHA_SIZE, &alpha_bits);
    eglGetConfigAttrib(kEglDisplay, egl_config[i], EGL_DEPTH_SIZE, &depth_bits);
    eglGetConfigAttrib(kEglDisplay, egl_config[i], EGL_STENCIL_SIZE,
                       &stencil_bits);

    s32 egl_samples;
    eglGetConfigAttrib(kEglDisplay, egl_config[i], EGL_SAMPLES, &egl_samples);

    if (red_bits + green_bits + blue_bits != 24) continue;
    if (alpha_bits != 8) continue;
    if (depth_bits != 24) continue;
    if (stencil_bits != 8) continue;
    selected_config = i;
    break;
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    return 0;
  }
#define MAX_EGL_ATTRIB 16
  s32 attrib[MAX_EGL_ATTRIB] = {0};
  s32* write_attrib = attrib;
  *write_attrib++ = EGL_CONTEXT_MAJOR_VERSION;
  *write_attrib++ = 4;
  *write_attrib++ = EGL_CONTEXT_MINOR_VERSION;
  *write_attrib++ = 1;
  *write_attrib++ = EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR;
  *write_attrib++ = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
  *write_attrib++ = EGL_CONTEXT_FLAGS_KHR;
  *write_attrib++ = EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR |
                    EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
  *write_attrib++ = EGL_NONE;
  *write_attrib++ = EGL_NONE;

  EGLContext egl_context =
      eglCreateContext(kEglDisplay, egl_config[selected_config], NULL, attrib);
  if (!egl_context) {
    return 0;
  }

  // X11 window create
  Visual* visual = DefaultVisual(kDisplay, screen);
  s32 depth = DefaultDepth(kDisplay, screen);

  s32 colormap = XCreateColormap(kDisplay, root_window, visual, AllocNone);

  XSetWindowAttributes wa;
  const unsigned long wamask = CWBorderPixel | CWColormap | CWEventMask;

  wa.colormap = colormap;
  wa.border_pixel = 0;
  wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                  PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                  ExposureMask | FocusChangeMask | VisibilityChangeMask |
                  EnterWindowMask | LeaveWindowMask | PropertyChangeMask;
  s32 border_width = 0;
  s32 x = 100, y = 100;
  Window parent = root_window;
  kWindowId = XCreateWindow(kDisplay, parent, x, y, width, height, border_width,
                            depth, InputOutput, CopyFromParent, wamask, &wa);

  if (!kWindowId) {
    return 0;
  }

  XMapWindow(kDisplay, kWindowId);
  XStoreName(kDisplay, kWindowId, name);

  if (fullscreen) {
    XSizeHints* sizehints = XAllocSizeHints();
    long flags = 0;
    XGetWMNormalHints(kDisplay, kWindowId, sizehints, &flags);
    sizehints->flags &= ~(PMinSize | PMaxSize);
    XSetWMNormalHints(kDisplay, kWindowId, sizehints);
    XFree(sizehints);

#define _NET_WM_STATE_ADD 1l
#define GET_ATOM(X) Atom X = XInternAtom(kDisplay, #X, False)
    GET_ATOM(_NET_WM_STATE);
    GET_ATOM(_NET_WM_STATE_FULLSCREEN);

    XEvent e = {};
    e.xany.type = ClientMessage;
    e.xclient.message_type = _NET_WM_STATE;
    e.xclient.format = 32;
    e.xclient.window = kWindowId;
    e.xclient.data.l[0] = _NET_WM_STATE_ADD;
    e.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
    e.xclient.data.l[3] = 0l;

    XSendEvent(kDisplay, RootWindow(kDisplay, screen), 0,
               SubstructureNotifyMask | SubstructureRedirectMask, &e);
  } else {
    GET_ATOM(_MOTIF_WM_HINTS);
    const s32 border = 0;
    struct {
      unsigned long flags;
      unsigned long functions;
      unsigned long decorations;
      long input_mode;
      unsigned long status;
    } MWMHints = {(1L << 1), 0, border ? 1 : 0, 0, 0};

    XChangeProperty(kDisplay, kWindowId, _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32,
                    PropModeReplace, (unsigned char*)&MWMHints,
                    sizeof(MWMHints) / sizeof(long));
  }

  attrib[0] = EGL_NONE;
  attrib[1] = EGL_NONE;
  kEglSurface = eglCreateWindowSurface(kEglDisplay, egl_config[selected_config],
                                       kWindowId, attrib);
  if (!kEglSurface) {
    return 0;
  }

  EGLint surface_width, surface_height;
  eglQuerySurface(kEglDisplay, kEglSurface, EGL_HEIGHT, &surface_height);
  eglQuerySurface(kEglDisplay, kEglSurface, EGL_WIDTH, &surface_width);
  kWidthPixels = surface_width;
  kHeightPixels = surface_height;

  XSetErrorHandler(x11_error_handler);
  XSetIOErrorHandler(x11_ioerror_handler);
  eglMakeCurrent(kEglDisplay, kEglSurface, kEglSurface, egl_context);

  if (eglSwapInterval(kEglDisplay, 0) == EGL_FALSE) {
    return 0;
  }

  glViewport(0, 0, surface_width, surface_height);

  return 1;
}

s32 Create(const char* name, const CreateInfo& create_info) {
  return Create(name, create_info.window_width, create_info.window_height,
                create_info.fullscreen);
}

void SwapBuffers() {
  eglSwapBuffers(kEglDisplay, kEglSurface);
}

b8 PollEvent(PlatformEvent* event) {
  event->type = NOT_IMPLEMENTED;
  event->key = 0;
  event->position = v2f(0.f, 0.f);

  XEvent xev;
  while (XCheckWindowEvent(kDisplay, kWindowId, -1, &xev)) {
    switch (xev.type) {
      default:
        // Discard unhandled events
        continue;
      case MotionNotify: {
        kMouseX = xev.xmotion.x;
        kMouseY = xev.xmotion.y;
        event->type = MOUSE_MOVE;
        event->position = {kMouseX, kMouseY};
      } break;
      case KeyPress: {
        event->type = KEY_DOWN;
        event->key = XLookupKeysym(&xev.xkey, 0);
        event->position = {xev.xkey.x, xev.xkey.y};
      } break;
      case KeyRelease: {
        event->type = KEY_UP;
        event->key = XLookupKeysym(&xev.xkey, 0);
        event->position = {xev.xkey.x, xev.xkey.y};
      } break;
      case ButtonPress: {
        switch (xev.xbutton.button) {
          case 1:
          case 2:
          case 3:
            event->type = MOUSE_DOWN;
            event->button = (PlatformButton)xev.xbutton.button;
            break;
          case 4:
            event->type = MOUSE_WHEEL;
            event->wheel_delta = 1.0f;
            break;
          case 5:
            event->type = MOUSE_WHEEL;
            event->wheel_delta = -1.0f;
            break;
        }
        event->position = {xev.xbutton.x, xev.xbutton.y};
      } break;
      case ButtonRelease: {
        event->type = MOUSE_UP;
        event->button = (PlatformButton)xev.xbutton.button;
        event->position = {xev.xbutton.x, xev.xbutton.y};
      } break;
    }

    event->position.y = kHeightPixels - event->position.y;
    return true;
  }

  return false;
}

v2f GetWindowSize() {
  return v2f(kWidthPixels, kHeightPixels);
}

v2f GetPrimaryMonitorSize() {
  // TODO:
  return {};
}

v2f GetCursorPosition() {
  return v2f(kMouseX, kMouseY);
}

b8 ShouldClose() {
  return false;
}

const char* GetBinaryPath() {
  return "";
}

}  // namespace window
