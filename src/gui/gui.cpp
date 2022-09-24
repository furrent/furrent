#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION
#include <sys/stat.h>

#include <string>
#include <vector>

namespace fur::gui {

const int BORDER = 5;
const int W_WIDTH = 800;
const int W_HEIGHT = 600;
const unsigned int PRIMARY_COLOR_HEX = 0xc9effeff;
const unsigned int DOWNLOADING_COLOR_HEX = 0xffaf7aff;
const unsigned int DONE_COLOR_HEX = 0xabf7b1ff;
const unsigned int ERROR_COLOR_HEX = 0xffa590ff;
const unsigned int TEXT_COLOR_HEX = 0x0492c7ff;
const unsigned int BACKGROUND_COLOR_HEX = 0xf5f5f5ff;
const Color BORDER_COLOR = GetColor(0x368bafff);
const Color PRIMARY_COLOR = GetColor(PRIMARY_COLOR_HEX);
const Color TEXT_COLOR = GetColor(TEXT_COLOR_HEX);
const Color BACKGROUND_COLOR = GetColor(BACKGROUND_COLOR_HEX);
const Color DIALOG_BACKGROUND_COLOR = Fade(BACKGROUND_COLOR, 0.85f);

enum TorrentState { STOP, DOWNLOAD, COMPLETED, ERROR };

struct TorrentGui {
  /// The index in the vector of torrents, automatically set by the gui
  int index{};
  std::string filename{};
  TorrentState status = STOP;
  /// Used to draw the progress bar from 0 to 100
  int progress{};
};

/// State of the settings dialog
struct GuiSettingsDialogState {
  bool show = false;
  /// True if the path has been updated and the dialog should be closed
  /// used in the main loop to retrieve the new path
  bool updated_path = false;
  /// The path that is currently displayed in the dialog
  char* input_path{};
  /// The real path that is used by the program
  std::string path{};
  /// If the error is initialized it will be displayed in a new dialog
  std::string error{};
};

struct GuiTorrentDialogState {
  bool show_settings = false;
  /// True if the button play has been pressed
  bool play = false;
  /// True if the button stop has been pressed
  bool delete_torrent = false;
  TorrentGui torrent{};
};

struct GuiScrollTorrentState {
  Vector2 scroll{};
  std::vector<TorrentGui> torrents{};
  GuiTorrentDialogState torrent_dialog_state{};
};

struct GuiConfirmDialogState {
  bool show = false;
  /// True if the user clicked on the confirm button
  bool confirm = false;
  /// True if the user has clicked on a button (confirm, cancel of exit)
  bool clicked = false;
  /// Message to display in the dialog
  std::string message{};
  /// Text of the button yes
  std::string confirm_button{};
  /// Text of the button no
  std::string cancel_button{};
};

/// Method to set the window configuration, fixed width and height, position and
/// text size
void setup_config() {
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(W_WIDTH, W_HEIGHT, "Furrent");
  GuiSetStyle(DEFAULT, TEXT_SIZE, 15);
  SetWindowPosition(0, 0);
  SetTargetFPS(60);
}

/// Method to draw a single torrent in the scroll panel
void draw_torrent_item(const fur::gui::TorrentGui& torrent, float pos,
                       fur::gui::GuiTorrentDialogState* state) {
  // Drawing text
  auto name = "Name: " + torrent.filename;
  // Cut the name if it is too long
  if (name.size() > 20) {
    name = name.substr(0, 20) + "...";
  }
  // Draw the name
  GuiDrawText(name.c_str(), {20, 112 + pos, 0, 20}, TEXT_ALIGN_LEFT, GRAY);

  // Adding buttons actions
  bool play = false;
  bool show_settings = false;
  switch (torrent.status) {
    case fur::gui::STOP:
      play = GuiButton({700, 110 + pos, 20, 20}, "#132#");
      // Set the color of the progress bar orange
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, PRIMARY_COLOR_HEX);
      show_settings = GuiButton({730, 110 + pos, 20, 20}, "#140#");
      break;
    case fur::gui::DOWNLOAD:
      play = GuiButton({700, 110 + pos, 20, 20}, "#131#");
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, DOWNLOADING_COLOR_HEX);
      show_settings = GuiButton({730, 110 + pos, 20, 20}, "#140#");
      break;
    case COMPLETED:
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, DONE_COLOR_HEX);
      break;
    case ERROR:
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, ERROR_COLOR_HEX);
      break;
    default:
      break;
  }

  auto delete_torrent = GuiButton({760, 110 + pos, 20, 20}, "#143#");
  // If one of the buttons is pressed, we update the state
  if (play || show_settings || delete_torrent) {
    state->play = play;
    state->show_settings = show_settings;
    state->delete_torrent = delete_torrent;
    state->torrent = torrent;
  }
  // Adding progress bar
  GuiProgressBar({275, 110 + pos, 300, 20},
                 (std::to_string(torrent.progress) + "% ").c_str(), NULL,
                 torrent.progress, 0, 100);

  GuiLine({5, 145 + pos, 800 - 10, 1}, NULL);
}

