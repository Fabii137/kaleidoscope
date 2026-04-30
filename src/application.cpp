#include "Application.hpp"
#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlImGui.h"

Application::Application() {}

void Application::run() {
  InitWindow(m_Settings.screenWidth, m_Settings.screenHeight, "Kaleidoscope");
  SetTargetFPS(m_Settings.targetFps);
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  m_Center = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
  initCamera();

  rlImGuiSetup(true);

  while (!WindowShouldClose()) {
    handleInput();

    float dt = GetFrameTime();
    update(dt);

    BeginDrawing();
    ClearBackground(BLACK);
    draw();
    EndDrawing();
  }

  CloseWindow();
}

void Application::initCamera() {
  m_Camera.target = {0.f, 0.f};
  m_Camera.offset = m_Center;
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
    m_MousePos = MOUSE_POS_UNINITIALIZED;
  }

  if (IsWindowResized()) {
    int scrWidth = GetScreenWidth();
    int scrHeight = GetScreenHeight();
    m_Settings.screenWidth = scrWidth;
    m_Settings.screenHeight = scrHeight;
    m_Center = {scrWidth / 2.f, scrHeight / 2.f};
    m_Camera.offset = m_Center;
  }
}

void Application::update(float dt) {
  if (ImGui::GetIO().WantCaptureMouse)
    return;

  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) || !IsCursorOnScreen() ||
      m_LineIdx >= MAX_LINES)
    return;

  Vector2 prevMousePos = m_MousePos;
  m_MousePos = GetMousePosition();

  if (prevMousePos == MOUSE_POS_UNINITIALIZED)
    return;

  Line &line = m_Lines[m_LineIdx++];
  line.start = Vector2Subtract(prevMousePos, m_Center);
  line.end = Vector2Subtract(m_MousePos, m_Center);
}

void Application::draw() {
  BeginMode2D(m_Camera);
  for (int i = 0; i < m_LineIdx; i++) {
    const Line &line = m_Lines[i];
    for (int j = 0; j < m_Settings.symmetry; j++) {
      Vector2 lineStart = Vector2Rotate(line.start, m_Angle * j * DEG2RAD);
      Vector2 lineEnd = Vector2Rotate(line.end, m_Angle * j * DEG2RAD);
      DrawLineEx(lineStart, lineEnd, m_Settings.lineThickness, RAYWHITE);

      // reflected line
      Vector2 refLineStart = Vector2Multiply(lineStart, m_ScaleVector);
      Vector2 refLineEnd = Vector2Multiply(lineEnd, m_ScaleVector);
      DrawLineEx(refLineStart, refLineEnd, m_Settings.lineThickness, RAYWHITE);
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

  if (ImGui::SliderInt("Symmetry", &m_Settings.symmetry, 1, 32)) {
    m_Angle = 360.f / m_Settings.symmetry;
  }

  ImGui::SliderFloat("Line Width", &m_Settings.lineThickness, 1.f, 10.f);

  if (ImGui::Button("Reset (R)")) {
    reset();
  }

  ImGui::End();
  rlImGuiEnd();
}

void Application::reset() {
  m_LineIdx = 0;
  m_MousePos = MOUSE_POS_UNINITIALIZED;
}
