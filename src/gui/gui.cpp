#include <raylib/raylib.h>

#include <chrono>

#define RAYGUI_IMPLEMENTATION
#include <raylib/raygui.h>
#undef RAYGUI_IMPLEMENTATION

#define GUI_FILE_DIALOG_IMPLEMENTATION
#include <raylib/file_dialog.h>
#undef GUI_FILE_DIALOG_IMPLEMENTATION

#include <gui/colors.hpp>
#include <gui/gui.hpp>
#include <iostream>

namespace fur::gui {

Window::Window(const std::string& title, uint32_t width, uint32_t height)
    : _title{title},
      _width{width},
      _height{height},
      _button_file_dialog{false},
      _file_loader{},
      _scroller{} {
  InitWindow(static_cast<int>(width), static_cast<int>(height), title.c_str());
  SetWindowPosition(0, 0);
  SetTargetFPS(60);

  configure_style();

  _file_loader =
      InitGuiFileDialog(550, 500, GetWorkingDirectory(), false, ".torrent");
}

Window::~Window() { CloseWindow(); }

void Window::run() {
  auto clock_old = std::chrono::high_resolution_clock::now();
  while (!WindowShouldClose()) {
    auto clock_now = std::chrono::high_resolution_clock::now();
    auto clock_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        clock_now - clock_old);
    if (clock_elapsed.count() >= 30) {
      // TODO: update furrent every tot milliseconds
      for (auto& torrent : _scroller.torrents)
        torrent = _update_fn(torrent.tid);

      clock_old = std::chrono::high_resolution_clock::now();
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    render_base();
    render_file_dialog();
    render_confirm_dialog();

    EndDrawing();
  }
}

void Window::set_torrent_insert_fn(torrent_insert_fn fn) { _insert_fn = fn; }

void Window::set_torrent_update_fn(torrent_update_fn fn) { _update_fn = fn; }

void Window::set_torrent_remove_fn(torrent_remove_fn fn) { _remove_fn = fn; }

static Rectangle create_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
  return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(w),
          static_cast<float>(h)};
}

void Window::render_file_dialog() {
  GuiFileDialog(&_file_loader);

  // File loader
  if (_button_file_dialog) _file_loader.fileDialogActive = true;

  // We have to load a file
  if (_file_loader.SelectFilePressed) {
    if (!IsFileExtension(_file_loader.fileNameText, ".torrent")) {
      _file_loader.SelectFilePressed = false;
      _file_loader.fileDialogActive = true;
      return;
    }

    // Create new torrent item
    auto item = _insert_fn(_file_loader.fileNameText, _file_loader.dirPathText);
    if (item.has_value()) {
      _scroller.torrents.emplace_back(*item);
    }

    _file_loader.SelectFilePressed = false;
    _file_loader.fileDialogActive = false;
  }
}

void Window::render_confirm_dialog() {
  if (!_scroller.torrent_dialog_state.delete_torrent) {
    return;
  }
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);

  auto response =
      GuiMessageBox({200, 100, 400, 200}, "#198# Confirm action",
                    "Are you sure you want to delete this torrent?", "Yes;No");

  // Se è stato cliccato un bottone chiudo la finestra e aggiorno lo stato
  if (response == 0 || response == 1 || response == 2) {
    _scroller.torrent_dialog_state.delete_torrent = false;
    if (response == 1) {
      auto torrent = _scroller.torrent_dialog_state.torrent;
      _remove_fn(torrent);
      // Remove torrent in the UI
      _scroller.torrents.erase(
          std::remove_if(_scroller.torrents.begin(), _scroller.torrents.end(),
                         [torrent](const TorrentGuiData& t) {
                           return t.tid == torrent.tid;
                         }),
          _scroller.torrents.end());
    }
  }
}

