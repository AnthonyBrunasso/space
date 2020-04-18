#pragma once

#include "renderer/renderer.cc"

#include "../common/common.cc"

EXTERN(unsigned imui_errno);

namespace imui
{

enum SpaceType {
  kHorizontal = 0,
  kVertical = 1,
};

enum FlowType {
  kNewLine,
  kSameLine,
};

constexpr int kMaxTextSize = 128;
constexpr int kClickForFrames = 100;

constexpr float kTextScale = 0.8f;
constexpr float kScrollBarWidth = 15.f;

static const v4f kWhite(1.f, 1.f, 1.f, 1.f);
static const v4f kPaneColor(0.0f, 0.0f, 0.0f, 0.4f);
static const v4f kPaneHeaderColor(0.12f, 0.16f, 0.154f, 1.f);
static const v4f kScrollColor(0.32f, 0.36f, 0.354f, .7f);
static const v4f kHeaderMinimizeColor(0.45f, 0.68f, 0.906f, 0.7f);

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

#define IMUI_RESULT_CIRCLE(center, radius) \
  Result(rect, IsCircleHighlighted(center, radius), IsCircleClicked(center, radius))

#define IF_HIDDEN(jmp) \
  if (kIMUI.begin_mode.show && !(*kIMUI.begin_mode.show)) jmp;

#define TITLE_WITH_TAG(title, tag)             \
  char title_with_tag[kMaxHashKeyLength] = {}; \
  char tag_append[2] = {};                     \
  snprintf(tag_append, 2, "%u", tag);          \
  strcat(title_with_tag, title);               \
  strcat(title_with_tag, tag_append);          \


struct TextOptions {
  v4f color = kWhite;
  v4f highlight_color = v4f();
  // If true the text will render regardless of clipping options.
  bool ignore_scissor_test = false;
};

struct ButtonCircleOptions {
  bool ignore_scissor_test = false;
};

struct PaneOptions {
  // Starting with and height of pane.
  float width = 0.f;
  float height = 0.f;
  // If set the pane will not expand these dimensions.
  float max_width = 0.f;
  float max_height = 0.f;
  v4f color = kPaneColor;
  Rectf header_rect;
};

// imui elements.

struct Pane {
  uint32_t tag;
  Rectf rect;
  PaneOptions options;
  // Now far up or down the user has scrolled.
  // 0 indicates no scroll.
  // A value greater than 0 means the user has scrolled down. All text ui
  // elements should be positioned accordingly.
  float vertical_scroll = 0.f;
  bool element_off_pane = false;
  Rectf header_rect;
  // The height the pane would be if constraints like max_height screen
  // clipping where done. This is used for scrolling.
  float theoretical_height = 0.f;
  bool hidden = false;
  bool debug_show_details = false;
  bool has_scroll_bar = false;
  Rectf scroll_rect;
  char title[kMaxHashKeyLength];
};

struct Text {
  char msg[kMaxTextSize];
  v2f pos;
  v4f color;  // TODO: This is duplicated in TextOptions
  Rectf rect;
  TextOptions options;
  Pane* pane;
};

struct Button {
  Rectf rect;
  v4f color;
  Pane* pane;
};

struct ButtonCircle {
  v2f position;
  float radius;
  v4f color;
  ButtonCircleOptions options;
  Pane* pane;
};

struct Line {
  v2f start;
  SpaceType type;
  v4f color;
  Pane* pane;
};

struct ProgressBar {
  Rectf rect;
  float current_progress;
  float max_progress;
  v4f fill_color;
  v4f outline_color;
  Pane* pane;
};

// imui metadata.

struct MouseDown {
  v2f pos;
  PlatformButton button;
};

struct MouseUp {
  v2f pos;
  PlatformButton button;
};

struct MouseWheel {
  float delta;
};

struct MousePosition {
  v2f pos;
};

struct LastMousePosition {
  v2f pos;
};

struct BeginMode {
  v2f pos;
  bool set = false;
  uint32_t tag = 0;
  // UI Element go one new line unless explcitly swapped.
  FlowType flow_type = kNewLine;
  bool flow_switch = false;
  Rectf last_rect;
  float x_reset;
  bool mouse_down;
  // A bit of a hack?
  bool ignore_vertical_scroll = false;
  float overwrite_width;
  bool* show = nullptr;
  v2f* start = nullptr;
  Pane* pane;
};

constexpr uint32_t kMaxTags = MAX_PLAYER + 1;
constexpr uint32_t kEveryoneTag = MAX_PLAYER;

struct IMUI {
  BeginMode begin_mode;
  bool mouse_down[kMaxTags];
  uint32_t text_exhaustion[kMaxTags];
  uint32_t button_exhaustion[kMaxTags];
  uint32_t button_circle_exhaustion[kMaxTags];
  bool debug_show_details[kMaxTags];
};

static IMUI kIMUI;

DECLARE_2D_ARRAY(Text, kMaxTags, 128);
DECLARE_2D_ARRAY(Line, kMaxTags, 32);
DECLARE_2D_ARRAY(Button, kMaxTags, 16);
DECLARE_2D_ARRAY(ButtonCircle, kMaxTags, 16);
DECLARE_2D_ARRAY(MouseDown, kMaxTags, 8);
DECLARE_2D_ARRAY(MouseUp, kMaxTags, 8);
DECLARE_2D_ARRAY(MouseWheel, kMaxTags, 8);
DECLARE_2D_ARRAY(ProgressBar, kMaxTags, 8);
DECLARE_2D_ARRAY(MousePosition, kMaxTags, MAX_PLAYER);
DECLARE_2D_ARRAY(LastMousePosition, kMaxTags, MAX_PLAYER);
// Panes exist as a global UI element that persist per imui begin / end calls.
// This allows for panes to stick around for bounds check, docking and imui
// debugability.
DECLARE_HASH_MAP_STR(Pane, 64);

void
GenerateUIMetadata(uint32_t tag)
{
  memcpy(kLastMousePosition[tag], kMousePosition[tag],
         sizeof(kMousePosition[tag]));
  kUsedLastMousePosition[tag] = kUsedMousePosition[tag];
}

void
ResetAll()
{
  for (int i = 0; i < kMaxTags; ++i) {
    GenerateUIMetadata(i);
  }
  memset(kUsedText, 0, sizeof(kUsedText));
  memset(kUsedButton, 0, sizeof(kUsedButton));
  memset(kUsedButtonCircle, 0, sizeof(kUsedButton));
  memset(kUsedMouseDown, 0, sizeof(kUsedMouseDown));
  memset(kUsedMouseUp, 0, sizeof(kUsedMouseUp));
  memset(kUsedMousePosition, 0, sizeof(kUsedMousePosition));
  memset(kUsedMouseWheel, 0, sizeof(kUsedMouseWheel));
  memset(kUsedProgressBar, 0, sizeof(kUsedProgressBar));
  memset(kUsedLine, 0, sizeof(kUsedLine));
}

void
ResetTag(uint32_t tag)
{
  assert(tag < kMaxTags);
  GenerateUIMetadata(tag);
  kUsedText[tag] = 0;
  kUsedButton[tag] = 0;
  kUsedButtonCircle[tag] = 0;
  kUsedMouseDown[tag] = 0;
  kUsedMouseUp[tag] = 0;
  kUsedMousePosition[tag] = 0;
  kUsedMouseWheel[tag] = 0;
  kUsedProgressBar[tag] = 0;
  kUsedLine[tag] = 0;
}

v2f
MouseDelta()
{
  uint32_t tag = kIMUI.begin_mode.tag;
  if (kUsedMousePosition[tag] < 1 || kUsedLastMousePosition[tag] < 1)
    return {};
  return kMousePosition[tag][0].pos - kLastMousePosition[tag][0].pos;
}

void
SetScissorWithPane(const Pane& pane, const v2f& viewport, bool ignore_scissor)
{
  if (ignore_scissor) {
    glScissor(0, 0, viewport.x, viewport.y);
  } else {
    // Scissor from header down.
    glScissor(pane.rect.x, pane.rect.y, pane.rect.width,
              pane.rect.height - pane.header_rect.height);
  }
}

void
Render(uint32_t tag)
{
  glDisable(GL_DEPTH_TEST);
  // TODO(abrunasso): Use glScissor for clipping panes so that when scrolling
  // text nicely gets clipped regardless of location in pane.
  // https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glScissor.xml
  auto dims = window::GetWindowSize();
  rgg::ModifyObserver mod(math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f),
                          math::Identity());

