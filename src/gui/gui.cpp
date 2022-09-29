#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include "file_dialog.h"
#define GUI_FILE_DIALOG_IMPLEMENTATION
#include <sys/stat.h>

#include <string>
#include <vector>

namespace fur::gui {
// Config
const int BORDER = 5;
const int W_WIDTH = 800;
const int W_HEIGHT = 600;
// Colors
const int DARK_BROWN_HEX = static_cast<int>(0x876645FF);
const int ORANGE_HEX = static_cast<int>(0xD27607FF);
const int LIGHT_BACKGROUND_HEX = static_cast<int>(0xF6F2EEFF);
const int DARK_BACKGROUND_HEX = static_cast<int>(0xBA9978FF);
const int PRESSED_BACKGROUND_HEX = static_cast<int>(0xEDE9E0FF);
const int DOWNLOADING_COLOR_HEX = static_cast<int>(0xC9EFFEFF);
const int STOP_COLOR_HEX = static_cast<int>(0xFFAF7AFF);
const int DONE_COLOR_HEX = static_cast<int>(0xABF7B1FF);
const int ERROR_COLOR_HEX = static_cast<int>(0xFFA590FF);
// Raygui colors
const Color DARK_BROWN_COLOR = GetColor(DARK_BROWN_HEX);
const Color DARK_BACKGROUND_COLOR = GetColor(DARK_BACKGROUND_HEX);
const Color PRESSED_BACKGROUND_COLOR = GetColor(PRESSED_BACKGROUND_HEX);
const Color DIALOG_BACKGROUND_COLOR = Fade(PRESSED_BACKGROUND_COLOR, 0.85f);

enum TorrentState { INDEXING, STOP, DOWNLOAD, COMPLETED, ERROR };

struct TorrentGui {
  /// The index in the vector of torrents, automatically set by the gui
  int index{};
  /// The priority of the torrent
  int priority{};
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
  char *input_path{};
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
  /// True if the button save on change has been pressed
  bool update_priority = false;
  /// The priority during the change
  int input_priority{};
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

struct GuiErrorDialogState {
  bool show = false;
  /// Message to display in the dialog
  std::string error{};
};

/// Method to set the window configuration, fixed width and height, position and
/// text size
void setup_config() {
  InitWindow(W_WIDTH, W_HEIGHT, "Furrent");
  GuiSetStyle(DEFAULT, TEXT_SIZE, 15);
  GuiSetStyle(DEFAULT, BACKGROUND_COLOR, PRESSED_BACKGROUND_HEX);
  SetWindowPosition(0, 0);
  SetTargetFPS(60);
  GuiSetStyle(DEFAULT, LINE_COLOR, DARK_BACKGROUND_HEX);
  // Default color
  GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, LIGHT_BACKGROUND_HEX);
  GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, DARK_BROWN_HEX);
  GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, DARK_BROWN_HEX);
  // Pressed
  GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, PRESSED_BACKGROUND_HEX);
  GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, ORANGE_HEX);
  GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, DARK_BROWN_HEX);
  // Focused
  GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, LIGHT_BACKGROUND_HEX);
  GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ORANGE_HEX);
  GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, DARK_BROWN_HEX);
  GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, DARK_BROWN_HEX);
  // Disabled
  GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, DARK_BROWN_HEX);
  GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, DARK_BROWN_HEX);
  GuiSetFont(LoadFont("../assets/Righteous-Regular.ttf"));
}

/// Method to draw a single torrent in the scroll panel
void draw_torrent_item(const TorrentGui &torrent, float pos,
                       GuiTorrentDialogState &state) {
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
    case TorrentState::STOP:
      play = GuiButton({700, 110 + pos, 20, 20}, "#131#");
      // Set the color of the progress bar orangeLINK
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, STOP_COLOR_HEX);
      show_settings = GuiButton({730, 110 + pos, 20, 20}, "#140#");
      // Adding progress bar
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(torrent.progress) + "% ").c_str(), "#132#Stopped",
                     torrent.progress, 0, 100);
      break;
    case TorrentState::DOWNLOAD:
      play = GuiButton({700, 110 + pos, 20, 20}, "#131#");
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, DOWNLOADING_COLOR_HEX);
      show_settings = GuiButton({730, 110 + pos, 20, 20}, "#140#");
      // Adding progress bar
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(torrent.progress) + "% ").c_str(), "#6#Downloading",
                     torrent.progress, 0, 100);
      break;
    case TorrentState::INDEXING:
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, STOP_COLOR_HEX);
      // Adding progress bar
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(torrent.progress) + "% ").c_str(), "#173#Indexing",
                     torrent.progress, 0, 100);
      break;
    case TorrentState::COMPLETED:
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, DONE_COLOR_HEX);
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(torrent.progress) + "% ").c_str(), "#112#Downloaded",
                     torrent.progress, 0, 100);
      break;
    case TorrentState::ERROR:
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, ERROR_COLOR_HEX);
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(torrent.progress) + "% ").c_str(), "#113#Error",
                     torrent.progress, 0, 100);
      break;
    default:
      break;
  }

  auto delete_torrent = GuiButton({760, 110 + pos, 20, 20}, "#143#");
  // If one of the buttons is pressed, we update the state
  if (play || show_settings || delete_torrent) {
    state.play = play;
    state.show_settings = show_settings;
    state.delete_torrent = delete_torrent;
    state.torrent = torrent;
  }


  GuiLine({5, 145 + pos, 800 - 10, 1}, NULL);
}

