#include <iostream>

#include "log/logger.hpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui/file_dialog.h"

enum TorrentState{
  STOP,
  DOWNLOAD,
  COMPLETED,
  ERROR
};

struct TorrentGui {
  std::string filename;
  TorrentState status;
  int progress;

};


void draw_list_element(TorrentGui torrent, float pos){
  // Drawing text
  auto name = "Name: "+ torrent.filename;
  // Cut the name if it is too long
  if (name.size() > 20){
      name = name.substr(0, 20) + "...";
  }
  GuiDrawText(name.c_str(), {20, 112+pos, 0, 20}, TEXT_ALIGN_LEFT,
              GRAY);
  // Adding progress bar
  const char* text = (std::to_string(torrent.progress) + "% ").c_str();
  GuiProgressBar({275, 110+pos, 300, 20}, text, NULL, torrent.progress, 0, 100);
  // Adding buttons actions
  GuiButton({700, 110 + pos, 20, 20}, GuiIconText(ICON_PLAYER_PLAY, NULL));
  GuiButton({730, 110 + pos, 20, 20}, GuiIconText(ICON_TOOLS, NULL));
  GuiButton({760, 110 + pos, 20, 20}, GuiIconText(ICON_BIN, NULL));
  GuiLine({5, 145+pos, 800-10, 1}, NULL);

}

void load_torrents(const std::vector<TorrentGui>&torrents ,const Vector2 scoll){

  float pos = 5;
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

  std::vector<TorrentGui> torrents{
      {"A", TorrentState::COMPLETED, 100},
      {"B", TorrentState::DOWNLOAD, 75},
      {"C", TorrentState::STOP, 50},
      {"D", TorrentState::ERROR, 0},
      {"E", TorrentState::COMPLETED, 100},
      {"F", TorrentState::DOWNLOAD, 75},
      {"G", TorrentState::STOP, 50},
      {"H", TorrentState::ERROR, 0},
  };
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
    GuiDrawText("Furrent", {BORDER, BORDER, 0, 50}, TEXT_ALIGN_LEFT,
                BLACK);
    auto button_file_dialog =
        GuiButton({w_width - BORDER - 190, 10, 150, 30},
                  GuiIconText(ICON_FOLDER_OPEN, "Open torrent"));

    GuiButton({w_width - BORDER - 30, 10, 30, 30},
              GuiIconText(ICON_GEAR, NULL));


    GuiScrollPanel({BORDER, 100, w_width - BORDER * 2, w_height-100-BORDER},
                   NULL,
                    {BORDER, 50, w_width - 150, static_cast<float>(50*torrents.size())},
                   &scroll);
    load_torrents(torrents, scroll);
    // Panel header of the list
    GuiDrawRectangle({BORDER, 61, w_width - BORDER * 2, 40},
                     1,
                     Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + (GuiState::STATE_FOCUSED*3))), guiAlpha),
                     GetColor(0xc9effeff));
    GuiDrawText("Torrents", {BORDER, 61, w_width - BORDER * 2, 40}, TEXT_ALIGN_CENTER,
                GetColor(0x0492c7ff));
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