  kIMUI.text_exhaustion[tag] = kUsedText[tag];
  kIMUI.button_exhaustion[tag] = kUsedButton[tag];
  kIMUI.button_circle_exhaustion[tag] = kUsedButtonCircle[tag];

  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) continue;
    rgg::RenderRectangle(pane->rect, pane->options.color);
    rgg::RenderRectangle(pane->header_rect, kPaneHeaderColor);
    rgg::RenderLineRectangle(
        pane->rect, 0.f, v4f(0.2f, 0.2f, 0.2f, 0.7f));
    if (pane->has_scroll_bar) {
      rgg::RenderRectangle(pane->scroll_rect, kScrollColor);
    }
  }

  for (int i = 0; i < kUsedButton[tag]; ++i) {
    Button* button = &kButton[tag][i];
    SetScissorWithPane(*button->pane, dims, false);
    rgg::RenderButton("test", button->rect, button->color);
  }

  for (int i = 0; i < kUsedButtonCircle[tag]; ++i) {
    ButtonCircle* button = &kButtonCircle[tag][i];
    SetScissorWithPane(*button->pane, dims,
                       button->options.ignore_scissor_test);
    rgg::RenderCircle(button->position, button->radius, button->color);
  }

  for (int i = 0; i < kUsedText[tag]; ++i) {
    Text* text = &kText[tag][i];
    SetScissorWithPane(*text->pane, dims, text->options.ignore_scissor_test);
    rgg::RenderText(text->msg, text->pos, kTextScale, text->color);
  }

  for (int i = 0; i < kUsedLine[tag]; ++i) {
    Line* line = &kLine[tag][i];
    SetScissorWithPane(*line->pane, dims, false);
    if (line->type == kHorizontal) {
      v2f end(line->start.x + line->pane->rect.width, line->start.y);
      rgg::RenderLine(line->start, end, line->color);
    }
  }

  for (int i = 0; i < kUsedProgressBar[tag]; ++i) {
    ProgressBar* pb = &kProgressBar[tag][i];
    SetScissorWithPane(*pb->pane, dims, false);
    rgg::RenderProgressBar(
        pb->rect, 0.f, pb->current_progress, pb->max_progress, pb->fill_color,
        pb->outline_color);
  }
  glScissor(0, 0, dims.x, dims.y);
  glEnable(GL_DEPTH_TEST);
}

