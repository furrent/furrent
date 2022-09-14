#include "log/logger.hpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui/file_dialog.h"

void draw_list_element(std::string text, int pos){
  GuiDrawText(text.c_str(), Rectangle{10, static_cast<float>(300+pos), 0, 200}, TEXT_ALIGN_LEFT,
              BLACK);
}


int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  const int w_width = 800;
  const int w_height = 600;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(w_width, w_height, "Furrent");
  // Set larger text size
  GuiSetStyle(DEFAULT, TEXT_SIZE, 15);

  SetWindowPosition(0, 0);

  SetTargetFPS(60);

  int flag_true = 1;
  const int border = 5;
  //GetWorkingDirectory()
  GuiFileDialogState file_dialog =
      InitGuiFileDialog(550, 500, "/home/nicof/Desktop/univr/furrent/extra/", false,".torrent");
  file_dialog.fileTypeActive = true;

  std::vector<std::string> torrents{ "A", "B", "C", "D", "E", "F", "G", "H", "I", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
  Vector2 scroll = {0, 0};
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    // Title
    GuiDrawText("Furrent", Rectangle{border, border, 0, 50}, TEXT_ALIGN_LEFT,
                BLACK);
    auto button_file_dialog =
        GuiButton(Rectangle{w_width - border - 150, 0, 150, 30},
                  GuiIconText(ICON_FOLDER_OPEN, "Open torrent"));
    // Adding button to the right of the group box

    GuiScrollPanel(Rectangle{border, 50, w_width - border * 2, w_height-50},
                   "Torrents",
                    Rectangle{border, 50, w_width - 100, 1000},
                   &scroll);

    int pos = 0;
    for(auto t: torrents){
      draw_list_element(t, pos);
      pos+=100;
    }
    GuiDrawRectangle(
        Rectangle{border, w_height-2, w_width, w_width},
        1,
        GetColor(0xf5f5f5ff),
        GetColor(0xf5f5f5ff));


    // ------
    // Events
    // ------
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0) {
      scroll.y += wheelMove * 20;
    }

    // Button add torrent
    if (button_file_dialog) {
      file_dialog.fileDialogActive = true;
    }
    // Action on filedialog
    if (file_dialog.SelectFilePressed) {
      logger->info("Selected file: {}", file_dialog.fileNameText);
      if (IsFileExtension(file_dialog.fileNameText, ".torrent")) {
        logger->info("Selected file: {}", file_dialog.fileNameText);
      } else {
        file_dialog.SelectFilePressed = false;
        file_dialog.fileDialogActive = true;
      }
    }
    GuiFileDialog(&file_dialog);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}


