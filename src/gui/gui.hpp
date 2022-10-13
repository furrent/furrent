#pragma once

#include <raylib/raylib.h>

#include <functional>
#include <furrent.hpp>
#include <list>
#include <string>

#undef RAYGUI_IMPLEMENTATION
#include <raylib/file_dialog.h>
#include <raylib/raygui.h>

namespace fur::gui {

/// State of the settings dialog
struct GuiSettingsDialogState {
  /// True if the path has been updated and the dialog should be closed
  /// used in the main loop to retrieve the new path
  bool updated_path;
  /// The path that is currently displayed in the dialog
  char* input_path;
  /// The real path that is used by the program
  std::string path;
  /// If the error is initialized it will be displayed in a new dialog
  std::string error;
};

struct GuiTorrentDialogState {
  /// True if the button play has been pressed
  bool play;
  /// True if the button stop has been pressed
  bool delete_torrent;
  /// The priority during the change
  int input_priority;
  /// Target torrent
  TorrentGuiData torrent;
};

struct GuiScrollTorrentState {
  Vector2 scroll;
  std::list<TorrentGuiData> torrents;
  GuiTorrentDialogState torrent_dialog_state;
};

struct GuiConfirmDialogState {
  /// True if the user clicked on the confirm button
  bool confirm = false;
  /// True if the user has clicked on a button (confirm, cancel of exit)
  bool clicked = false;
  /// Message to display in the dialog
  std::string message;
  /// Text of the button yes
  std::string confirm_button;
  /// Text of the button no
  std::string cancel_button;
};

using torrent_insert_fn =
    std::function<std::optional<TorrentGuiData>(const std::string&, const std::string&)>;
using torrent_update_fn = std::function<TorrentGuiData(TorrentID tid)>;
using torrent_remove_fn = std::function<void(const TorrentGuiData&)>;

/// Responsible for rendering the UI
class Window {
  std::string _title;
  uint32_t _width, _heigth;

 private:
  // All GUI buttons
  bool _button_file_dialog;

  /// File dialog for file loading
  GuiFileDialogState _file_loader;
  /// All item to be displayed
  GuiScrollTorrentState _scroller;
  /// Confirm dialog
  GuiConfirmDialogState _confirm_dialog;

  torrent_insert_fn _insert_fn;
  torrent_update_fn _update_fn;
  torrent_remove_fn _remove_fn;

 public:
  /// Constructs a new window of a fixed size
  Window(const std::string& title, uint32_t width, uint32_t heigth);
  virtual ~Window();

  /// Start the UI loop
  void run();

  void set_torrent_insert_fn(torrent_insert_fn fn);
  void set_torrent_update_fn(torrent_update_fn fn);
  void set_torrent_remove_fn(torrent_remove_fn fn);

 private:
  /// Configure window style
  void configure_style();

  /// Render the main window panel
  void render_base();
  /// Render all dialogs
  void render_file_dialog();
  /// Render single torrent item
  void render_torrent_item(const TorrentGuiData& torrent, float pos);
  /// Render the torrents items
  void render_torrents();
  /// Render the confirm dialog
  void render_confirm_dialog();
};

}  // namespace fur::gui