/// Function to draw all the torrents based of the scroll state
void draw_torrents(GuiScrollTorrentState &state) {
  float pos = 5;
  int index = 0;
  for (auto &torr : state.torrents) {
    // If window has been scrolled down some elements are not drawn
    if (pos + 50 < abs(state.scroll.y)) {
      pos += 50;
      continue;
    }
    // Update the torrent index
    torr.index = index;
    // Draw the torrent
    draw_torrent_item(torr, pos + state.scroll.y, state.torrent_dialog_state);
    pos += 50;
    index++;
  }
}

/// Given the dialog setting state, it draws and manage the dialog
void settings_dialog(GuiSettingsDialogState &settings,
                     GuiErrorDialogState &dialog_error) {
  // If there are some errors don't do anything or the dialog is not open do
  // anything
  if (!settings.show || settings.updated_path) {
    return;
  }
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);
  // Add text input for the download folder
  auto result = GuiTextInputBox({250, 100, 300, 200}, "#198# Settings dialog",
                                "Change the download folder:", "Save;Dismiss",
                                settings.input_path, 255, NULL);
  // Some action pressed
  if (result == 0 || result == 1 || result == 2) {
    // If save action is pressed, we update the path
    if (result == 1) {
      // Check if the path exists and it is a directory
      struct stat info {};
      if (stat(settings.input_path, &info) != 0) {
        dialog_error.error = "The path does not exist";
      } else if (info.st_mode & S_IFDIR) {
        // It's a directory, try to update the path
        // TODO: check this operation
        settings.path = std::string{settings.input_path};
        settings.updated_path = true;
        // The dialog now is closed on main loop
      } else {
        dialog_error.error = "The path is not a directory";
      }
    } else {
      // Close or dismiss action
      // TODO: check this operation
      settings.input_path = settings.path.data();
      settings.show = false;
    }
  }
}

/// Given the torrent dialog state, it draws and manage the dialog
void torrent_dialog(GuiTorrentDialogState &torrent_dialog) {
  if (!torrent_dialog.show_settings) return;
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);
  // If torrent_settings is set, we draw a dialog with a slider to change the
  // priority
  auto result = GuiMessageBox({250, 100, 300, 200}, "#198# Torrent settings",
                              "Change the priority:", "Save;Dismiss");
  GuiSpinner({300, 210, 200, 30}, NULL, &torrent_dialog.input_priority, 0, 10,
             false);
  if (torrent_dialog.input_priority < 0) {
    torrent_dialog.input_priority = 0;
  }
  if (torrent_dialog.input_priority > 10) {
    torrent_dialog.input_priority = 10;
  }
  if (result == 0 || result == 1 || result == 2) {
    if (result == 1) {
      // Button save
      torrent_dialog.update_priority = true;
    } else {
      // Button close or dismiss
      torrent_dialog.input_priority = torrent_dialog.torrent.priority;
    }
    torrent_dialog.show_settings = false;
  }
}

/// Given the torrent dialog state, it draws and manage the dialog
void confirm_dialog(GuiConfirmDialogState &state) {
  if (!state.show) {
    return;
  }
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);

  auto response = GuiMessageBox(
      {200, 100, 400, 200}, "#198# Confirm action", state.message.c_str(),
      (state.confirm_button + ";" + state.cancel_button).c_str());

  // Se è stato cliccato un bottone chiudo la finestra e aggiorno lo stato
  if (response == 0 || response == 1 || response == 2) {
    state.show = false;
    state.clicked = true;
    state.confirm = response == 1;
  }
}

/// Given the error dialog state, it draws and manage the dialog
void error_dialog(GuiErrorDialogState &state) {
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);
  auto error_dialog =
      GuiMessageBox({250, 100, 300, 200}, "#198# Something went wrong",
                    state.error.c_str(), "Ok");
  if (error_dialog == 0 || error_dialog == 1) {
    // Clear the error
    state.error = "";
  }
}

// ------
// Events handles
// ------