void Window::render_base() {
  static auto logo = LoadTexture("../assets/Furrent.png");

  DrawTexture(logo, WINDOW_BORDER, WINDOW_BORDER, WHITE);
  GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
  Rectangle title_rect = {65, WINDOW_BORDER, 0, 50};
  GuiDrawText("Furrent", title_rect, TEXT_ALIGN_LEFT, DARK_BROWN_COLOR);
  GuiSetStyle(DEFAULT, TEXT_SIZE, 15);

  // Load file button
  // with width = _width - WINDOW_BORDER - 150
  static Rectangle button_file_rect =
      create_rect(_width - WINDOW_BORDER - 150, 15, 150, 30);
  _button_file_dialog = GuiButton(button_file_rect, "#3# Open torrent");

  // Scroll panel
  static Rectangle scroll_rect_1 = {
      WINDOW_BORDER, 100, static_cast<float>(_width - WINDOW_BORDER * 2),
      static_cast<float>(_height - 100 - WINDOW_BORDER)};

  // Draw scroll panel
  GuiScrollPanel(scroll_rect_1, NULL,
                 {WINDOW_BORDER, 50, static_cast<float>(_width - 150),
                  static_cast<float>(50 * _scroller.torrents.size())},
                 &_scroller.scroll);
  // Drawing torrents
  render_torrents();

  // Scroll panel head
  static Rectangle scroll_head_rect_1 =
      create_rect(WINDOW_BORDER, 61, _width - WINDOW_BORDER * 2, 40);
  static Rectangle scroll_head_rect_2 =
      create_rect(WINDOW_BORDER, 61, _width - WINDOW_BORDER * 2, 40);
  GuiDrawRectangle(scroll_head_rect_1, 1, DARK_BROWN_COLOR,
                   DARK_BACKGROUND_COLOR);
  GuiDrawText("Torrents", scroll_head_rect_2, TEXT_ALIGN_CENTER,
              DARK_BROWN_COLOR);

  // Scroll panel bottom
  static Rectangle scroll_bottom_rect =
      create_rect(WINDOW_BORDER, _height - WINDOW_BORDER, _width, _width);
  GuiDrawRectangle(scroll_bottom_rect, 1, PRESSED_BACKGROUND_COLOR,
                   PRESSED_BACKGROUND_COLOR);
  float wheel_move = GetMouseWheelMove();
  if (wheel_move != 0) {
    _scroller.scroll.y += wheel_move * 20;
  }
}

void Window::render_torrent_item(const TorrentGuiData& torrent, float pos) {
  // Cut filename if it is too long
  std::string name = "Name: " + torrent.filename;
  if (name.size() > 20) name = name.substr(0, 20) + "...";

  // Draw the name
  Rectangle name_rect =
      create_rect(20, static_cast<uint32_t>(112 + pos), 0, 20);
  GuiDrawText(name.c_str(), name_rect, TEXT_ALIGN_LEFT, GRAY);

  int progress = static_cast<int>(
      (static_cast<float>(torrent.pieces_processed) / torrent.pieces_count) *
      100);

  switch (torrent.state) {
    case TorrentState::Downloading: {
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, DOWNLOADING_COLOR_HEX);
      Rectangle rect = create_rect(275, static_cast<int>(110 + pos), 300, 20);
      GuiProgressBar(rect, (std::to_string(progress) + "% ").c_str(),
                     "#6#Downloading", progress, 0, 100);

    } break;
    case TorrentState::Completed: {
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, DONE_COLOR_HEX);
      Rectangle rect = create_rect(275, static_cast<int>(110 + pos), 300, 20);
      GuiProgressBar(rect, (std::to_string(progress) + "% ").c_str(),
                     "#112#Downloaded", progress, 0, 100);

    } break;
    case TorrentState::Stopped: {
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, STOP_COLOR_HEX);
      Rectangle rect = create_rect(275, static_cast<int>(110 + pos), 300, 20);
      GuiProgressBar(rect, (std::to_string(progress) + "% ").c_str(),
                     "#132#Stopped", progress, 0, 100);

    } break;
    case TorrentState::Error: {
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, ERROR_COLOR_HEX);
      Rectangle rect = create_rect(275, static_cast<int>(110 + pos), 300, 20);
      GuiProgressBar(rect, (std::to_string(progress) + "% ").c_str(),
                     "#113#Error", progress, 0, 100);

    } break;
    default:
      break;
  }

  Rectangle end_rect = create_rect(5, static_cast<uint32_t>(145 + pos), 790, 1);
  GuiLine(end_rect, NULL);
}

void Window::render_torrents() {
  float pos = 5;
  for (auto it = _scroller.torrents.begin(); it != _scroller.torrents.end();
       ++it) {
    if (pos + 50 < abs(_scroller.scroll.y)) {
      pos += 50;
      continue;
    }

    render_torrent_item(*it, pos + _scroller.scroll.y);

    // Delete torrent event
    Rectangle button_rect = create_rect(
        760, static_cast<uint32_t>(110 + pos + _scroller.scroll.y), 20, 20);
    if (GuiButton(button_rect, "#143#")) {
      _scroller.torrent_dialog_state.delete_torrent = true;
      _scroller.torrent_dialog_state.torrent = *it;
    }
    pos += 50;
  }
}

void Window::configure_style() {
  GuiSetFont(LoadFont("../assets/Righteous-Regular.ttf"));

  GuiSetStyle(DEFAULT, TEXT_SIZE, 15);
  GuiSetStyle(DEFAULT, BACKGROUND_COLOR, PRESSED_BACKGROUND_HEX);
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
}

}  // namespace fur::gui
