#include <iostream>

#include "log/logger.hpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui/file_dialog.h"

enum TorrentState { STOP, DOWNLOAD, COMPLETED, ERROR };

struct TorrentGui {
  std::string filename;
  TorrentState status;
  int progress;
};

struct GuiSettingsDialogState {
  bool show;
};

struct GuiTorrentDialogState {
  bool play{};
  bool show_settings{};
  bool remove{};
  TorrentGui torrent;
};

void draw_torrent_item(TorrentGui torrent, float pos, GuiTorrentDialogState* state) {
  // Drawing text
  auto name = "Name: " + torrent.filename;
  // Cut the name if it is too long
  if (name.size() > 20) {
    name = name.substr(0, 20) + "...";
  }
  GuiDrawText(name.c_str(), {20, 112 + pos, 0, 20}, TEXT_ALIGN_LEFT, GRAY);
  // Adding progress bar
  GuiProgressBar({275, 110 + pos, 300, 20}, (std::to_string(torrent.progress) + "% ").c_str(), NULL, torrent.progress, 0,
                 100);
  // Adding buttons actions
  auto play = GuiButton({700, 110 + pos, 20, 20}, GuiIconText(ICON_PLAYER_PLAY, NULL));
  auto show_settings = GuiButton({730, 110 + pos, 20, 20}, GuiIconText(ICON_TOOLS, NULL));
  auto remove = GuiButton({760, 110 + pos, 20, 20}, GuiIconText(ICON_BIN, NULL));
  // If one of the buttons is pressed, we update the state
  if(play || show_settings || remove) {
    state->play = play;
    state->show_settings = show_settings;
    state->remove = remove;
    state->torrent = torrent;
  }

  GuiLine({5, 145 + pos, 800 - 10, 1}, NULL);
}

// Function to draw all the torrents
void draw_torrents(const std::vector<TorrentGui>& torrents,
                   const Vector2 scoll, GuiTorrentDialogState* state) {
  float pos = 5;
  for (const auto& t : torrents) {
    // If window has been scrolled, we move the element
    if (pos + 50 < abs(scoll.y)) {
      pos += 50;
      continue;
    }
    draw_torrent_item(t, pos + scoll.y, state);
    pos += 50;
  }
}

// Show the furrent settings dialog and update the state
void settings_dialog(GuiSettingsDialogState* settings) {
  if (!settings->show) return;
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.85f));
  settings->show = !GuiWindowBox({200, 50, 400, 400}, "#198# Settings dialog");
  // TODO: Add furrent settings
}

// Show torrent dialog and update the state
void torrent_dialog(GuiTorrentDialogState* torrent){
  if (!torrent->show_settings) return;
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.85f));
  torrent->show_settings = !GuiWindowBox({200, 50, 400, 400}, "#198# Torrent dialog");
  // TODO: Add torrent settings
}

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  const int BORDER = 5;
  const int W_WIDTH = 800;
  const int W_HEIGHT = 600;

  std::vector<TorrentGui> torrents{
      {"A", TorrentState::COMPLETED, 100}, {"B", TorrentState::DOWNLOAD, 75},
      {"C", TorrentState::STOP, 50},       {"D", TorrentState::ERROR, 0},
      {"E", TorrentState::COMPLETED, 100}, {"F", TorrentState::DOWNLOAD, 75},
      {"G", TorrentState::STOP, 50},       {"H", TorrentState::ERROR, 0},
  };
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(W_WIDTH, W_HEIGHT, "Furrent");
  // Set larger text size
  GuiSetStyle(DEFAULT, TEXT_SIZE, 15);

  SetWindowPosition(0, 0);

  SetTargetFPS(60);

  // GetWorkingDirectory()
  GuiFileDialogState file_state = InitGuiFileDialog(
      550, 500, "/home/nicof/Desktop/univr/furrent/extra/", false, ".torrent");
  file_state.fileTypeActive = true;

  Vector2 scroll = {0, 0};

  GuiSettingsDialogState settings_state = {false};
  GuiTorrentDialogState torrent_state = {false, false, false, {"", TorrentState::STOP, 0}};
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Title
    GuiDrawText("Furrent", {BORDER, BORDER, 0, 50}, TEXT_ALIGN_LEFT, BLACK);
    auto button_file_dialog =
        GuiButton({W_WIDTH - BORDER - 190, 10, 150, 30},
                  GuiIconText(ICON_FOLDER_OPEN, "Open torrent"));

    auto button_settings = GuiButton({W_WIDTH - BORDER - 30, 10, 30, 30},
                                     GuiIconText(ICON_GEAR, NULL));
    // GuiScrollPanel
    GuiScrollPanel(
        {BORDER, 100, W_WIDTH - BORDER * 2, W_HEIGHT - 100 - BORDER}, NULL,
        {BORDER, 50, W_WIDTH - 150, static_cast<float>(50 * torrents.size())},
        &scroll);
    draw_torrents(torrents, scroll, &torrent_state);
    // List view header
    GuiDrawRectangle(
        {BORDER, 61, W_WIDTH - BORDER * 2, 40}, 1,
        Fade(GetColor(
                 GuiGetStyle(LISTVIEW, BORDER + (GuiState::STATE_FOCUSED * 3))),
             guiAlpha),
        GetColor(0xc9effeff));
    GuiDrawText("Torrents", {BORDER, 61, W_WIDTH - BORDER * 2, 40},
                TEXT_ALIGN_CENTER, GetColor(0x0492c7ff));
    // White panel to cut the scroll panel list
    GuiDrawRectangle(Rectangle{BORDER, W_HEIGHT - BORDER, W_WIDTH, W_WIDTH}, 1,
                     GetColor(0xf5f5f5ff), GetColor(0xf5f5f5ff));

    // ------
    // Events
    // ------
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0) {
      scroll.y += wheelMove * 20;
    }

    // Button add torrent
    if (button_file_dialog) {
      file_state.fileDialogActive = true;
    }
    // Button furrent settings
    if (button_settings) {
      settings_state.show = true;
    }
    // Action on filedialog
    if (file_state.SelectFilePressed) {
      logger->info("Selected file: {}", file_state.fileNameText);
      if (IsFileExtension(file_state.fileNameText, ".torrent")) {
        logger->info("Selected file: {}", file_state.fileNameText);
      } else {
        file_state.SelectFilePressed = false;
        file_state.fileDialogActive = true;
      }
    }
    GuiFileDialog(&file_state);
    settings_dialog(&settings_state);
    torrent_dialog(&torrent_state);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
