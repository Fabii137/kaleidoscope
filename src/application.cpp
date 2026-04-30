#include "Application.hpp"
#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include <cstdint>

Application::Application() {}

void Application::run() {
  InitWindow(m_Settings.screenWidth, m_Settings.screenHeight, "Kaleidoscope");
  SetTargetFPS(m_Settings.targetFps);
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  m_Offset = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  initCamera();

  rlImGuiSetup(true);

  while (!WindowShouldClose()) {
    handleInput();

    float dt = GetFrameTime();
    update(dt);

    BeginDrawing();
    ClearBackground(m_Settings.backgroundColor);
    draw();
    EndDrawing();
  }

  CloseWindow();
}

void Application::initCamera() {
  m_Camera.target = {0.f, 0.f};
  m_Camera.offset = m_Offset;
  m_Camera.rotation = 0.f;
  m_Camera.zoom = 1.f;
}

void Application::handleInput() {
  if (IsKeyPressed(KEY_R)) {
    reset();
  }

  if (IsKeyPressed(KEY_TAB)) {
    m_ShowSettings = !m_ShowSettings;
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    m_MousePos.reset();
  }

  if (IsWindowResized()) {
    int scrWidth = GetScreenWidth();
    int scrHeight = GetScreenHeight();
    m_Settings.screenWidth = scrWidth;
    m_Settings.screenHeight = scrHeight;
    m_Offset = {scrWidth / 2.f, scrHeight / 2.f};
    m_Camera.offset = m_Offset;
  }
}

void Application::update(float dt) {
  if (ImGui::GetIO().WantCaptureMouse)
    return;

  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) || !IsCursorOnScreen() ||
      m_LineIdx >= MAX_LINES)
    return;

  auto prevMousePos = m_MousePos;
  m_MousePos = GetMousePosition();

  if (!prevMousePos.has_value())
    return;

  if (Vector2Distance(prevMousePos.value(), m_MousePos.value()) <
      m_Settings.minMouseDistance)
    return;

  Line &line = m_Lines[m_LineIdx++];
  line.start = Vector2Subtract(prevMousePos.value(), m_Offset);
  line.end = Vector2Subtract(m_MousePos.value(), m_Offset);
}

void Application::draw() {
  BeginMode2D(m_Camera);
  float angle = 360.f / m_Settings.symmetry * DEG2RAD;
  for (int i = 0; i < m_LineIdx; i++) {
    const Line &line = m_Lines[i];
    for (int j = 0; j < m_Settings.symmetry; j++) {
      Vector2 lineStart = Vector2Rotate(line.start, angle * j);
      Vector2 lineEnd = Vector2Rotate(line.end, angle * j);
      DrawLineEx(lineStart, lineEnd, m_Settings.lineThickness,
                 m_Settings.lineColor);

      if (m_Settings.enableReflection) {
        // reflected line
        Vector2 refLineStart =
            Vector2Multiply(lineStart, m_Settings.reflectionScale);
        Vector2 refLineEnd =
            Vector2Multiply(lineEnd, m_Settings.reflectionScale);
        DrawLineEx(refLineStart, refLineEnd, m_Settings.lineThickness,
                   m_Settings.lineColor);
      }
    }
  }
  EndMode2D();

  if (m_ShowSettings)
    drawSettings();
  else
    DrawText("Press TAB to open settngs", 20, 20, 12, WHITE);
}

void Application::drawSettings() {
  rlImGuiBegin();
  ImGui::Begin("Settings");

  ImGui::Text("FPS: %d", GetFPS());
  ImGui::Separator();

  ImGui::Text("General");
  if (ImGui::SliderInt("Target FPS", &m_Settings.targetFps, 1, 240)) {
    SetTargetFPS(m_Settings.targetFps);
  }
  ImGui::SliderInt("Symmetry", &m_Settings.symmetry, 1, 32);
  ImGui::SliderFloat("Line Thickness", &m_Settings.lineThickness, 1.f, 10.f);
  ImGui::SliderFloat("Min. Mouse Distance", &m_Settings.minMouseDistance, 0.f,
                     32.f);
  ImGui::Separator();

  ImGui::Text("Reflection");
  ImGui::Checkbox("Enable Reflection", &m_Settings.enableReflection);
  ImGui::SliderFloat("Reflection Scale X", &m_Settings.reflectionScale.x, -1,
                     1);
  ImGui::SliderFloat("Reflection Scale Y", &m_Settings.reflectionScale.y, -1,
                     1);
  ImGui::Separator();

  float bgColor[4];
  float lineColor[4];
  colorToFloat4(m_Settings.backgroundColor, bgColor);
  colorToFloat4(m_Settings.lineColor, lineColor);
  ImGui::Text("Colors");
  if (ImGui::ColorEdit4("Background", bgColor)) {
    m_Settings.backgroundColor = float4ToColor(bgColor);
  }
  if (ImGui::ColorEdit4("Line", lineColor)) {
    m_Settings.lineColor = float4ToColor(lineColor);
  }

  if (ImGui::Button("Reset Lines(R)")) {
    reset();
  }
  ImGui::SameLine();
  if (ImGui::Button("Reset Settings")) {
    m_Settings = {GetScreenWidth(), GetScreenHeight()};
    m_Settings.screenWidth = GetScreenWidth();
    m_Settings.screenHeight = GetScreenHeight();
  }

  ImGui::End();
  rlImGuiEnd();
}

void Application::reset() {
  m_LineIdx = 0;
  m_MousePos.reset();
}

void Application::colorToFloat4(Color c, float *float4) {
  float4[0] = c.r / 255.f;
  float4[1] = c.g / 255.f;
  float4[2] = c.b / 255.f;
  float4[3] = c.a / 255.f;
}

Color Application::float4ToColor(float *float4) {
  return Color{static_cast<std::uint8_t>(float4[0] * 255.f),
               static_cast<std::uint8_t>(float4[1] * 255.f),
               static_cast<std::uint8_t>(float4[2] * 255.f),
               static_cast<std::uint8_t>(float4[3] * 255.f)};
}
