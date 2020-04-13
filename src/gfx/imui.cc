#pragma once

#include "renderer/renderer.cc"

#include "../common/common.cc"

EXTERN(unsigned imui_errno);

namespace imui
{
constexpr int kMaxTextSize = 128;
constexpr int kClickForFrames = 100;

static const v4f kWhite(1.f, 1.f, 1.f, 1.f);
static const v4f kPaneColor(0.0f, 0.0f, 0.0f, 0.4f);

struct Result {
  Result() = default;
  Result(const Rectf& rect, bool highlighted, bool clicked) :
      rect(rect), highlighted(highlighted), clicked(clicked) {}
  Rectf rect;
  bool highlighted = false;
  bool clicked = false;
};

#define IMUI_RESULT(rect) \
  Result(rect, IsRectHighlighted(rect), IsRectClicked(rect))

struct TextOptions {
  v4f color = kWhite;
  v4f highlight_color = v4f();
  float scale = 1.0f;
};

struct PaneOptions {
  enum SizeMode {
    kAutoResize,
    kFixedSize,
  };
  PaneOptions() = default;
  PaneOptions(float width, float height)
      : size_mode(kFixedSize), width(width), height(height)
  {
  }

  SizeMode size_mode = kAutoResize;
  float width = 0.f;
  float height = 0.f;
};

struct Text {
  char msg[kMaxTextSize];
  v2f pos;
  v4f color;  // TODO: This is duplicated in TextOptions
  Rectf rect;
  TextOptions options;
};

struct Pane {
  Rectf rect;
  PaneOptions options;
};

struct UIClick {
  v2f pos;
  PlatformButton button;
};

struct MousePosition {
  v2f pos;
};

struct UIClickRender {
  v2f pos;
  int render_frames;
};

struct Button {
  Rectf rect;
  v4f color;
};

struct BeginMode {
  v2f pos;
  bool set = false;
  int text_calls = 0;
  uint32_t tag = 0;
  Pane* pane;
};

struct IMUI {
  BeginMode begin_mode;
};

struct Box {
  Rectf rect;
  v4f color;
  v4f outline_color;
};

static IMUI kIMUI;

constexpr uint32_t kMaxTags = MAX_PLAYER + 1;
constexpr uint32_t kEveryoneTag = MAX_PLAYER;

DECLARE_2D_ARRAY(Text, kMaxTags, 64);
DECLARE_2D_ARRAY(Button, kMaxTags, 16);
DECLARE_2D_ARRAY(UIClick, kMaxTags, 8);
DECLARE_2D_ARRAY(MousePosition, kMaxTags, MAX_PLAYER);
DECLARE_2D_ARRAY(Pane, kMaxTags, 8);
DECLARE_QUEUE(UIClickRender, 8);

void
ResetAll()
{
  memset(kUsedText, 0, sizeof(kUsedText));
  memset(kUsedButton, 0, sizeof(kUsedButton));
  memset(kUsedUIClick, 0, sizeof(kUsedUIClick));
  memset(kUsedMousePosition, 0, sizeof(kUsedMousePosition));
  memset(kUsedPane, 0, sizeof(kUsedPane));
}

void
ResetTag(uint32_t tag)
{
  assert(tag < kMaxTags);
  kUsedText[tag] = 0;
  kUsedButton[tag] = 0;
  kUsedPane[tag] = 0;
  kUsedUIClick[tag] = 0;
  kUsedMousePosition[tag] = 0;
}

void
Render(uint32_t tag)
{
  glDisable(GL_DEPTH_TEST);
  auto dims = window::GetWindowSize();
  rgg::ModifyObserver mod(math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f),
                          math::Identity());

  for (int i = 0; i < kUsedPane[tag]; ++i) {
    Pane* pane = &kPane[tag][i];
    rgg::RenderRectangle(pane->rect, kPaneColor);
  }

  for (int i = 0; i < kUsedButton[tag]; ++i) {
    Button* button = &kButton[tag][i];
    rgg::RenderButton("test", button->rect, button->color);
  }

  for (int i = 0; i < kUsedText[tag]; ++i) {
    Text* text = &kText[tag][i];
    rgg::RenderText(text->msg, text->pos, text->options.scale, text->color);
  }

  for (int i = kReadUIClickRender; i < kWriteUIClickRender; ++i) {
    UIClickRender* render = &kUIClickRender[i % kMaxUIClickRender];
    rgg::RenderCircle(
        render->pos, 3.5f,
        v4f(1.f, 0.f, 0.65f, (float)render->render_frames / kClickForFrames));
    --render->render_frames;
    if (!render->render_frames) {
      PopUIClickRender();
    }
  }
  glEnable(GL_DEPTH_TEST);
}

bool
IsRectHighlighted(Rectf rect)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedMousePosition[tag]; ++i) {
    MousePosition* mp = &kMousePosition[tag][i];
    if (math::PointInRect(mp->pos, rect)) return true;
  }
  return false;
}

bool
IsRectClicked(Rectf rect)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedUIClick[tag]; ++i) {
    UIClick* click = &kUIClick[tag][i];
    if (math::PointInRect(click->pos, rect)) return true;
  }
  return false;
}

void
Indent(int spaces)
{
  assert(kIMUI.begin_mode.set);
  auto& font = rgg::kUI.font;
  rgg::FontMetadataRow* row = &rgg::kFontMetadataRow[' '];
  if (!row || !row->id) return;
  kIMUI.begin_mode.pos.x += spaces * row->xadvance;
}