float
GetMouseWheel()
{
  uint32_t tag = kIMUI.begin_mode.tag;
  if (!kUsedMouseWheel[tag]) return 0.f;
  float d = 0.f;
  for (int i = 0; i < kUsedMouseWheel[tag]; ++i) {
    d += kMouseWheel[tag][i].delta;
  }
  return d;
}

v2f
GetMousePosition()
{
  uint32_t tag = kIMUI.begin_mode.tag;
  if (!kUsedMousePosition[tag]) return {};
  return kMousePosition[tag][0].pos;
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
IsRectPreviouslyHighlighted(Rectf rect)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedLastMousePosition[tag]; ++i) {
    LastMousePosition* mp = &kLastMousePosition[tag][i];
    if (math::PointInRect(mp->pos, rect)) return true;
  }
  return false;
}

bool
IsRectClicked(Rectf rect)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedMouseDown[tag]; ++i) {
    MouseDown* click = &kMouseDown[tag][i];
    if (math::PointInRect(click->pos, rect)) return true;
  }
  return false;
}

bool
IsCircleHighlighted(v2f center, float radius)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedMousePosition[tag]; ++i) {
    MousePosition* mp = &kMousePosition[tag][i];
    if (math::PointInCircle(mp->pos, center, radius)) return true;
  }
  return false;
}

