#include "gui/gui.cpp"
#include "log/logger.hpp"
#include "raygui.h"
#include "raylib.h"
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui/file_dialog.h"
#define GUI_FILE_DIALOG_IMPLEMENTATION

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  const int BORDER = 5;
  const int W_WIDTH = 800;
  const int W_HEIGHT = 600;
  const Color BORDER_COLOR = GetColor(0x368BAF);
  const Color PRIMARY_COLOR = GetColor(0xc9effeff);
  const Color TEXT_COLOR = GetColor(0x0492c7ff);

  fur::gui::set_config(W_WIDTH, W_HEIGHT);

  GuiFileDialogState file_state =
      InitGuiFileDialog(550, 500, GetWorkingDirectory(), false, ".torrent");

  fur::gui::GuiSettingsDialogState settings_state{};
  fur::gui::GuiScrollTorrentState torrents_state{
      Vector2{},
      {
          {"A", fur::gui::TorrentState::COMPLETED, 100},
          {"B", fur::gui::TorrentState::DOWNLOAD, 75},
          {"C", fur::gui::TorrentState::STOP, 50},
          {"D", fur::gui::TorrentState::ERROR, 0},
          {"E", fur::gui::TorrentState::COMPLETED, 100},
          {"F", fur::gui::TorrentState::DOWNLOAD, 75},
          {"G", fur::gui::TorrentState::STOP, 50},
          {"H", fur::gui::TorrentState::ERROR, 0},
      },
      fur::gui::GuiTorrentDialogState{}};
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    // ------
    // Drawing the page
    // ------
    // Title
    GuiDrawText("Furrent", {BORDER, BORDER, 0, 50}, TEXT_ALIGN_LEFT, BLACK);
    auto button_file_dialog =
        GuiButton({W_WIDTH - BORDER - 190, 10, 150, 30}, "#3# Open torrent");

    auto button_settings =
        GuiButton({W_WIDTH - BORDER - 30, 10, 30, 30}, "#141#");
    // Scroll panel
    GuiScrollPanel({BORDER, 100, W_WIDTH - BORDER * 2, W_HEIGHT - 100 - BORDER},
                   NULL,
                   {BORDER, 50, W_WIDTH - 150,
                    static_cast<float>(50 * torrents_state.torrents.size())},
                   &torrents_state.scroll);
    // Drawing torrents
    fur::gui::draw_torrents(&torrents_state);
    // Scroll panel head
    GuiDrawRectangle({BORDER, 61, W_WIDTH - BORDER * 2, 40}, 1, BORDER_COLOR,
                     PRIMARY_COLOR);
    GuiDrawText("Torrents", {BORDER, 61, W_WIDTH - BORDER * 2, 40},
                TEXT_ALIGN_CENTER, TEXT_COLOR);
    // Scroll panel bottom
    GuiDrawRectangle(Rectangle{BORDER, W_HEIGHT - BORDER, W_WIDTH, W_WIDTH}, 1,
                     GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)),
                     GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    // ------
    // Events
    // ------

    // Event caught for updating the scroll panel
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0) {
      torrents_state.scroll.y += wheelMove * 20;
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
      if (IsFileExtension(file_state.fileNameText, ".torrent")) {
        logger->info("Selected file: {}", file_state.fileNameText);
        // TODO: load new torrent from path
      } else {
        file_state.SelectFilePressed = false;
        file_state.fileDialogActive = true;
      }
    }
    // Update dialogs
    GuiFileDialog(&file_state);
    settings_dialog(&settings_state);
    torrent_dialog(&torrents_state.torrent_dialog_state);
    EndDrawing();
  }

  CloseWindow();
}
