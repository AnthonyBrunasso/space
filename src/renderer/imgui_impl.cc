#pragma once

#include <stdio.h>
#include <stdlib.h>

static b8 ImGuiImplInitForOpenGL() {
  GLint major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  ImGuiIO& io = ImGui::GetIO();
  v2f window_size = window::GetWindowSize();
  io.DisplaySize.x = window_size.x;
  io.DisplaySize.y = window_size.y;
  return ImGui_ImplOpenGL3_Init("#version 130");
}

static b8 ImGuiImplSetup() {
  ImGui::CreateContext(); 
  ImGui::StyleColorsDark();
  return ImGuiImplInitForOpenGL();
}

static void ImGuiImplNewFrame() {
  ImGui_ImplOpenGL3_NewFrame();
}

static void ImGuiImplRenderDrawData() {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static ImGuiKey ImGuiImplPlatformKeyToImguiKey(u32 keycode) {
  switch (keycode) {
    case KEY_ESC: return ImGuiKey_Escape;
    case KEY_A: return ImGuiKey_A;
    case KEY_B: return ImGuiKey_B;
    case KEY_C: return ImGuiKey_C;
    case KEY_D: return ImGuiKey_D;
    case KEY_E: return ImGuiKey_E;
    case KEY_F: return ImGuiKey_F;
    case KEY_G: return ImGuiKey_G;
    case KEY_H: return ImGuiKey_H;
    case KEY_I: return ImGuiKey_I;
    case KEY_J: return ImGuiKey_J;
    case KEY_K: return ImGuiKey_K;
    case KEY_L: return ImGuiKey_L;
    case KEY_M: return ImGuiKey_M;
    case KEY_N: return ImGuiKey_N;
    case KEY_O: return ImGuiKey_O;
    case KEY_P: return ImGuiKey_P;
    case KEY_Q: return ImGuiKey_Q;
    case KEY_R: return ImGuiKey_R;
    case KEY_S: return ImGuiKey_S;
    case KEY_T: return ImGuiKey_T;
    case KEY_U: return ImGuiKey_U;
    case KEY_V: return ImGuiKey_V;
    case KEY_W: return ImGuiKey_W;
    case KEY_X: return ImGuiKey_X;
    case KEY_Y: return ImGuiKey_Y;
    case KEY_Z: return ImGuiKey_Z;
    case KEY_RETURN: return ImGuiKey_Enter;
    case KEY_TAB: return ImGuiKey_Tab;
    case KEY_HOME: return ImGuiKey_Home;
    case KEY_END: return ImGuiKey_End;
    case KEY_DEL: return ImGuiKey_Delete;
    case KEY_BACKSPACE: return ImGuiKey_Backspace;
    case KEY_SPACE: return ImGuiKey_Space;
    case KEY_COMMA: return ImGuiKey_Comma;
    case KEY_PERIOD: return ImGuiKey_Period;
    case KEY_SLASH: return ImGuiKey_Slash;
    case KEY_SEMICOLON: return ImGuiKey_Semicolon;
    case KEY_EQUALS: return ImGuiKey_Equal;
  }
  return ImGuiKey_None;
}

static bool ImGuiImplProcessEvent(const PlatformEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  switch (event.type) {
    case MOUSE_MOVE: {
      v2f dims = window::GetWindowSize();
      io.AddMousePosEvent(event.position.x, dims.y - event.position.y);
      return true;
    } break;
    case MOUSE_WHEEL: {
      // TODO: Wheel event for x and y????
      io.AddMouseWheelEvent(0.f, event.wheel_delta > 0.f ? 1.f : -1.f);
      return true;
    } break;
    case MOUSE_DOWN:
    case MOUSE_UP: {
      int mouse_button = -1;
      if (event.button == BUTTON_LEFT) mouse_button = 0;
      if (event.button == BUTTON_RIGHT) mouse_button = 1;
      if (event.button == BUTTON_MIDDLE) mouse_button = 2;
      if (mouse_button == -1) break;
      io.AddMouseButtonEvent(mouse_button, event.type == MOUSE_DOWN);
      return true;
    } break;
    case KEY_DOWN:
    case KEY_UP: {
      //LOG(INFO, "event key %u", event.key);
      ImGuiKey key = ImGuiImplPlatformKeyToImguiKey(event.key);
      //LOG(INFO, "imgui event key %u", key);
      io.AddKeyEvent(key, (event.type == KEY_DOWN));
      if (event.key >= 0 && event.key < 256 && event.type == KEY_UP) {
        io.AddInputCharactersUTF8((const char*)&event.key);
      }
      return true;
    } break;
  }
  return false;
}

static void ImGuiRenderLastItemBoundingBox() {
  ImVec2 min = ImGui::GetItemRectMin();
  ImVec2 max = ImGui::GetItemRectMax();
  ImGui::GetForegroundDrawList()->AddRect( min, max, IM_COL32( 255, 255, 0, 255 ) );
}

static void ImGuiTextRect(const char* text, const Rectf& rect) {
  ImGui::Text("%s %.1f %.1f %.1f %.1f", text, rect.x, rect.y, rect.width, rect.height);
}
