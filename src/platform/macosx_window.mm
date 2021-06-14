#include "window.h"

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>

@interface OpenGLView : NSView
- (id) init;
- (BOOL) isOpaque;
- (BOOL) canBecomeKeyView;
- (BOOL) acceptsFirstResponder;
@end

@implementation OpenGLView
- (id)
init
{
  return nil;
}

- (BOOL)
isOpaque
{
  return YES;
}

- (BOOL)
canBecomeKeyView
{
  return YES;
}

- (BOOL)
acceptsFirstResponder
{
  return YES;
}

- (void)
keyDown:(NSEvent*)theEvent
{
  // Capture this event and do nothing with it because the default behavior
  // is to make some macos specific noise that indicates the user did something
  // incorrectly - or the app performed no action. It kinda sounds like <bloop>.
}

@end

@interface BaseWindow : NSWindow
- (void) close;

- (BOOL) acceptsFirstResponder;
@end

@implementation BaseWindow
// Close the window - https://developer.apple.com/documentation/appkit/nswindow/1419662-close?language=objc
// This also terminates the NSApplication 
- (void)
close
{
  [NSApp terminate:self];
  // If the app refused to terminate, this window should still close.
  [super close];
}

// The receiver is the first object in the responder chain to be sent
// key events and action messages.
// https://developer.apple.com/documentation/appkit/nsresponder/1528708-acceptsfirstresponder?language=objc
- (BOOL)
acceptsFirstResponder
{
  return YES;
}

@end

namespace window
{

struct Window {
  NSView* nsview;

  NSWindow* nswindow;