bool
IsCircleClicked(v2f center, float radius)
{
  uint32_t tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedMouseDown[tag]; ++i) {
    MouseDown* click = &kMouseDown[tag][i];
    if (math::PointInCircle(click->pos, center, radius)) return true;
  }
  return false;
}

bool
IsMouseDown()
{
  uint32_t tag = kIMUI.begin_mode.tag;
  return kIMUI.mouse_down[tag];
}

void
Indent(int spaces)
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
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
UpdatePane(float width, float height, bool* element_in_pane)
{
  assert(element_in_pane);
  IF_HIDDEN(return Rectf());
  auto& begin_mode = kIMUI.begin_mode;
  assert(begin_mode.set);
  assert(begin_mode.pane);
  if (begin_mode.overwrite_width) width = begin_mode.overwrite_width;
  begin_mode.overwrite_width = 0.f;
  if (begin_mode.flow_switch || begin_mode.flow_type == kNewLine) {
    if (begin_mode.pos.y <= begin_mode.pane->rect.y) {
      begin_mode.pane->rect.y -= height;
      begin_mode.pane->rect.height += height;
      begin_mode.pane->theoretical_height += height;
      if (begin_mode.pane->options.max_height) {
        if (begin_mode.pane->rect.height >
            begin_mode.pane->options.max_height) {
          begin_mode.pane->rect.height = begin_mode.pane->options.max_height;
          begin_mode.pane->rect.y =
              begin_mode.start->y - begin_mode.pane->rect.height;
        }
      }
    }
  }
  float new_width = (begin_mode.pos.x + width) - begin_mode.pane->rect.x;
  if (!begin_mode.flow_switch && begin_mode.flow_type == kSameLine) {
    new_width += begin_mode.last_rect.width;
  }
  if (new_width > begin_mode.pane->rect.width) {
    begin_mode.pane->rect.width = new_width;
    if (begin_mode.pane->options.max_width &&
        begin_mode.pane->rect.width > begin_mode.pane->options.max_width) {
      begin_mode.pane->rect.width = begin_mode.pane->options.max_width;
    }
  }
  if (begin_mode.flow_switch || begin_mode.flow_type == kNewLine) {
    begin_mode.pos.y -= height;
  }
  if (!begin_mode.flow_switch && begin_mode.flow_type == kSameLine) {
    begin_mode.pos.x += begin_mode.last_rect.width;
  }
  begin_mode.flow_switch = false;
  begin_mode.last_rect = Rectf(
      begin_mode.pos.x, begin_mode.pos.y, width, height);
  if (!begin_mode.ignore_vertical_scroll) {
    begin_mode.last_rect.y += begin_mode.pane->vertical_scroll;
  }
  *element_in_pane =
      math::IntersectRect(begin_mode.last_rect, begin_mode.pane->rect);
  begin_mode.pane->element_off_pane =
      !math::IsContainedInRect(begin_mode.last_rect, begin_mode.pane->rect);
  return begin_mode.last_rect;
}

Result
Text(const char* msg, TextOptions options)
{
  assert(kIMUI.begin_mode.set);
  auto& begin_mode = kIMUI.begin_mode;
  uint32_t tag = kIMUI.begin_mode.tag;
  Result data;
  IF_HIDDEN(return data);
  if (strlen(msg) > kMaxTextSize) {
    imui_errno = 2;
    return data;
  }
  Rectf text_rect = 
      rgg::GetTextRect(msg, strlen(msg), begin_mode.pos, kTextScale);
  bool in_pane = false;
  Rectf rect = UpdatePane(text_rect.width, text_rect.height, &in_pane);
  if (!in_pane) return data;
  struct Text* text = UseText(tag);
  if (!text) {
    imui_errno = 1;
    return data;
  }
  strcpy(text->msg, msg);
  text->pos = v2f(rect.x, rect.y);
  text->color = options.color;
  if (IsRectHighlighted(rect) && options.highlight_color != v4f()) {
    text->color = options.highlight_color;
  }
  text->options = options;
  text->rect = rect;
  text->pane = begin_mode.pane;
  return IMUI_RESULT(rect);
}

