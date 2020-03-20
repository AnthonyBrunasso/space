#include "window.h"

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>

@interface OpenGLView : NSView {
@private
  NSOpenGLContext* gl_context_;
}

  //@property (readonly, retain) NSOpenGLContext* gl_context;
- (id) init;
- (id) initWithFrame:(NSRect)rect glContext:(NSOpenGLContext*)ctx;
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

- (id)
initWithFrame:(NSRect)rect glContext:(NSOpenGLContext*)ctx
{
  self = [super initWithFrame:rect];
  if (self == nil)
    return nil;
  gl_context_ = ctx;
  return self;
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
  // TODO: Verify this is needed. Likely doesn't matter though since
  // few allocations should be going on in here.
  NSAutoreleasePool* nspool;

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
      kCGLPFASampleBuffers, (CGLPixelFormatAttribute)0,
      (CGLPixelFormatAttribute) 0,
  };

  int ignore;
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


int
Create(const char* name, int width, int height, bool fullscreen)
{
  kWindow.gl_context = CreateOpenGLContext();

  unsigned int style_mask = NSTitledWindowMask   |
                            NSClosableWindowMask |
                            NSWindowStyleMaskResizable;
  kWindow.nsview = [[OpenGLView alloc]
                  initWithFrame:NSMakeRect(0,0, width, height)
                      glContext:kWindow.gl_context]; // TODO: Remove context.

  kWindow.nswindow = [[BaseWindow alloc]
                      initWithContentRect:[kWindow.nsview frame]
                                styleMask:style_mask
                                  backing:NSBackingStoreBuffered
                                    defer:NO];

  [kWindow.nswindow autorelease];
  [kWindow.nswindow setTitle:[NSString stringWithUTF8String:name]];
  [kWindow.nswindow setContentView:kWindow.nsview];
  [kWindow.nswindow makeFirstResponder:kWindow.nsview];
  [kWindow.nswindow center];
  [kWindow.nswindow makeKeyAndOrderFront:nil];
  [kWindow.nswindow setAcceptsMouseMovedEvents:YES];

  [kWindow.gl_context makeCurrentContext];
  [kWindow.gl_context setView:kWindow.nsview];

  // No idea where these constants come from or why this isn't the default but
  // this is required to allow the application to come to the foreground and
  // receive key events.
  ProcessSerialNumber psn = {0, kCurrentProcess};
  OSStatus status =
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
  return 1;
}

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

bool
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

bool
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
GetCursorPosition()
{
  NSPoint pos;
  pos = [kWindow.nswindow mouseLocationOutsideOfEventStream];
  // Change origin of screen to be top left to be consistent with other platforms.
  return v2f(pos.x, pos.y);
}
}  // namespace window
