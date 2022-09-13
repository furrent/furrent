#include "raylib.h"
#include "log/logger.hpp"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui/file_dialog.h"


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
  GuiFileDialogState file_dialog = InitGuiFileDialog(550, 500, GetWorkingDirectory(),false);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    // Title
    GuiDrawText("Furrent", Rectangle{border,border, 0, 50}, TEXT_ALIGN_LEFT, BLACK);

    // Button add torrent
    if (GuiButton(Rectangle{w_width-border-150, 10, 150, 30},
                  GuiIconText(ICON_FOLDER_OPEN, "Open torrent"))) {
      file_dialog.fileDialogActive = true;
    }

    // Adding button to the right of the group box
    GuiListView(Rectangle{ border, 50, w_width-border*2, w_height}, "Torrent list",
                &flag_true, flag_true);
    // Action on filedialog
    if(file_dialog.SelectFilePressed){
      if(IsFileExtension(file_dialog.fileNameText, ".torrent")){
        logger->info("Selected file: {}", file_dialog.fileNameText);
        file_dialog.fileDialogActive = false;
      }
    }
    GuiFileDialog(&file_dialog);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
