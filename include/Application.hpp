#pragma once

#include "Line.hpp"
#include "raylib.h"
#include <array>
#include <limits>

constexpr Vector2 MOUSE_POS_UNINITIALIZED = {std::numeric_limits<float>::min()};

struct ApplicationSettings {
  int screenWidth = 1200;
  int screenHeight = 900;
  int targetFps = 60;
  int symmetry = 6;
  float lineThickness = 3.f;
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

private:
  static constexpr int MAX_LINES = 10000;
  std::array<Line, MAX_LINES> m_Lines{};
  int m_LineIdx{};

  Camera2D m_Camera{};
  ApplicationSettings m_Settings{};

  Vector2 m_Center{};
  Vector2 m_ScaleVector{1.f, -1.f};

  Vector2 m_MousePos = MOUSE_POS_UNINITIALIZED;
  float m_Angle = 360.f / m_Settings.symmetry;

  bool m_ShowSettings = false;
};