// Returns rect representing bounds for the current object.
// Updates global bounds of pane.
// Set begin_mode.pos to point to the bottom left of where the current element
// should draw.
Rectf
UpdatePane(float width, float height, Pane* pane)
{
  assert(pane);
  auto& begin_mode = kIMUI.begin_mode;
  // Must call Begin() before UI rendering starts and End() when it's done.
  assert(begin_mode.set);
  begin_mode.pane->rect.y -= height;
  begin_mode.pane->rect.height += height;
  float new_width = (begin_mode.pos.x + width) - begin_mode.pane->rect.x;
  if (new_width > begin_mode.pane->rect.width) {
    begin_mode.pane->rect.width = new_width;
  }
  begin_mode.pos.y -= height;
  return Rectf(begin_mode.pos.x, begin_mode.pos.y, width, height);
}

void
UpdatePaneOnEnd(Pane* pane)
{
  if (!pane) return;
  auto& begin_mode = kIMUI.begin_mode;
  uint32_t tag = begin_mode.tag;
  assert(begin_mode.set);
  switch (pane->options.size_mode) {
    case PaneOptions::kAutoResize:
      break;
    case PaneOptions::kFixedSize: {
      int start = kUsedText[tag] - 1;
      int end = kUsedText[tag] - begin_mode.text_calls;
      assert(start < kMaxText);
      assert(end >= 0);
      int move_text = begin_mode.text_calls - 1;
      for (; start >= end; --start) {
        struct Text* text = &kText[tag][start];
        text->pos.y += text->rect.height * move_text;
        v2f text_top_left = text->pos + v2f(0.f, text->rect.height);
        // Discard the text element if it is outside of the pane.
        if (!math::PointInRect(text_top_left, pane->rect)) {
          CompressText(tag, start);
          continue;
        }
      }
    } break;
    default:
      break;
  }
}

Result
Text(const char* msg, TextOptions options)
{
  assert(kIMUI.begin_mode.set);
  auto& begin_mode = kIMUI.begin_mode;
  uint32_t tag = kIMUI.begin_mode.tag;
  struct Text* text = UseText(tag);
  Result data;
  if (!text) {
    imui_errno = 1;
    return data;
  }
  if (strlen(msg) > kMaxTextSize) {
    imui_errno = 2;
    return data;
  }
  Rectf text_rect = 
      rgg::GetTextRect(msg, strlen(msg), begin_mode.pos, options.scale);
  Rectf rect =
      UpdatePane(text_rect.width, text_rect.height, begin_mode.pane);
  strcpy(text->msg, msg);
  text->pos = v2f(rect.x, rect.y);
  text->color = options.color;
  if (IsRectHighlighted(rect) && options.highlight_color != v4f()) {
    text->color = options.highlight_color;
  }
  text->options = options;
  text->rect = rect;
  ++begin_mode.text_calls;
  return IMUI_RESULT(rect);
}

Result
Text(const char* msg)
{
  assert(kIMUI.begin_mode.set);
  return Text(msg, {kWhite, kWhite});
}

void
Begin(v2f start, uint32_t tag, const PaneOptions& pane_options)
{
  assert(tag < kMaxTags);
  auto& begin_mode = kIMUI.begin_mode;
  // End must be called before Begin.
  assert(!begin_mode.set);
  begin_mode.pos = start;
  begin_mode.set = true;
  begin_mode.text_calls = 0;
  begin_mode.tag = tag;
  begin_mode.pane = UsePane(tag);
  begin_mode.pane->rect.x = start.x;
  begin_mode.pane->rect.y = start.y;
  begin_mode.pane->rect.width = pane_options.width;
  begin_mode.pane->rect.height = pane_options.height;
  begin_mode.pane->options = pane_options;
}

void
Begin(v2f start, uint32_t tag)
{
  Begin(start, tag, PaneOptions());
}

void
End()
{
  UpdatePaneOnEnd(kIMUI.begin_mode.pane);
  kIMUI.begin_mode.set = false;
}

Result
Button(float width, float height, const v4f& color)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  uint32_t tag = kIMUI.begin_mode.tag;
  struct Button* button = UseButton(tag);
  Result result;
  if (!button) {
    imui_errno = 3;
    return result;
  }
  Rectf rect = UpdatePane(width, height, kIMUI.begin_mode.pane);
  button->rect = rect;
  button->color = color;
  return IMUI_RESULT(button->rect);
}

void
MouseClick(v2f pos, PlatformButton b, uint32_t tag)
{
  UIClick* click = UseUIClick(tag);
  if (!click) {
    imui_errno = 4;
    return;
  }
  click->pos = pos;
  click->button = b;
}

void
MousePosition(v2f pos, uint32_t tag)
{
  struct MousePosition* mp = UseMousePosition(tag);
  if (!mp) {
    imui_errno = 4;
    return;
  }
  mp->pos = pos;
}

const char*
LastErrorString()
{
  unsigned err = imui_errno;
  imui_errno = 0;
  switch (err) {
    case 1:
      return ("imui text count exhausted.");
    case 2:
      return ("text provided surpasses max allowed imui character count.");
    case 3:
      return ("imui button count exhausted.");
    case 4:
      return ("imui click count exhausted.");
  };

  return 0;
}

}  // namespace imui
