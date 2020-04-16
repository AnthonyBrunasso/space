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

static const v4f kWhite(1.f, 1.f, 1.f, 1.f);
static const v4f kPaneColor(0.0f, 0.0f, 0.0f, 0.4f);
static const v4f kPaneHeaderColor(0.12f, 0.16f, 0.154f, 1.f);
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
  v4f color = kPaneColor;
  Rectf header_rect;
};

// imui elements.

struct Pane {
  uint32_t tag;
  Rectf rect;
  PaneOptions options;
  bool hidden = false;
  bool debug_show_details = false;
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
  Pane* pane;
};

struct Line {
  v2f start;
  SpaceType type;
  v4f color;
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


struct MousePosition {
  v2f pos;
};

struct LastMousePosition {
  v2f pos;
};

struct BeginMode {
  v2f pos;
  bool set = false;
  int text_calls = 0;
  uint32_t tag = 0;
  // UI Element go one new line unless explcitly swapped.
  FlowType flow_type = kNewLine;
  bool flow_switch = false;
  Rectf last_rect;
  float x_reset;
  bool mouse_down;
  bool* show = nullptr;
  v2f* start = nullptr;
  Pane* pane;
};

constexpr uint32_t kMaxTags = MAX_PLAYER + 1;
constexpr uint32_t kEveryoneTag = MAX_PLAYER;

struct IMUI {
  BeginMode begin_mode;
  bool mouse_down[kMaxTags];
};

static IMUI kIMUI;

DECLARE_2D_ARRAY(Text, kMaxTags, 128);
DECLARE_2D_ARRAY(Line, kMaxTags, 16);
DECLARE_2D_ARRAY(Button, kMaxTags, 16);
DECLARE_2D_ARRAY(ButtonCircle, kMaxTags, 16);
DECLARE_2D_ARRAY(MouseDown, kMaxTags, 8);
DECLARE_2D_ARRAY(MouseUp, kMaxTags, 8);
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
Render(uint32_t tag)
{
  glDisable(GL_DEPTH_TEST);
  auto dims = window::GetWindowSize();
  rgg::ModifyObserver mod(math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f),
                          math::Identity());

  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) continue;
    rgg::RenderRectangle(pane->rect, pane->options.color);
    pane->options.header_rect.x = pane->rect.x;
    pane->options.header_rect.y =
        pane->rect.y + pane->rect.height - pane->options.header_rect.height;
    pane->options.header_rect.width = pane->rect.width;
    rgg::RenderRectangle(pane->options.header_rect,
                         kPaneHeaderColor);
    rgg::RenderLineRectangle(
        pane->rect, 0.f, v4f(0.2f, 0.2f, 0.2f, 0.7f));
  }

  for (int i = 0; i < kUsedButton[tag]; ++i) {
    Button* button = &kButton[tag][i];
    rgg::RenderButton("test", button->rect, button->color);
  }

  for (int i = 0; i < kUsedButtonCircle[tag]; ++i) {
    ButtonCircle* button = &kButtonCircle[tag][i];
    rgg::RenderCircle(button->position, button->radius, button->color);
  }

  for (int i = 0; i < kUsedText[tag]; ++i) {
    Text* text = &kText[tag][i];
    rgg::RenderText(text->msg, text->pos, kTextScale, text->color);
  }

  for (int i = 0; i < kUsedLine[tag]; ++i) {
    Line* line = &kLine[tag][i];
    if (line->type == kHorizontal) {
      v2f end(line->start.x + line->pane->rect.width, line->start.y);
      rgg::RenderLine(line->start, end, line->color);
    }
  }
  glEnable(GL_DEPTH_TEST);
}

v2f GetMousePosition()
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
UpdatePane(float width, float height)
{
  IF_HIDDEN(return Rectf());
  auto& begin_mode = kIMUI.begin_mode;
  assert(begin_mode.set);
  assert(begin_mode.pane);
  if (begin_mode.flow_switch || begin_mode.flow_type == kNewLine) {
    begin_mode.pane->rect.y -= height;
    begin_mode.pane->rect.height += height;
  }
  float new_width = (begin_mode.pos.x + width) - begin_mode.pane->rect.x;
  if (!begin_mode.flow_switch && begin_mode.flow_type == kSameLine) {
    new_width += begin_mode.last_rect.width;
  }
  if (new_width > begin_mode.pane->rect.width) {
    begin_mode.pane->rect.width = new_width;
  }
  if (begin_mode.flow_switch || begin_mode.flow_type == kNewLine) {
    begin_mode.pos.y -= height;
  }
  if (!begin_mode.flow_switch && begin_mode.flow_type == kSameLine) {
    begin_mode.pos.x += begin_mode.last_rect.width;
  }
  begin_mode.flow_switch = false;
  begin_mode.last_rect =
      Rectf(begin_mode.pos.x, begin_mode.pos.y, width, height);
  return begin_mode.last_rect;
}