Result
Text(const char* msg)
{
  assert(kIMUI.begin_mode.set);
  return Text(msg, {kWhite, kWhite});
}

void
HorizontalLine(const v4f& color)
{
  assert(kIMUI.begin_mode.set);
  uint32_t tag = kIMUI.begin_mode.tag;
  IF_HIDDEN(return);
  bool in_pane = false;
  Rectf rect = UpdatePane(kIMUI.begin_mode.pane->rect.width, 1.f, &in_pane);
  // TODO(abrunass): Not working for horizontal lines - not sure why.
  //if (!in_pane) return;
  Line* line = UseLine(tag);
  if (!line) {
    imui_errno = 5;
    return;
  }
  line->start.x = rect.x;
  line->start.y = rect.y;
  line->pane = kIMUI.begin_mode.pane;
  line->color = color;
  line->pane = kIMUI.begin_mode.pane;
}

void
Space(SpaceType type, int count)
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  bool in_pane = false;
  if (type == kHorizontal) {
    UpdatePane(count, 0.f, &in_pane);
    if (!in_pane) return;
  } else {
    UpdatePane(0.f, count, &in_pane);
    if (!in_pane) return;
  }
}

Result
Button(float width, float height, const v4f& color)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  uint32_t tag = kIMUI.begin_mode.tag;
  Result result;
  IF_HIDDEN(return result);
  bool in_pane = false;
  Rectf rect = UpdatePane(width, height, &in_pane);
  if (!in_pane) return result;
  struct Button* button = UseButton(tag);
  if (!button) {
    imui_errno = 3;
    return result;
  }
  button->rect = rect;
  button->color = color;
  button->pane = kIMUI.begin_mode.pane;
  return IMUI_RESULT(button->rect);
}

Result
ButtonCircle(float radius, const v4f& color,
             const ButtonCircleOptions& options)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  Result result;
  IF_HIDDEN(return result);
  uint32_t tag = kIMUI.begin_mode.tag;
  bool in_pane = false;
  Rectf rect = UpdatePane(2.f * radius, 2.f * radius, &in_pane);
  if (!in_pane) return result;
  struct ButtonCircle* button = UseButtonCircle(tag);
  if (!button) {
    imui_errno = 3;
    return result;
  }
  // RenderButton renders from center.
  button->position = v2f(rect.x, rect.y) + v2f(radius, radius);
  button->radius = radius;
  button->color = color;
  button->pane = kIMUI.begin_mode.pane;
  button->options = options;
  return IMUI_RESULT_CIRCLE(button->position, radius);
}

Result
ButtonCircle(float radius, const v4f& color)
{
  ButtonCircleOptions options;
  return ButtonCircle(radius, color, options);
}

Result
ProgressBar(float width, float height, float current_progress,
            float max_progress, const v4f& fill_color,
            const v4f& outline_color)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  uint32_t tag = kIMUI.begin_mode.tag;
  Result result;
  IF_HIDDEN(return result);
  bool in_pane = false;
  Rectf rect = UpdatePane(width, height, &in_pane);
  if (!in_pane) return result;
  struct ProgressBar* pb = UseProgressBar(tag);
  if (!pb) {
    imui_errno = 3;
    return result;
  }
  pb->rect = rect;
  pb->current_progress = current_progress;
  pb->max_progress = max_progress;
  pb->outline_color = outline_color;
  pb->fill_color = fill_color;
  pb->pane = kIMUI.begin_mode.pane;
  return IMUI_RESULT(pb->rect);
}

void
SameLine()
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  kIMUI.begin_mode.flow_type = kSameLine;
  kIMUI.begin_mode.flow_switch = true;
  kIMUI.begin_mode.x_reset = kIMUI.begin_mode.pos.x;
}

// Overwrite width to a certain value for the next imui call.
void
SetWidth(float width)
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  kIMUI.begin_mode.overwrite_width = width;
}

void
NewLine()
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  kIMUI.begin_mode.flow_type = kNewLine;
  kIMUI.begin_mode.flow_switch = true;
  kIMUI.begin_mode.pos.x = kIMUI.begin_mode.x_reset;
}

