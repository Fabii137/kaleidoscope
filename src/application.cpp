#include "Application.hpp"
#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"
#include <algorithm>
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

  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    constexpr float zoomFactor = 1.1f;
    m_Camera.zoom *= (wheel > 0) ? zoomFactor : (1.0f / zoomFactor);
    m_Camera.zoom = std::clamp(m_Camera.zoom, 0.01f, 20.f);
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
  if (m_Settings.rotate) {
    m_Rotation = fmodf(m_Rotation + m_Settings.rotationSpeed * dt, 360.f);
  }

  if (ImGui::GetIO().WantCaptureMouse)
    return;

  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) || !IsCursorOnScreen() ||
      m_LineIdx >= MAX_LINES)
    return;

  auto prevMousePos = m_MousePos;
  m_MousePos = GetScreenToWorld2D(GetMousePosition(), m_Camera);

  if (!prevMousePos.has_value())
    return;

  if (Vector2Distance(prevMousePos.value(), m_MousePos.value()) <
      m_Settings.minMouseDistance)
    return;

  Line &line = m_Lines[m_LineIdx++];
  line.start = prevMousePos.value();
  line.end = m_MousePos.value();
}

void Application::draw() {
  BeginMode2D(m_Camera);
  float angle = 360.f / m_Settings.symmetry * DEG2RAD;
  float thickness = m_Settings.lineThickness;
  float rotation = m_Settings.rotate ? m_Rotation * DEG2RAD : 0.f;
  if (m_Settings.scaleThicknessWithZoom)
    thickness /= m_Camera.zoom;

  for (int i = 0; i < m_LineIdx; i++) {
    const Line &line = m_Lines[i];
    for (int j = 0; j < m_Settings.symmetry; j++) {
      Vector2 lineStart = Vector2Rotate(line.start, angle * j + rotation);
      Vector2 lineEnd = Vector2Rotate(line.end, angle * j + rotation);
      DrawLineEx(lineStart, lineEnd, thickness, m_Settings.lineColor);

      if (m_Settings.enableReflection) {
        // reflected line
        Vector2 refLineStart =
            Vector2Multiply(lineStart, m_Settings.reflectionScale);
        Vector2 refLineEnd =
            Vector2Multiply(lineEnd, m_Settings.reflectionScale);
        DrawLineEx(refLineStart, refLineEnd, thickness, m_Settings.lineColor);
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
  ImGui::Text("Zoom: %.2f", m_Camera.zoom);
  ImGui::Separator();

  ImGui::Text("General");
  if (ImGui::SliderInt("Target FPS", &m_Settings.targetFps, 1, 240)) {
    SetTargetFPS(m_Settings.targetFps);
  }
  ImGui::SliderInt("Symmetry", &m_Settings.symmetry, 1, 32);
  ImGui::SliderFloat("Min. Mouse Distance", &m_Settings.minMouseDistance, 0.f,
                     32.f);
  ImGui::SliderFloat("Line Thickness", &m_Settings.lineThickness, 1.f, 10.f);
  ImGui::Checkbox("Scale Thickness with Zoom",
                  &m_Settings.scaleThicknessWithZoom);
  ImGui::Separator();

  ImGui::Text("Rotation");
  ImGui::Checkbox("Enable Rotation", &m_Settings.rotate);
  ImGui::SliderFloat("Speed", &m_Settings.rotationSpeed, 5.f, 500.f);
  ImGui::Separator();

  ImGui::Text("Reflection");
  ImGui::Checkbox("Enable Reflection", &m_Settings.enableReflection);
  ImGui::SliderFloat("Scale X", &m_Settings.reflectionScale.x, -1, 1);
  ImGui::SliderFloat("Scale Y", &m_Settings.reflectionScale.y, -1, 1);
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