void
UpdatePaneOnEnd(Pane* pane)
{
  if (!pane) return;
  IF_HIDDEN(return);
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
  Result data;
  IF_HIDDEN(return data);
  struct Text* text = UseText(tag);
  if (!text) {
    imui_errno = 1;
    return data;
  }
  if (strlen(msg) > kMaxTextSize) {
    imui_errno = 2;
    return data;
  }
  Rectf text_rect = 
      rgg::GetTextRect(msg, strlen(msg), begin_mode.pos, kTextScale);
  Rectf rect = UpdatePane(text_rect.width, text_rect.height);
  strcpy(text->msg, msg);
  text->pos = v2f(rect.x, rect.y);
  text->color = options.color;
  if (IsRectHighlighted(rect) && options.highlight_color != v4f()) {
    text->color = options.highlight_color;
  }
  text->options = options;
  text->rect = rect;
  text->pane = begin_mode.pane;
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
HorizontalLine(const v4f& color)
{
  assert(kIMUI.begin_mode.set);
  uint32_t tag = kIMUI.begin_mode.tag;
  IF_HIDDEN(return);
  Line* line = UseLine(tag);
  if (!line) {
    imui_errno = 5;
    return;
  }
  line->start.x = kIMUI.begin_mode.pane->rect.x;
  line->start.y = kIMUI.begin_mode.pos.y;
  line->pane = kIMUI.begin_mode.pane;
  line->color = color;
  line->pane = kIMUI.begin_mode.pane;
  UpdatePane(line->pane->rect.width, 1.f);
}

void
Space(SpaceType type, int count)
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  if (type == kHorizontal) {
    UpdatePane(count, 0.f);
  } else {
    UpdatePane(0.f, count);
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
  struct Button* button = UseButton(tag);
  if (!button) {
    imui_errno = 3;
    return result;
  }
  Rectf rect = UpdatePane(width, height);
  button->rect = rect;
  button->color = color;
  button->pane = kIMUI.begin_mode.pane;
  return IMUI_RESULT(button->rect);
}

Result
ButtonCircle(float radius, const v4f& color)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  Result result;
  IF_HIDDEN(return result);
  uint32_t tag = kIMUI.begin_mode.tag;
  struct ButtonCircle* button = UseButtonCircle(tag);
  if (!button) {
    imui_errno = 3;
    return result;
  }
  Rectf rect = UpdatePane(2.f * radius, 2.f * radius);
  // RenderButton renders from center.
  button->position = v2f(rect.x, rect.y) + v2f(radius, radius);
  button->radius = radius;
  button->color = color;
  button->pane = kIMUI.begin_mode.pane;
  return IMUI_RESULT_CIRCLE(button->position, radius);
}

void
ToggleSameLine()
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  kIMUI.begin_mode.flow_type = kSameLine;
  kIMUI.begin_mode.flow_switch = true;
  kIMUI.begin_mode.x_reset = kIMUI.begin_mode.pos.x;
}

void
ToggleNewLine()
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
  TITLE_WITH_TAG(title, tag);
  assert(tag < kMaxTags);
  assert(title);
  uint32_t title_with_tag_len = strlen(title_with_tag);
  assert(title_with_tag_len < kMaxHashKeyLength);
  auto& begin_mode = kIMUI.begin_mode;
  // End must be called before Begin.
  assert(!begin_mode.set);
  begin_mode.pos = *start;
  begin_mode.set = true;
  begin_mode.text_calls = 0;
  begin_mode.tag = tag;
  begin_mode.x_reset = start->x;
  begin_mode.start = start;
  begin_mode.pane = FindPane(title, title_with_tag_len);
  if (!begin_mode.pane) {
    begin_mode.pane = UsePane(title, title_with_tag_len);
  }
  uint32_t title_len = strlen(begin_mode.pane->title);
  strcpy(begin_mode.pane->title, title);
  begin_mode.pane->tag = tag;
  begin_mode.pane->rect.x = start->x;
  begin_mode.pane->rect.y = start->y;
  begin_mode.pane->rect.width = pane_options.width;
  begin_mode.pane->rect.height = pane_options.height;
  begin_mode.pane->options = pane_options;
  begin_mode.pane->hidden = show ? !(*show) : false;
  // Header. TODO(abrunasso): imui now relies on hashing header title for pane
  // persistence - but it is worth adding a pane option to hide it here.
  ToggleSameLine();
  begin_mode.pos.x += 5.f;
  Rectf t = rgg::GetTextRect(title, title_len, *start, kTextScale);
  if (ButtonCircle(10.f, kHeaderMinimizeColor).clicked) {
    if (show) (*show) = !(*show);
  }
  begin_mode.pos.y -= 2.f;
  begin_mode.pos.x += 5.f;
  Rectf trect = Text(title).rect;
  begin_mode.pane->options.header_rect.height = trect.height;
  ToggleNewLine();
  if (show) begin_mode.show = show;
}

void
Begin(const char* title, uint32_t tag, v2f* start, bool* show = nullptr)
{
  PaneOptions pane_options;
  pane_options.color = v4f(0.f, 0.f, 0.f, 0.f);
  Begin(title, tag, pane_options, start, show);
}

void
End()
{
  UpdatePaneOnEnd(kIMUI.begin_mode.pane);
  // Move around panes if a user has click and held in them.
  if (kIMUI.begin_mode.start && IsMouseDown() &&
      IsRectPreviouslyHighlighted(kIMUI.begin_mode.pane->rect)) {
    *kIMUI.begin_mode.start += MouseDelta();
  }
  kIMUI.begin_mode = {};
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
               pane->rect.x, pane->rect.y, pane->rect.width, pane->rect.height);
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
  // This needs to run last else MouseInUI won't run correctly against this
  // panels bounds...
  ToggleSameLine();
  Text("Mouse in UI ");
  for (int i = 0; i < kMaxTags; ++i) {
    snprintf(buffer, 64, "(tag:%i, in_ui:%u) ", i, imui::MouseInUI(mouse_pos, i));
    Text(buffer);
  }
  Text(")");
  ToggleNewLine();
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
