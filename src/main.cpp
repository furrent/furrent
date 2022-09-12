#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "log/logger.hpp"

#include "raygui.h"

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  const int screenWidth = 800;
  const int screenHeight = 600;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(screenWidth, screenHeight, "Furrent");

  SetWindowPosition(0, 0);

  SetTargetFPS(60);

  int counter = 0;

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (GuiButton(Rectangle{10, 10, screenWidth - 10, 100}, "I am Furrent!")) {
      logger->info("button pressed");
      counter++;
    }

    auto counter_string = std::to_string(counter);
    DrawText(counter_string.c_str(), 10, 150, 10, GRAY);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
