#pragma once

#include "renderer/renderer.cc"

#include "../common/array.cc"

namespace imui
{
constexpr int kMaxTextSize = 128;

struct Text {
  char msg[kMaxTextSize];
  float screen_x;
  float screen_y;
};

struct UIClick {
  v2f pos;
};

struct Button {
  math::Rect rect;
  v4f color;
};

DECLARE_ARRAY(Text, 32);
DECLARE_ARRAY(Button, 16);
DECLARE_ARRAY(UIClick, 8);

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
    rgg::RenderText(text->msg, text->screen_x, text->screen_y, v4f());
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
  if (strlen(msg) > kMaxTextSize) {
    printf("text provided surpasses max allowed imui character count.\n");
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
Button(const math::Rect& rect, const v4f& color)
{
  struct Button* button = UseButton();
  if (!button) {
    printf("imui button count exhausted.\n");
    return false;
  }
  button->rect = rect;
  button->color = color;
  return IsButtonClicked(rect);
}

void
MouseClick(v2f pos)
{
  UIClick* click = UseUIClick();
  if (!click) {
    printf("imui click count exhausted.\n");
    return;
  }
  click->pos = pos;
}

}
