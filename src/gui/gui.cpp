
#define RAYGUI_IMPLEMENTATION
#include <raylib/raygui.h>
#undef RAYGUI_IMPLEMENTATION

#define GUI_FILE_DIALOG_IMPLEMENTATION
#include <raylib/file_dialog.h>
#define GUI_FILE_DIALOG_IMPLEMENTATION

#include <sys/stat.h>

#include <string>
#include <vector>
#include <unordered_set>
#include <functional>

#include <furrent.hpp>
#include <gui/colors.hpp>

namespace fur::gui {

const int BORDER = 5;
const int W_WIDTH = 800;
const int W_HEIGHT = 600;

// ===============================================================
// Callbacks

using cbfn_torrent_insert = std::function<bool(const std::string&, const std::string&)>;
using cbfn_torrent_remove = std::function<bool(const TorrentGuiData&)>;
using cbfn_torrent_update = std::function<bool(const TorrentGuiData&)>;
using cbfn_setting_update = std::function<bool(const std::string&)>;

// ===============================================================

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
  TorrentGuiData torrent;
};

struct GuiScrollTorrentState {
  Vector2 scroll;
  std::unordered_map<size_t, TorrentGuiData> torrents;
  GuiTorrentDialogState torrent_dialog_state;
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
void draw_torrent_item(const TorrentGuiData &snapshot, float pos,
                       GuiTorrentDialogState &state) {
  
  #if 1
  // Drawing text
  auto name = "Name: " + snapshot.filename;
  // Cut the name if it is too long
  if (name.size() > 20) {
    name = name.substr(0, 20) + "...";
  }
  // Draw the name
  GuiDrawText(name.c_str(), {20, 112 + pos, 0, 20}, TEXT_ALIGN_LEFT, GRAY);
  int progress = static_cast<int>((static_cast<float>(snapshot.pieces_processed) / snapshot.pieces_count) * 100);

  // Adding buttons actions
  bool play = false;
  bool show_settings = false;
  switch (snapshot.state) {
    case TorrentState::Stopped:
      play = GuiButton({700, 110 + pos, 20, 20}, "#131#");
      // Set the color of the progress bar orangeLINK
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, STOP_COLOR_HEX);
      show_settings = GuiButton({730, 110 + pos, 20, 20}, "#140#");
      // Adding progress bar
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(progress) + "% ").c_str(), "#132#Stopped",
                     progress, 0, 100);
      break;
    case TorrentState::Downloading:
      play = GuiButton({700, 110 + pos, 20, 20}, "#131#");
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, DOWNLOADING_COLOR_HEX);
      show_settings = GuiButton({730, 110 + pos, 20, 20}, "#140#");
      // Adding progress bar
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(progress) + "% ").c_str(), "#6#Downloading",
                     progress, 0, 100);
      break;
    case TorrentState::Loading:
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, STOP_COLOR_HEX);
      // Adding progress bar
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(progress) + "% ").c_str(), "#173#Indexing",
                     progress, 0, 100);
      break;
    case TorrentState::Completed:
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, DONE_COLOR_HEX);
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(progress) + "% ").c_str(), "#112#Downloaded",
                     progress, 0, 100);
      break;
    case TorrentState::Error:
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, ERROR_COLOR_HEX);
      GuiProgressBar({275, 110 + pos, 300, 20},
                     (std::to_string(progress) + "% ").c_str(), "#113#Error",
                     progress, 0, 100);
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
    state.torrent = snapshot;
  }


  GuiLine({5, 145 + pos, 800 - 10, 1}, NULL);
  #endif
}

/// Function to draw all the torrents based of the scroll state
void draw_torrents(GuiScrollTorrentState &state) {
  float pos = 5;
  //int index = 0;
  for (auto &torr : state.torrents) {
    // If window has been scrolled down some elements are not drawn
    if (pos + 50 < abs(state.scroll.y)) {
      pos += 50;
      continue;
    }
    // Update the torrent index
    //torr.second.index = index;
    // Draw the torrent
    draw_torrent_item(torr.second, pos + state.scroll.y, state.torrent_dialog_state);
    pos += 50;
    //index++;
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
      //torrent_dialog.input_priority = torrent_dialog.torrent.priority;
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
                 cbfn_torrent_insert fn) {

  // If the file selected is not a torrent do nothing
  if (!IsFileExtension(file_dialog_state.fileNameText, ".torrent")) {
    file_dialog_state.SelectFilePressed = false;
    file_dialog_state.fileDialogActive = true;
    return;
  }

  // Call the callback to add the torrent to the list of torrents
  auto result = fn(file_dialog_state.realFileName, file_dialog_state.fileNameText);
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
                    cbfn_torrent_remove fn) {

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
    if (confirm_dialog_state.confirm) {
      // Call the callback to remove the torrent
      auto result = fn(scroll_state.torrent_dialog_state.torrent);
      if (!result) {
        dialog_error.error = "Some error occurred";
        confirm_dialog_state.confirm = false;
        return;
      }
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
                     cbfn_setting_update fn) {
  if (settings_dialog_state.updated_path) {
    auto callback = fn(settings_dialog_state.path);
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
    cbfn_torrent_update fn) {

      #if 1
  auto &torrent =
      scroll_state.torrents[scroll_state.torrent_dialog_state.torrent.tid];
  auto callback = fn(torrent);
  if (!callback) {
    dialog_error.error = "Some error occurred";
    scroll_state.torrent_dialog_state.play = false;
    return;
  }
  switch (torrent.state) {
    case TorrentState::Stopped:
      torrent.state = TorrentState::Downloading;
      break;
    case TorrentState::Downloading:
      torrent.state = TorrentState::Stopped;
      break;
    default:
      break;
  }
  // Reset the action
  scroll_state.torrent_dialog_state.play = false;
  #endif
}

/// Function to update the torrent priority, it is called when the user clicks
/// on the tools icon
void update_torrent_priority(
    GuiScrollTorrentState &scroll_state, GuiErrorDialogState &dialog_error,
    bool (*update_torrent_priority_callback)(const TorrentGuiData &)) {
      #if 1
  auto &torrent =
      scroll_state.torrents[scroll_state.torrent_dialog_state.torrent.tid];
  auto callback = update_torrent_priority_callback(torrent);
  if (!callback) {
    dialog_error.error = "Some error occurred";
    scroll_state.torrent_dialog_state.update_priority = false;
    scroll_state.torrent_dialog_state.show_settings = true;
    return;
  }
  //torrent.priority = scroll_state.torrent_dialog_state.input_priority;
  // Reset the action
  scroll_state.torrent_dialog_state.update_priority = false;
#endif
}

}  // namespace fur::gui