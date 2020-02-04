#pragma once

#include "renderer/renderer.cc"

#include "../common/array.cc"
#include "../common/queue.cc"

namespace imui
{
constexpr int kMaxTextSize = 128;
constexpr int kClickForFrames = 100;

static const v4f kWhite(1.f, 1.f, 1.f, 1.f);
static const v4f kPaneColor(0.f, 0.f, 0.f, 0.5f);

struct Result {
  math::Rect rect;
  bool highlighted = false;
  bool clicked = false;
};

struct TextOptions {
  v4f color = kWhite;
  v4f highlight_color = v4f();
};

struct PaneOptions {
  enum Settings {
    kNone = 0x0,
    kFixedSize = 0x1,
    kAutoResize = 0x2,
  };
  Settings settings = kNone;
  float width = 0.f;
  float height = 0.f;
};

struct Text {
  char msg[kMaxTextSize];
  v2f pos;
  v4f color;
};

struct Pane {
  math::Rect rect;
  PaneOptions options;
};

struct UIClick {
  v2f pos;
};

struct UIClickRender {
  v2f pos;
  int render_frames;
};

struct Button {
  math::Rect rect;
  v4f color;
};

struct BeginMode {
  v2f pos;
  bool set;
  Pane* pane;
};

struct IMUI {
  BeginMode begin_mode;
};

static IMUI kIMUI;

DECLARE_ARRAY(Text, 32);
DECLARE_ARRAY(Button, 16);
DECLARE_ARRAY(UIClick, 8);
DECLARE_ARRAY(Pane, 8);
DECLARE_QUEUE(UIClickRender, 8);

void
Reset()
{
  kUsedText = 0;
  kUsedButton = 0;
  kUsedUIClick = 0;
  kUsedPane = 0;
}

void
Render()
{
  auto dims = window::GetWindowSize();
  rgg::ModifyObserver mod(
      math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f), math::Identity());

  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    rgg::RenderRectangle(pane->rect, kPaneColor);
  }

  for (int i = 0; i < kUsedButton; ++i) {
    Button* button = &kButton[i];
    rgg::RenderButton("test", button->rect, button->color);
  }

  for (int i = 0; i < kUsedText; ++i) {
    Text* text = &kText[i];
    rgg::RenderText(text->msg, text->pos, text->color);
  }

  for (int i = kReadUIClickRender; i < kWriteUIClickRender; ++i) {
    UIClickRender* render = &kUIClickRender[i % kMaxUIClickRender];
    rgg::RenderCircle(render->pos, 3.5f,
                      v4f(1.f, 0.f, 0.65f,
                          (float)render->render_frames / kClickForFrames));
    --render->render_frames;
    if (!render->render_frames) {
      PopUIClickRender();
    }
  }
}

bool
IsRectHighlighted(math::Rect rect)
{
  return math::PointInRect(window::GetCursorPosition(), rect);
}

bool
IsRectClicked(math::Rect rect)
{
  for (int i = 0; i < kUsedUIClick; ++i) {
    UIClick* click = &kUIClick[i];
    if (math::PointInRect(click->pos, rect)) return true;
  }
  return false;
}

void
Indent(int spaces)
{
  auto& font = rgg::kUI.font;
  FntMetadataRow* row = &font.metadata.rows[' '];
  if (!row) return;
  kIMUI.begin_mode.pos.x += spaces * row->xadvance;
}

Result
Text(const char* msg, v2f pos, TextOptions options)
{
  Result data;
  struct Text* text = UseText();
  if (!text) {
    printf("imui text count exhausted.\n");
    return data;
  }
  if (strlen(msg) > kMaxTextSize) {
    printf("text provided surpasses max allowed imui character count.\n");
    return data;
  }
  auto& begin_mode = kIMUI.begin_mode;
  data.rect = rgg::GetTextRect(msg, strlen(msg), begin_mode.pos);
  data.highlighted = IsRectHighlighted(data.rect);
  data.clicked = IsRectClicked(data.rect);
  strcpy(text->msg, msg);
  text->pos = pos;
  text->color = options.color;
  if (data.highlighted && options.highlight_color != v4f()) {
    text->color = options.highlight_color;
  }
  return data;
}

Result
Text(const char* msg, v2f pos)
{
  return Text(msg, pos, {kWhite, kWhite});
}

void
Begin(v2f start, const PaneOptions& pane_options)
{
  auto& begin_mode = kIMUI.begin_mode;
  // End must be called before Begin.
  assert(!begin_mode.set);
  begin_mode.pos = start;
  begin_mode.set = true; 
  begin_mode.pane = UsePane();
  begin_mode.pane->rect.x = start.x;
  begin_mode.pane->rect.y = start.y;
  begin_mode.pane->rect.width = pane_options.width;
  begin_mode.pane->rect.height = pane_options.height;
  begin_mode.pane->options = pane_options;
}

void
Begin(v2f start)
{
  auto& begin_mode = kIMUI.begin_mode;
  // End must be called before Begin.
  assert(!begin_mode.set);
  begin_mode.pos = start;
  begin_mode.set = true; 
  begin_mode.pane = nullptr;
}

void
UpdatePane(const math::Rect& rect, Pane* pane)
{
  if (!pane) return;
  switch (pane->options.settings) {
  case PaneOptions::kNone: break;
  case PaneOptions::kFixedSize: break;
  case PaneOptions::kAutoResize: {
    auto& begin_mode = kIMUI.begin_mode;
    begin_mode.pane->rect.y = begin_mode.pos.y;
    begin_mode.pane->rect.height += rect.height;
    float width = (rect.x + rect.width) - begin_mode.pane->rect.x;
    if (width > begin_mode.pane->rect.width) {
      begin_mode.pane->rect.width = width;
    }
  } break;
  default: break;
  }
}

Result
Text(const char* msg, TextOptions options)
{
  auto& begin_mode = kIMUI.begin_mode;
  // Call StartText before this.
  assert(kIMUI.begin_mode.set);
  Result data = Text(msg, begin_mode.pos, options);
  UpdatePane(data.rect, begin_mode.pane);
  begin_mode.pos.y -= data.rect.height;
  return data;
}

Result
Text(const char* msg)
{
  return Text(msg, {kWhite, kWhite});
}

void
End()
{
  kIMUI.begin_mode.set = false; 
}

Result
Button(const math::Rect& rect, const v4f& color)
{
  struct Button* button = UseButton();
  Result result;
  if (!button) {
    printf("imui button count exhausted.\n");
    return result;
  }
  button->rect = rect;
  button->color = color;
  result.clicked = IsRectClicked(rect);
  result.highlighted = IsRectHighlighted(rect);
  result.rect = rect;
  return result;
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
  PushUIClickRender({pos, kClickForFrames});
}
}
