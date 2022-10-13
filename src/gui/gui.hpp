#pragma once

#include <raylib/raylib.h>

#include <functional>
#include <furrent.hpp>
#include <list>
#include <string>

#include <raylib/file_dialog.h>
#include <raylib/raygui.h>

namespace fur::gui {

struct GuiTorrentDialogState {
  /// True if the button play has been pressed
  bool play;
  /// True if the button delete of a specific torrent has been pressed
  bool delete_torrent;
  /// Target torrent
  TorrentGuiData torrent;
};

struct GuiScrollTorrentState {
  Vector2 scroll;
  std::list<TorrentGuiData> torrents;
  GuiTorrentDialogState torrent_dialog_state;
};

using torrent_insert_fn = std::function<std::optional<TorrentGuiData>(
    const std::string&, const std::string&)>;
using torrent_update_fn = std::function<TorrentGuiData(TorrentID tid)>;
using torrent_remove_fn = std::function<void(const TorrentGuiData&)>;

/// Responsible for rendering the UI
class Window {
  std::string _title;
  uint32_t _width, _height;

 private:
  // All GUI buttons
  bool _button_file_dialog;

  /// File dialog for file loading
  GuiFileDialogState _file_loader;
  /// All item to be displayed
  GuiScrollTorrentState _scroller;

  torrent_insert_fn _insert_fn;
  torrent_update_fn _update_fn;
  torrent_remove_fn _remove_fn;

 public:
  /// Constructs a new window of a fixed size
  Window(const std::string& title, uint32_t width, uint32_t height);
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
