#include <iostream>

#include "gui/gui.cpp"
#include "log/logger.hpp"
#include "raygui.h"
#include "raylib.h"
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "gui/file_dialog.h"
#define GUI_FILE_DIALOG_IMPLEMENTATION

using namespace fur;

void add_new_torrent(GuiFileDialogState *file_dialog_state, fur::gui::GuiScrollTorrentState *scroll_state){
  // Add the torrent to the current list of torrents
  std::cout << "Adding new torrent: " << file_dialog_state->realFileName << std::endl;
  fur::gui::TorrentGui torrent{file_dialog_state->realFileName, fur::gui::STOP, 0};
  scroll_state->torrents.push_back(torrent);
  // Reset the dialog state
  file_dialog_state->SelectFilePressed = false;
  file_dialog_state->fileDialogActive = false;

}

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  // Set the window configuration
  fur::gui::setup_config();
  // Create all the states
  GuiFileDialogState file_state =
      InitGuiFileDialog(550, 500, GetWorkingDirectory(), false, ".torrent");
  fur::gui::GuiSettingsDialogState settings_state{};
  fur::gui::GuiScrollTorrentState scroll_state{
      Vector2{},
      {{"A", fur::gui::TorrentState::COMPLETED, 100},
       {"B", fur::gui::TorrentState::DOWNLOAD, 75},
       {"C", fur::gui::TorrentState::STOP, 50}},
      fur::gui::GuiTorrentDialogState{}};
  // Main loop
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
                    static_cast<float>(50 * scroll_state.torrents.size())},
                   &scroll_state.scroll);
    // Drawing torrents
    fur::gui::draw_torrents(&scroll_state);
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
      scroll_state.scroll.y += wheelMove * 20;
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
        add_new_torrent(&file_state, &scroll_state);
      } else {
        file_state.SelectFilePressed = false;
        file_state.fileDialogActive = true;
      }
    }
    // Update dialogs
    GuiFileDialog(&file_state);
    settings_dialog(&settings_state);
    torrent_dialog(&scroll_state.torrent_dialog_state);
    EndDrawing();
  }

  CloseWindow();
}
