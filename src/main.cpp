#include "gui/gui.cpp"
#include "log/logger.hpp"
#include "raygui.h"
#include "raylib.h"
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui/file_dialog.h"
#define GUI_FILE_DIALOG_IMPLEMENTATION

using namespace fur;

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  fur::gui::set_config();

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
    GuiDrawText("Furrent", {gui::BORDER, gui::BORDER, 0, 50}, TEXT_ALIGN_LEFT,
                BLACK);
    auto button_file_dialog = GuiButton(
        {gui::W_WIDTH - gui::BORDER - 190, 10, 150, 30}, "#3# Open torrent");

    auto button_settings =
        GuiButton({gui::W_WIDTH - gui::BORDER - 30, 10, 30, 30}, "#141#");
    // Scroll panel
    GuiScrollPanel({gui::BORDER, 100, gui::W_WIDTH - gui::BORDER * 2,
                    gui::W_HEIGHT - 100 - gui::BORDER},
                   NULL,
                   {gui::BORDER, 50, gui::W_WIDTH - 150,
                    static_cast<float>(50 * torrents_state.torrents.size())},
                   &torrents_state.scroll);
    // Drawing torrents
    fur::gui::draw_torrents(&torrents_state);
    // Scroll panel head
    GuiDrawRectangle({gui::BORDER, 61, gui::W_WIDTH - gui::BORDER * 2, 40}, 1,
                     gui::BORDER_COLOR, gui::PRIMARY_COLOR);
    GuiDrawText("Torrents",
                {gui::BORDER, 61, gui::W_WIDTH - gui::BORDER * 2, 40},
                TEXT_ALIGN_CENTER, gui::TEXT_COLOR);
    // Scroll panel bottom
    GuiDrawRectangle(Rectangle{gui::BORDER, gui::W_HEIGHT - gui::BORDER,
                               gui::W_WIDTH, gui::W_WIDTH},
                     1, gui::BACKGROUND_COLOR, gui::BACKGROUND_COLOR);

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
