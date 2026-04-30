#pragma once

#include "Line.hpp"
#include "raylib.h"
#include <array>
#include <optional>

struct ApplicationSettings {
  Color lineColor = RAYWHITE;
  Color backgroundColor = BLACK;
  Vector2 reflectionScale{1.f, -1.f};
  int screenWidth = 1200;
  int screenHeight = 900;
  int targetFps = 60;
  int symmetry = 6;
  float lineThickness = 3.f;
  float minMouseDistance = 2.f;
  bool enableReflection = true;

  ApplicationSettings() = default;
  ApplicationSettings(int screenWidth, int screenHeight) {
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
  }
};

class Application {
public:
  Application();

  void run();

private:
  void initCamera();
  void handleInput();
  void update(float dt);
  void draw();
  void drawSettings();
  void reset();
  static void colorToFloat4(Color c, float *float4);
  static Color float4ToColor(float *float4);

private:
  static constexpr int MAX_LINES = 10000;
  std::array<Line, MAX_LINES> m_Lines{};
  int m_LineIdx{};

  Camera2D m_Camera{};
  ApplicationSettings m_Settings{};

  Vector2 m_Offset{};
  std::optional<Vector2> m_MousePos;

  bool m_ShowSettings = false;
};
