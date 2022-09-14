#include <iostream>

#include "log/logger.hpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui/file_dialog.h"


void draw_list_element(std::string text, float pos){
  auto bounds = Rectangle{20, 130+pos, 0, 0};
  GuiDrawText(text.c_str(), bounds, TEXT_ALIGN_LEFT,
              BLACK);
  bounds.x = 5;
  bounds.y += 20;
  bounds.width = 800-23;
  GuiLine(bounds, NULL);

}

void load_torrents(const std::vector<std::string>&torrents ,const Vector2 scoll){

  float pos = 0;
  for (auto t:torrents){
    if(pos + 50 < abs(scoll.y)){
      pos+=50;
      continue;
    }
    draw_list_element(t,pos+scoll.y);
    pos+=50;

  }
}

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  const int BORDER = 5;
  const int w_width = 800;
  const int w_height = 600;

  std::vector<std::string> torrents{ "A", "B", "C", "D", "E", "F", "G", "H", "I", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(w_width, w_height, "Furrent");
  // Set larger text size
  GuiSetStyle(DEFAULT, TEXT_SIZE, 15);

  SetWindowPosition(0, 0);

  SetTargetFPS(60);

  int flag_true = 1;
  //GetWorkingDirectory()
  GuiFileDialogState file_dialog =
      InitGuiFileDialog(550, 500, "/home/nicof/Desktop/univr/furrent/extra/", false,".torrent");
  file_dialog.fileTypeActive = true;

  Vector2 scroll = {0, 0};
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    // Title
    GuiDrawText("Furrent", Rectangle{BORDER, BORDER, 0, 50}, TEXT_ALIGN_LEFT,
                BLACK);
    auto button_file_dialog =
        GuiButton(Rectangle{w_width - BORDER - 150, 10, 150, 30},
                  GuiIconText(ICON_FOLDER_OPEN, "Open torrent"));

    // Panel header of the list
    GuiScrollPanel(Rectangle{BORDER, 100, w_width - BORDER * 2, w_height-100-BORDER},
                   NULL,
                    Rectangle{BORDER, 50, w_width - 150, static_cast<float>(50*torrents.size())},
                   &scroll);
    load_torrents(torrents, scroll);
    GuiDrawRectangle(
        Rectangle{BORDER, 55, w_width - BORDER * 2, 50},
        0,
        DARKBLUE,
        SKYBLUE);


    // White panel to cut the scroll panel list
    GuiDrawRectangle(
        Rectangle{BORDER, w_height-BORDER, w_width, w_width},
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