  NSOpenGLContext* gl_context;
};

static Window kWindow;

NSOpenGLContext*
CreateOpenGLContext()
{
  CGLError error;
  CGLPixelFormatAttribute pixel_attrs[] = {
      kCGLPFADoubleBuffer,
      kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
      kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
      kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
      kCGLPFADepthSize, (CGLPixelFormatAttribute)24,
      kCGLPFAStencilSize, (CGLPixelFormatAttribute)8,
      kCGLPFASampleBuffers, (CGLPixelFormatAttribute)1,
      kCGLPFASamples, (CGLPixelFormatAttribute)4,
      kCGLPFANoRecovery, (CGLPixelFormatAttribute)1,
      kCGLPFAMultisample, (CGLPixelFormatAttribute)1,
      (CGLPixelFormatAttribute) 0,
  };

  s32 ignore;
  CGLPixelFormatObj pixel_format;
  error = CGLChoosePixelFormat(pixel_attrs, &pixel_format, &ignore);
  assert(!error);
  assert(pixel_format);

  // Create the GL context.
  CGLContextObj context;
  error = CGLCreateContext(pixel_format, 0, &context);
  assert(!error);
  assert(context);

  // Disable vsync
  GLint sync = 0;
  CGLSetParameter(context, kCGLCPSwapInterval, &sync);

  return [[NSOpenGLContext alloc] initWithCGLContextObj:context];
}


s32
Create(const char* name, const CreateInfo& create_info)
{
  kWindow.gl_context = CreateOpenGLContext();

  u32 style_mask = NSTitledWindowMask   |
                   NSClosableWindowMask |
                   NSWindowStyleMaskResizable;
  kWindow.nsview = [[OpenGLView alloc]
                  initWithFrame:NSMakeRect(0, 0, create_info.window_width, create_info.window_height)];

  kWindow.nswindow = [[BaseWindow alloc]
                      initWithContentRect:[kWindow.nsview frame]
                                styleMask:style_mask
                                  backing:NSBackingStoreBuffered
                                    defer:NO];

  [kWindow.nswindow autorelease];
  [kWindow.nswindow setTitle:[NSString stringWithUTF8String:name]];
  [kWindow.nswindow setContentView:kWindow.nsview];
  [kWindow.nswindow makeFirstResponder:kWindow.nsview];
  if (create_info.window_pos_x != UINT64_MAX && create_info.window_pos_y != UINT64_MAX) {
    [kWindow.nswindow setFrame:CGRectMake(create_info.window_pos_x, create_info.window_pos_y, [kWindow.nswindow frame].size.width , [kWindow.nswindow frame].size.height) display:YES];
  } else {
    [kWindow.nswindow center];
  }
  [kWindow.nswindow makeKeyAndOrderFront:nil];
  [kWindow.nswindow setAcceptsMouseMovedEvents:YES];
  if (create_info.fullscreen) {
    [kWindow.nswindow
        setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [kWindow.nswindow toggleFullScreen:nil];
    [NSMenu setMenuBarVisible:NO];
  }

  [kWindow.gl_context makeCurrentContext];
  // https://developer.apple.com/documentation/appkit/nsview/1414938-wantsbestresolutionopenglsurface?language=objc
  [kWindow.nsview setWantsBestResolutionOpenGLSurface:NO];
  [kWindow.gl_context setView:kWindow.nsview];

  // No idea where these constants come from or why this isn't the default but
  // this is required to allow the application to come to the foreground and
  // receive key events.
  ProcessSerialNumber psn = {0, kCurrentProcess};
  OSStatus status =
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);

  // Bring window to forefront.
  NSApplication* app = [NSApplication sharedApplication];
  [app activateIgnoringOtherApps:YES];

  return 1;
}

//s32
//Create(const char* name, const CreateInfo& create_info)
//{
//  printf("Creating window %i %i\n", create_info.window_width, create_info.window_height);
//  return Create(name, create_info.window_width, create_info.window_height,
//                create_info.fullscreen);
//}

void
SetEventPosition(NSEvent* nsevent, PlatformEvent* event)
{
  NSPoint pos = [nsevent locationInWindow];
  event->position.x = pos.x;
  event->position.y = pos.y;
}

void
TranslateEvent(NSEvent* nsevent, PlatformEvent* event)
{
  NSEventType nsevent_type = [nsevent type];
  switch (nsevent_type) {
    case NSEventTypeLeftMouseDown: {
      event->type = MOUSE_DOWN;
      event->button = BUTTON_LEFT;
      SetEventPosition(nsevent, event);
    } break;
    case NSEventTypeLeftMouseUp: {
      event->type = MOUSE_UP;
      event->button = BUTTON_LEFT;
      SetEventPosition(nsevent, event);
    } break;
    case NSEventTypeRightMouseDown: {
      event->type = MOUSE_DOWN;
      event->button = BUTTON_RIGHT;
      SetEventPosition(nsevent, event);
    } break;
    case NSEventTypeRightMouseUp: {
      event->type = MOUSE_UP;
      event->button = BUTTON_RIGHT;
      SetEventPosition(nsevent, event);
    } break;
    case NSEventTypeScrollWheel: {
      event->type = MOUSE_WHEEL;
      event->wheel_delta = [nsevent deltaY];
    } break;
    case NSEventTypeKeyDown: {
      event->type = KEY_DOWN;
      // TODO: Unfortunately this indicates an event can be associated with
      // multiple characters. I think an Event system that only has at most
      // one character event per keyboard press makes more sense. It would be
      // good to expand these to multiple events or change the Event api to
      // accomdate multiple characters in one event.
      //
      // Or maybe this is not that big a deal. I wonder how often a single
      // nsevent can have multiple key presses.
      NSString* characters = [nsevent charactersIgnoringModifiers];
      event->key = [characters characterAtIndex:0];
    } break;
    case NSEventTypeKeyUp: {
      event->type = KEY_UP;
      NSString* characters = [nsevent charactersIgnoringModifiers];
      event->key = [characters characterAtIndex:0];
    } break;
    default:
      break;
  }
}

b8
PollEvent(PlatformEvent* event)
{
  event->type = NOT_IMPLEMENTED;
  event->key = 0;
  event->position = v2f(0.f, 0.f);

  NSEvent* nsevent = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
  if (!nsevent) return false;
  // Convert the NSEvent* to an Event*
  TranslateEvent(nsevent, event);
  // Send the NSEvent nsevent to the app so it can handle it.
  [NSApp sendEvent:nsevent];
  return true;
}

void
SwapBuffers()
{
  [kWindow.gl_context flushBuffer];
}

b8
ShouldClose()
{
  return false;
}

v2f
GetWindowSize()
{
  NSRect frame = [kWindow.nsview frame];
  return v2f(frame.size.width, frame.size.height);
}

v2f
GetPrimaryMonitorSize()
{
  // TODO:
  return {};
}

v2f
GetCursorPosition()
{
  NSPoint pos;
  pos = [kWindow.nswindow mouseLocationOutsideOfEventStream];
  // Change origin of screen to be top left to be consistent with other platforms.
  return v2f(pos.x, pos.y);
}

void
SetCursorPosition(v2f pos)
{
  CGPoint pt;
  pt.x = pos.x;
  pt.y = pos.y;
  CGWarpMouseCursorPosition(pt);
}

const char*
GetBinaryPath()
{
  return "";
}
}  // namespace window
