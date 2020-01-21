#pragma once

#include "renderer/renderer.cc"

#include "../common/array.cc"

namespace imui
{
constexpr int kMaxTextSize = 128;
constexpr int kMaxTextCount = 32;
constexpr int kMaxButtonCount = 16;
constexpr int kMaxFrameClick = 8;

struct Text {
  char msg[kMaxTextSize];
  float screen_x;
  float screen_y;
};

struct UIClick {
  math::Vec2f pos;
};

struct Button {
  math::Rect rect;
  math::Vec4f color;
};

DECLARE_ARRAY(Text, kMaxTextCount);
DECLARE_ARRAY(Button, kMaxButtonCount);
DECLARE_ARRAY(UIClick, kMaxFrameClick);

void
Reset()
{
  kUsedText = 0;
  kUsedButton = 0;
  kUsedUIClick = 0;
}

void
Render()
{
  for (int i = 0; i < kUsedButton; ++i) {
    Button* button = &kButton[i];
    rgg::RenderButton("test", button->rect, button->color);
  }

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

bool
IsButtonClicked(const math::Rect& rect)
{
  for (int i = 0; i < kUsedUIClick; ++i) {
    UIClick* click = &kUIClick[i];
    if (math::PointInRect(click->pos, rect)) return true;
  }
  return false;
}

bool
Button(const math::Rect& rect, const math::Vec4f& color)
{
  struct Button* button = UseButton();
  if (!button) {
    printf("imui text count exhausted.\n");
    return false;
  }
  button->rect = rect;
  button->color = color;
  return IsButtonClicked(rect);
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