/// Function to add a new torrent, it is called when the user clicks on the add
void add_torrent(GuiFileDialogState &file_dialog_state,
                 GuiScrollTorrentState &scroll_state,
                 GuiErrorDialogState &dialog_error,
                 bool (*add_torrent_callback)(GuiScrollTorrentState &,const std::string &, const std::string &)) {
  // If the file selected is not a torrent do nothing
  if (!IsFileExtension(file_dialog_state.fileNameText, ".torrent")) {
    file_dialog_state.SelectFilePressed = false;
    file_dialog_state.fileDialogActive = true;
    return;
  }
  // Call the callback to add the torrent to the list of torrents
  auto result = add_torrent_callback(scroll_state, file_dialog_state.realFileName, file_dialog_state.fileNameText);
  if (!result) {
    dialog_error.error = "Some error occurred";
    // Remove the action on the button
    file_dialog_state.SelectFilePressed = false;
    file_dialog_state.fileDialogActive = true;
    return;
  }
  // (Menaged in the callback )If the torrent was added successfully, we add it to the scroll state
  // TorrentGui torrent{0, 0, file_dialog_state.realFileName, STOP, 0};
  // scroll_state.torrents.push_back(torrent);
  // Close the dialog
  file_dialog_state.SelectFilePressed = false;
  file_dialog_state.fileDialogActive = false;
}

/// Function to remove a torrent, it is called when the user clicks on the trash
void remove_torrent(GuiScrollTorrentState &scroll_state,
                    GuiConfirmDialogState &confirm_dialog_state,
                    GuiErrorDialogState &dialog_error,
                    bool (*remove_torrent_callback)(const gui::TorrentGui &)) {
  // Open the confirm dialog if it is not already open or have been clicked
  if (!confirm_dialog_state.show && !confirm_dialog_state.clicked) {
    confirm_dialog_state.show = true;
    confirm_dialog_state.clicked = false;
    confirm_dialog_state.message =
        "Are you sure you want to delete the torrent?";
    confirm_dialog_state.confirm_button = "Yes";
    confirm_dialog_state.cancel_button = "No";
  }
  // If the user clicked on an option of the confirm dialog
  if (confirm_dialog_state.clicked) {
    // Confirm the deletion
    if (confirm_dialog_state.confirm) {
      // Call the callback to remove the torrent
      auto result =
          remove_torrent_callback(scroll_state.torrent_dialog_state.torrent);
      if (!result) {
        dialog_error.error = "Some error occurred";
        confirm_dialog_state.confirm = false;
        return;
      }
      // Remove the torrent from the list
      scroll_state.torrents.erase(
          scroll_state.torrents.begin() +
          scroll_state.torrent_dialog_state.torrent.index);
    }
    // Close the dialog
    confirm_dialog_state.show = false;
    confirm_dialog_state.confirm = false;
    confirm_dialog_state.clicked = false;
    // Close the torrent dialog
    scroll_state.torrent_dialog_state.delete_torrent = false;
  }
}

/// Function to update the furrent settings, it is called when the user clicks
/// on the settings icon
void update_settings(GuiSettingsDialogState &settings_dialog_state,
                     GuiErrorDialogState &dialog_error,
                     bool (*update_settings_callback)(const std::string &)) {
  if (settings_dialog_state.updated_path) {
    auto callback = update_settings_callback(settings_dialog_state.path);
    if (!callback) {
      dialog_error.error = "Some error occurred";
      settings_dialog_state.updated_path = false;
      return;
    }
    settings_dialog_state.updated_path = false;
    settings_dialog_state.show = false;
  }
}

/// Function to update the torrent priority, it is called when the user clicks
/// on the play/pause button
void update_torrent_state(
    GuiScrollTorrentState &scroll_state, GuiErrorDialogState &dialog_error,
    bool (*update_torrent_state_callback)(const gui::TorrentGui &)) {
  auto &torrent =
      scroll_state.torrents[scroll_state.torrent_dialog_state.torrent.index];
  auto callback = update_torrent_state_callback(torrent);
  if (!callback) {
    dialog_error.error = "Some error occurred";
    scroll_state.torrent_dialog_state.play = false;
    return;
  }
  switch (torrent.status) {
    case TorrentState::STOP:
      torrent.status = TorrentState::DOWNLOAD;
      break;
    case TorrentState::DOWNLOAD:
      torrent.status = TorrentState::STOP;
      break;
    default:
      break;
  }
  // Reset the action
  scroll_state.torrent_dialog_state.play = false;
}

/// Function to update the torrent priority, it is called when the user clicks
/// on the tools icon
void update_torrent_priority(
    GuiScrollTorrentState &scroll_state, GuiErrorDialogState &dialog_error,
    bool (*update_torrent_priority_callback)(const gui::TorrentGui &)) {
  auto &torrent =
      scroll_state.torrents[scroll_state.torrent_dialog_state.torrent.index];
  auto callback = update_torrent_priority_callback(torrent);
  if (!callback) {
    dialog_error.error = "Some error occurred";
    scroll_state.torrent_dialog_state.update_priority = false;
    scroll_state.torrent_dialog_state.show_settings = true;
    return;
  }
  torrent.priority = scroll_state.torrent_dialog_state.input_priority;
  // Reset the action
  scroll_state.torrent_dialog_state.update_priority = false;
}

}  // namespace fur::gui