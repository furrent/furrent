#include "gui/gui.cpp"
#include "log/logger.hpp"
#include <platform/io.hpp>
#include <raylib/raylib.h>
#include "furrent.hpp"

using namespace fur;

bool remove_torrent_callback(const TorrentSnapshot& snapshot) {
  auto logger = spdlog::get("custom");
  logger->info("Removing torrent: {}", snapshot.filename);
  return true;
}

bool update_settings_callback(const std::string &path) {
  auto logger = spdlog::get("custom");
  logger->info("Updating settings: {}", path);
  return true;
}

bool update_torrent_callback(const TorrentSnapshot& snapshot) {
  auto logger = spdlog::get("custom");
  logger->info("Updating torrent: {}", snapshot.filename);
  return true;
}

int main(int argc, char* argv[]) {

  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  Furrent furrent;
  //furrent.add_torrent("../extra/debian-11.5.0-amd64-i386-netinst.iso.torrent");
  //while(true);

#if 1

  const std::string argv_0(argv[0]);
  auto last_slash = argv_0.find_last_of('/');
  const std::string path_base = argv_0.substr(0, last_slash + 1);

  // Create output directory
  fur::platform::io::create_subfolders(path_base, { "output" });

  // Set the window configuration
  fur::gui::setup_config();
  
  // Create all the states
  GuiFileDialogState file_state =
      InitGuiFileDialog(550, 500, GetWorkingDirectory(), false, ".torrent");
  
  fur::gui::GuiSettingsDialogState settings_state{
      false, false, const_cast<char *>(GetWorkingDirectory()),
      GetWorkingDirectory(), ""};

  fur::gui::GuiScrollTorrentState scroll_state;
  fur::gui::GuiConfirmDialogState confirm_dialog_state;
  fur::gui::GuiErrorDialogState error_dialog_state;

  auto furrent_logo = LoadTexture("../assets/Furrent.png");

  // Main loop
  while (!WindowShouldClose()) {

    // Update furrent status
    for(auto& snapshot : furrent.get_torrents_snapshot()) {
      auto it = scroll_state.torrents.find(snapshot.uid);
      if (it != scroll_state.torrents.end())
        it->second = snapshot;
    }
    
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
        [&] (const std::string &fine_name, const std::string &file_path) -> bool {
          logger->info("Adding new torrent: {}", file_path);
          
          size_t uid = furrent.add_torrent(file_path);
          scroll_state.torrents.emplace(uid, *furrent.get_snapshot(uid));
          
          return true;
        });
    }

    // Button remove torrent
    if (scroll_state.torrent_dialog_state.delete_torrent) {
      fur::gui::remove_torrent(scroll_state, confirm_dialog_state, error_dialog_state, 
        [&] (const TorrentSnapshot& snapshot) -> bool {
          logger->info("Removing torrent: {}", snapshot.filename);
          
          furrent.remove_torrent(snapshot.uid);
          scroll_state.torrents.erase(snapshot.uid);
          
          return true;
        });
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
#endif

}
