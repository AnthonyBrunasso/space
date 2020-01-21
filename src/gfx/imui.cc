#pragma once

#include "renderer/renderer.cc"

#include "../common/array.cc"

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

struct UIClick {
  math::Vec2f pos;
};

DECLARE_ARRAY(Text, kMaxTextCount);
DECLARE_ARRAY(UIClick, kMaxFrameClick);

void
Reset()
{
  kUsedText = 0;
  kUsedUIClick = 0;
}

void
Render()
{
  rgg::RenderButton("test", math::Rect(10, 10, 40, 40),
                    math::Vec4f(1.0f, 1.0f, 1.0f, 0.5f));

  rgg::RenderButton("test", math::Rect(55, 10, 40, 40),
                    math::Vec4f(1.0f, 1.0f, 1.0f, 0.5f));

  // Draw all text.
  for (int i = 0; i < kUsedText; ++i) {
    Text* text = &kText[i];
    rgg::RenderText(text->msg, text->screen_x, text->screen_y, math::Vec4f());
  }
}

void
Text(const char* msg, float screen_x, float screen_y)
{
  struct Text* text = UseText();
  if (!text) {
    printf("imui text count exhausted.\n");
    return;
  }
  strcpy(text->msg, msg);
  text->screen_x = screen_x;
  text->screen_y = screen_y;
}

void
MouseClick(math::Vec2f pos)
{
  UIClick* click = UseUIClick();
  if (!click) {
    printf("imui click count exhausted.\n");
    return;
  }
  click->pos = pos;
}

}