/// Function to draw all the torrents based of the scroll state
void draw_torrents(fur::gui::GuiScrollTorrentState* state) {
  float pos = 5;
  int index = 0;
  for (auto& torr : state->torrents) {
    // If window has been scrolled down some elements are not drawn
    if (pos + 50 < abs(state->scroll.y)) {
      pos += 50;
      continue;
    }
    // Update the torrent index
    torr.index = index;
    // Draw the torrent
    draw_torrent_item(torr, pos + state->scroll.y,
                      &state->torrent_dialog_state);
    pos += 50;
    index++;
  }
}

/// Given the dialog setting state, it draws and manage the dialog
void settings_dialog(fur::gui::GuiSettingsDialogState* settings) {
  if (!settings->show || settings->updated_path) {
    return;
  }
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);
  // If there are some errors
  if (!settings->error.empty()) {
    auto error_dialog =
        GuiMessageBox({250, 100, 300, 200}, "#198# Something went wrong",
                      settings->error.c_str(), "Ok");
    if (error_dialog == 0 || error_dialog == 1) {
      // Clear the error
      settings->error = "";
    }
    return;
  }
  // Add text input for the download folder
  auto result = GuiTextInputBox({250, 100, 300, 200}, "#198# Settings dialog",
                                "Change the download folder:", "Save;Dismiss",
                                settings->input_path, 255, NULL);
  // Some action pressed
  if (result == 0 || result == 1 || result == 2) {
    // If save action is pressed, we update the path
    if (result == 1) {
      // Check if the path exists and it is a directory
      struct stat info {};
      if (stat(settings->input_path, &info) != 0) {
        settings->error = "The path does not exist";
      } else if (info.st_mode & S_IFDIR) {
        // It's a directory, try to update the path
        // TODO: check this operation
        settings->path = std::string{settings->input_path};
        settings->updated_path = true;
        // The dialog now is closed on main loop
      } else {
        settings->error = "The path is not a directory";
      }
    } else {
      // Close or dismiss action
      // TODO: check this operation
      settings->input_path = settings->path.data();
      settings->show = false;
    }
  }
}

/// Given the torrent dialog state, it draws and manage the dialog
void torrent_dialog(fur::gui::GuiTorrentDialogState* torrent) {
  if (!torrent->show_settings) return;
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);
  torrent->show_settings =
      !GuiWindowBox({200, 50, 400, 400}, "#198# Torrent dialog");
  // TODO: Add torrent settings
}

/// Given the torrent dialog state, it draws and manage the dialog
void confirm_dialog(fur::gui::GuiConfirmDialogState* state) {
  if (!state->show) {
    return;
  }
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);

  auto response = GuiMessageBox(
      {200, 100, 400, 200}, "#198# Confirm action", state->message.c_str(),
      (state->confirm_button + ";" + state->cancel_button).c_str());

  // Se è stato cliccato un bottone chiudo la finestra e aggiorno lo stato
  if (response == 0 || response == 1 || response == 2) {
    state->show = false;
    state->clicked = true;
    state->confirm = response == 1;
  }
}

}  // namespace fur::gui