void
Begin(const char* title, uint32_t tag, const PaneOptions& pane_options,
      v2f* start, bool* show = nullptr)
{
  assert(tag < kMaxTags);
  assert(title);
  if (pane_options.max_width)
    assert(pane_options.width <= pane_options.max_width);
  if (pane_options.max_height)
    assert(pane_options.height <= pane_options.max_height);
  TITLE_WITH_TAG(title, tag);
  uint32_t title_with_tag_len = strlen(title_with_tag);
  assert(title_with_tag_len < kMaxHashKeyLength);
  auto& begin_mode = kIMUI.begin_mode;
  // End must be called before Begin.
  assert(!begin_mode.set);
  begin_mode.pos = *start;
  begin_mode.set = true;
  begin_mode.tag = tag;
  begin_mode.x_reset = start->x;
  begin_mode.start = start;
  begin_mode.pane = FindPane(title, title_with_tag_len);
  if (!begin_mode.pane) {
    begin_mode.pane = UsePane(title, title_with_tag_len);
  }
  uint32_t title_len = strlen(begin_mode.pane->title);
  strcpy(begin_mode.pane->title, title);
  // Header. TODO(abrunasso): imui now relies on hashing header title for pane
  // persistence - but it is worth adding a pane option to hide it here.
  SameLine();
  begin_mode.pos.x += 5.f;
  Rectf t = rgg::GetTextRect(title, title_len, *start, kTextScale);
  begin_mode.pane->tag = tag;
  begin_mode.pane->rect.width =
      pane_options.width > 0.f ? pane_options.width : t.width;
  begin_mode.pane->rect.height =
      pane_options.height > 0.f ? pane_options.height : 0.f;
  begin_mode.pane->theoretical_height = begin_mode.pane->rect.height;
  begin_mode.pane->rect.x = start->x;
  begin_mode.pane->rect.y = start->y - begin_mode.pane->rect.height;
  begin_mode.pane->options = pane_options;
  begin_mode.pane->hidden = show ? !(*show) : false;
  // Header is not effect by vertical scroll - it's kinda special.
  begin_mode.ignore_vertical_scroll = true;
  ButtonCircleOptions button_options;
  button_options.ignore_scissor_test = true;
  if (ButtonCircle(10.f, kHeaderMinimizeColor, button_options).clicked) {
    if (show) (*show) = !(*show);
  }
  begin_mode.pos.y -= 2.f;
  begin_mode.pos.x += 5.f;
  TextOptions text_options; 
  text_options.ignore_scissor_test = true;
  Rectf trect = Text(title, text_options).rect;
  begin_mode.ignore_vertical_scroll = false;
  begin_mode.pane->header_rect.height = trect.height;
  NewLine();
  if (show) begin_mode.show = show;
}

void
Begin(const char* title, uint32_t tag, v2f* start, bool* show = nullptr)
{
  PaneOptions pane_options;
  pane_options.color = v4f(0.f, 0.f, 0.f, 0.f);
  Begin(title, tag, pane_options, start, show);
}

bool
CanScroll(const Pane& pane, float delta)
{
  if (delta > 0.f && pane.element_off_pane) return true;
  if (delta < 0.f && pane.vertical_scroll > 0.f) return true;
  return false;
}

void
ClampVerticalScroll()
{
  assert(kIMUI.begin_mode.set);
  Pane* pane = kIMUI.begin_mode.pane;
  // Clamp the vertical scroll to the min and max possible scroll.
  if (pane->vertical_scroll < 0.f) pane->vertical_scroll = 0.f;
  if (pane->rect.height + pane->vertical_scroll > pane->theoretical_height) {
    pane->vertical_scroll = pane->theoretical_height - pane->rect.height;
  }
}

