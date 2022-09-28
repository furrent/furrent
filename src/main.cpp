#include "gui/gui.cpp"
#include "log/logger.hpp"
#include "raylib.h"

using namespace fur;

/// Function to handle a add new torrent, this will be called when the user
/// clicks the add new torrent button inside the file chooser dialog.
bool add_torrent_callback(const std::string &path) {
  auto logger = spdlog::get("custom");
  logger->info("Adding new torrent: {}", path);
  return true;
}

bool remove_torrent_callback(const gui::TorrentGui &torrent) {
  auto logger = spdlog::get("custom");
  logger->info("Removing torrent: {}", torrent.filename);
  return true;
}

bool update_settings_callback(const std::string &path) {
  auto logger = spdlog::get("custom");
  logger->info("Updating settings: {}", path);
  return true;
}

bool update_torrent_callback(const gui::TorrentGui &torrent) {
  auto logger = spdlog::get("custom");
  logger->info("Updating torrent: {}", torrent.filename);
  return true;
}

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  // Set the window configuration
  fur::gui::setup_config();
  // Create all the states
  GuiFileDialogState file_state =
      InitGuiFileDialog(550, 500, GetWorkingDirectory(), false, ".torrent");
  fur::gui::GuiSettingsDialogState settings_state{
      false, false, const_cast<char *>(GetWorkingDirectory()),
      GetWorkingDirectory(), ""};
  fur::gui::GuiScrollTorrentState scroll_state{
      Vector2{},
      {{0, 0, "A", fur::gui::TorrentState::COMPLETED, 100},
       {0, 0, "B", fur::gui::TorrentState::DOWNLOAD, 50},
       {0, 0, "C", fur::gui::TorrentState::STOP, 50},
       {0, 0, "D", fur::gui::TorrentState::ERROR, 50}},
      fur::gui::GuiTorrentDialogState{}};
  fur::gui::GuiConfirmDialogState confirm_dialog_state{};
  fur::gui::GuiErrorDialogState error_dialog_state{};
  auto furrent_logo = LoadTexture("../assets/Furrent.png");

  // Main loop
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    // ------
    // Drawing the page
    // ------
    // Add furrent image

    DrawTexture(furrent_logo, gui::BORDER, gui::BORDER, WHITE);
    // Title
    // Increase the font for the title
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
    GuiDrawText("Furrent", {65, gui::BORDER, 0, 50}, TEXT_ALIGN_LEFT,
                fur::gui::DARK_BROWN_COLOR);
    // Reset the font
    GuiSetStyle(DEFAULT, TEXT_SIZE, 15);
    auto button_file_dialog = GuiButton(
        {gui::W_WIDTH - gui::BORDER - 190, 15, 150, 30}, "#3# Open torrent");

    auto button_settings =
        GuiButton({gui::W_WIDTH - gui::BORDER - 30, 15, 30, 30}, "#141#");
    // Scroll panel
    GuiScrollPanel({gui::BORDER, 100, gui::W_WIDTH - gui::BORDER * 2,
                    gui::W_HEIGHT - 100 - gui::BORDER},
                   NULL,
                   {gui::BORDER, 50, gui::W_WIDTH - 150,
                    static_cast<float>(50 * scroll_state.torrents.size())},
                   &scroll_state.scroll);
    // Drawing torrents
    fur::gui::draw_torrents(scroll_state);
    // Scroll panel head
    GuiDrawRectangle({gui::BORDER, 61, gui::W_WIDTH - gui::BORDER * 2, 40}, 1,
                     fur::gui::DARK_BROWN_COLOR,
                     fur::gui::DARK_BACKGROUND_COLOR);
    GuiDrawText("Torrents",
                {gui::BORDER, 61, gui::W_WIDTH - gui::BORDER * 2, 40},
                TEXT_ALIGN_CENTER, fur::gui::DARK_BROWN_COLOR);
    // Scroll panel bottom
    GuiDrawRectangle(Rectangle{gui::BORDER, gui::W_HEIGHT - gui::BORDER,
                               gui::W_WIDTH, gui::W_WIDTH},
                     1, gui::PRESSED_BACKGROUND_COLOR,
                     gui::PRESSED_BACKGROUND_COLOR);

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
      fur::gui::add_torrent(file_state, scroll_state, error_dialog_state,
                            &add_torrent_callback);
    }
    // Furrent settings dialog
    if (settings_state.show) {
      fur::gui::update_settings(settings_state, error_dialog_state,
                                &update_settings_callback);
    }
    // Button play/pause on torrent
    if (scroll_state.torrent_dialog_state.play) {
      fur::gui::update_torrent_state(scroll_state, error_dialog_state,
                                     &update_torrent_callback);
    }
    // Button settings on torrent
    if (scroll_state.torrent_dialog_state.update_priority) {
      fur::gui::update_torrent_priority(scroll_state, error_dialog_state,
                                        &update_torrent_callback);
    }
    // Button remove torrent
    if (scroll_state.torrent_dialog_state.delete_torrent) {
      fur::gui::remove_torrent(scroll_state, confirm_dialog_state,
                               error_dialog_state, &remove_torrent_callback);
    }
    // Update dialogs

    GuiFileDialog(&file_state);
    if (!error_dialog_state.error.empty()) {
      error_dialog(error_dialog_state);
    } else {
      torrent_dialog(scroll_state.torrent_dialog_state);
      settings_dialog(settings_state, error_dialog_state);
      confirm_dialog(confirm_dialog_state);
    }

    EndDrawing();
  }

  CloseWindow();
}
