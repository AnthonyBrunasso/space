#pragma once

#include "renderer/renderer.cc"

namespace imui
{
constexpr int kMaxTextSize = 128;
constexpr int kMaxTextCount = 32;
constexpr int kMaxFrameClick = 8;

struct Text {
  char msg[kMaxTextSize];
  float screen_x;
  float screen_y;
};

struct IMUI {
  Text text[kMaxTextCount];
  int text_count;
  math::Vec2f frame_click[kMaxFrameClick];
  int frame_click_count;
};

static IMUI kIMUI;

void
Reset()
{
  kIMUI.text_count = 0;
  kIMUI.frame_click_count = 0;
}

void
Render()
{
  rgg::RenderButton("test", math::Rect(10, 10, 40, 40),
                    math::Vec4f(1.0f, 1.0f, 1.0f, 0.5f));

  rgg::RenderButton("test", math::Rect(55, 10, 40, 40),
                    math::Vec4f(1.0f, 1.0f, 1.0f, 0.5f));

  // Draw all text.
  for (int i = 0; i < kIMUI.text_count; ++i) {
    Text& text = kIMUI.text[i];
    rgg::RenderText(text.msg, text.screen_x, text.screen_y, math::Vec4f());
  }
}

void
Text(const char* msg, float screen_x, float screen_y)
{
  assert(kIMUI.text_count + 1 < kMaxTextCount);
  if (kIMUI.text_count + 1 >= kMaxTextCount) return;
  int len = strlen(msg);
  if (len > kMaxTextSize) return;
  struct Text& text = kIMUI.text[kIMUI.text_count++];
  strcpy(text.msg, msg);
  text.screen_x = screen_x;
  text.screen_y = screen_y;
}

void
MouseClick(math::Vec2f pos)
{
  if (kIMUI.frame_click_count + 1 >= kMaxFrameClick) return;
  kIMUI.frame_click[kIMUI.frame_click_count++] = pos;
}

}