void
End()
{
  assert(kIMUI.begin_mode.set);
  Pane* pane = kIMUI.begin_mode.pane;
  pane->header_rect.x = pane->rect.x;
  pane->header_rect.y =
      pane->rect.y + pane->rect.height - pane->header_rect.height;
  pane->header_rect.width = pane->rect.width;
  bool scroll_highlighted =
    pane->has_scroll_bar && IsRectPreviouslyHighlighted(pane->scroll_rect);
  // Move around panes if a user has click and held in them.
  if (kIMUI.begin_mode.start && IsMouseDown() &&
      IsRectPreviouslyHighlighted(kIMUI.begin_mode.pane->rect) &&
      !scroll_highlighted) {
    *kIMUI.begin_mode.start += MouseDelta();
  }
  float d = GetMouseWheel();
  bool pane_highlighted = IsRectHighlighted(pane->rect);
  if (CanScroll(*pane, d) && pane_highlighted && d != 0.f) {
    pane->vertical_scroll += d;
  }
  ClampVerticalScroll();
  pane->has_scroll_bar = pane->theoretical_height > pane->rect.height;
  if (pane->has_scroll_bar) {
    Rectf scroll_bar(
        pane->rect.x + pane->rect.width - kScrollBarWidth, pane->rect.y,
        kScrollBarWidth, pane->rect.height - pane->header_rect.height);
    float hdiff = pane->theoretical_height - pane->rect.height;
    float p_off = pane->rect.height / pane->theoretical_height;
    float scroll_rect_height =
        (pane->rect.height - pane->header_rect.height) * p_off;
    Rectf scroll_cursor(scroll_bar);
    scroll_cursor.height = scroll_rect_height;
    scroll_cursor.y += (scroll_bar.height - scroll_rect_height);
    float p_diff = pane->vertical_scroll /
        (pane->theoretical_height - pane->rect.height);
    float p_bar_diff_to_bot = fabs(scroll_cursor.y) - fabs(pane->rect.y);
    scroll_cursor.y -= (p_bar_diff_to_bot * p_diff);
    pane->scroll_rect = scroll_cursor;
    if (IsMouseDown() && scroll_highlighted) {
      pane->vertical_scroll -= MouseDelta().y;
    }
  }
  ClampVerticalScroll();
  kIMUI.begin_mode = {};
}

void
VerticalScrollDelta(float delta)
{
  assert(kIMUI.begin_mode.set);
  kIMUI.begin_mode.pane->vertical_scroll += delta;
  if (kIMUI.begin_mode.pane->vertical_scroll < 0.f) {
    kIMUI.begin_mode.pane->vertical_scroll = 0.f;
  }
}

bool
MouseInUI(v2f pos, uint32_t tag)
{
  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) continue;
    if (math::PointInRect(pos, pane->rect)) return true;
  }
  return false;
}

bool
MouseInUI(uint32_t tag)
{
  for (int i = 0; i < kUsedMousePosition[tag]; ++i) {
    MousePosition* mp = &kMousePosition[tag][i];
    if (MouseInUI(mp->pos, tag)) return true;
  }
  return false;
}

void
MouseDown(v2f pos, PlatformButton b, uint32_t tag)
{
  // Only record mouse down events if they occurred in the UI.
  if (!MouseInUI(pos, tag)) return;
  struct MouseDown* click = UseMouseDown(tag);
  if (!click) {
    imui_errno = 4;
    return;
  }
  click->pos = pos;
  click->button = b;
  kIMUI.mouse_down[tag] = true;
}

void
MouseUp(v2f pos, PlatformButton b, uint32_t tag)
{
  struct MouseUp* click = UseMouseUp(tag);
  if (!click) {
    imui_errno = 4;
    return;
  }
  click->pos = pos;
  click->button = b;
  kIMUI.mouse_down[tag] = false;
}

void
MouseWheel(float delta, uint32_t tag)
{
  struct MouseWheel* wheel = UseMouseWheel(tag);
  if (!wheel) {
    imui_errno = 4;
    return;
  }
  wheel->delta = -delta * 5.f;
}

// Returns true if the mouse is contained within UI given bounds of last
// UI frame.
bool
MousePosition(v2f pos, uint32_t tag)
{
  struct MousePosition* mp = UseMousePosition(tag);
  if (!mp) {
    imui_errno = 4;
    return false;
  }
  mp->pos = pos;
  return MouseInUI(pos, tag);
}

// Prints useful imui internals for debugging.
void
DebugPane(const char* title, uint32_t tag, v2f* pos, bool* show)
{
  char buffer[64];
  PaneOptions pane_options;
  Begin(title, tag, pane_options, pos, show);
  snprintf(buffer, 64, "Pane Count (%u) Max Hash Count (%u) Collision (%.2f%%)",
           kUsedPane, kMaxHashPane,
           ((float)(kFindCollisionsPane) / kFindCallsPane) * 100.f);
  Text(buffer);
  Text("Panes");
  HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  Indent(2);
  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    TextOptions toptions;
    toptions.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
    if (Text(pane->title, toptions).clicked) {
      pane->debug_show_details = !pane->debug_show_details;
    }
    HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
    if (pane->debug_show_details) {
      Indent(2);
      TITLE_WITH_TAG(pane->title, pane->tag);
      uint32_t hash = GetHash(title_with_tag, strlen(title_with_tag));
      snprintf(buffer, 64, "tag: %u", pane->tag);
      Text(buffer);
      snprintf(buffer, 64, "hash: %u", hash);
      Text(buffer);
      snprintf(buffer, 64, "hash_idx: %u", hash % kMaxHashPane);
      Text(buffer);
      snprintf(buffer, 64, "hidden: %i", pane->hidden);
      Text(buffer);
      snprintf(buffer, 64, "rect (%.2f,%.2f,%.2f,%.2f)",
               pane->rect.x, pane->rect.y, pane->rect.width,
               pane->rect.height);
      Text(buffer);
      snprintf(buffer, 64, "theoretical_height (%.2f)",
               pane->theoretical_height);
      Text(buffer);
      snprintf(buffer, 64, "vscroll (%.2f)", pane->vertical_scroll);
      Text(buffer);
      Indent(-2);
      HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
    }
  }
  Indent(-2);

  Text("Tags");
  HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  Indent(2);
  for (int i = 0; i < kMaxTags; ++i) {
    snprintf(buffer, 64, "Tag %i", i);
    TextOptions toptions;
    toptions.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
    if (Text(buffer, toptions).clicked) {
      kIMUI.debug_show_details[i] = !kIMUI.debug_show_details[i];
    }
    HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
    if (kIMUI.debug_show_details[i]) {
      Indent(2);
      snprintf(buffer, 64, "Text Exhaustion (%u / %u)  ",
               kIMUI.text_exhaustion[i], kMaxText);
      Text(buffer);
      ProgressBar(100.f, 8.f, kIMUI.text_exhaustion[i], kMaxText,
                  v4f(1.f, 0.f, 0.f, 1.f), v4f(.3f, .3f, .3f, 1.f));
      snprintf(buffer, 64, "Button Exhaustion (%u / %u) Circle (%u / %u)",
               kIMUI.button_exhaustion[i], kMaxButton,
               kIMUI.button_circle_exhaustion[i], kMaxButtonCircle);
      Text(buffer);
      Indent(-2);
      HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
    }
  }
  Indent(-2);

  v2f mouse_pos = GetMousePosition();
  snprintf(buffer, 64, "Mouse Pos (%.2f,%.2f)", mouse_pos.x, mouse_pos.y);
  Text(buffer);
  v2f delta = MouseDelta();
  snprintf(buffer, 64, "Mouse Delta (%.2f,%.2f)", delta.x, delta.y);
  Text(buffer);
  snprintf(buffer, 64, "Mouse Down (%i)", imui::IsMouseDown());
  Text(buffer);
  snprintf(buffer, 64, "Mouse Wheel (%.2f)", imui::GetMouseWheel());
  Text(buffer);
  
  // This needs to run last else MouseInUI won't run correctly against this
  // panels bounds...
  SameLine();
  Text("Mouse in UI ");
  for (int i = 0; i < kMaxTags; ++i) {
    snprintf(buffer, 64, "(tag:%i, in_ui:%u) ", i, imui::MouseInUI(mouse_pos, i));
    Text(buffer);
  }
  Text(")");
  NewLine();
  End();
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
    case 5:
      return ("imui line count exhausted.");

  };

  return 0;
}

}  // namespace imui
