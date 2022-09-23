#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION
#include <string>
#include <vector>

namespace fur::gui {

const int BORDER = 5;
const int W_WIDTH = 800;
const int W_HEIGHT = 600;
const unsigned int PRIMARY_COLOR_HEX = 0xc9effeff;
const unsigned int SECONDARY_COLOR_HEX = 0xffaf7aff;
const unsigned int TEXT_COLOR_HEX = 0x0492c7ff;
const unsigned int BACKGROUND_COLOR_HEX = 0xf5f5f5ff;
const Color BORDER_COLOR = GetColor(0x368bafff);
const Color PRIMARY_COLOR = GetColor(PRIMARY_COLOR_HEX);
const Color TEXT_COLOR = GetColor(TEXT_COLOR_HEX);
const Color BACKGROUND_COLOR = GetColor(BACKGROUND_COLOR_HEX);
const Color DIALOG_BACKGROUND_COLOR = Fade(BACKGROUND_COLOR, 0.85f);

enum TorrentState { STOP, DOWNLOAD, COMPLETED, ERROR };

struct TorrentGui {
  std::string filename{};
  TorrentState status = STOP;
  int progress{};
};

struct GuiSettingsDialogState {
  bool show = false;
};

struct GuiTorrentDialogState {
  bool play = false;
  bool show_settings = false;
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
  bool confirm = false;
  bool clicked = false;
  std::string message{};
  std::string button_yes{};
  std::string button_no{};
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
      play = GuiButton({700, 110 + pos, 20, 20}, "#131#");
      // Set the color of the progress bar orange
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, PRIMARY_COLOR_HEX);
      show_settings = GuiButton({730, 110 + pos, 20, 20}, "#140#");
      break;
    case fur::gui::DOWNLOAD:
      play = GuiButton({700, 110 + pos, 20, 20}, "#132#");
      GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, SECONDARY_COLOR_HEX);
      show_settings = GuiButton({730, 110 + pos, 20, 20}, "#140#");
      break;
    case COMPLETED:
    case ERROR:
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
  for (const auto& torr : state->torrents) {
    // If window has been scrolled down some elements are not drawn
    if (pos + 50 < abs(state->scroll.y)) {
      pos += 50;
      continue;
    }
    draw_torrent_item(torr, pos + state->scroll.y,
                      &state->torrent_dialog_state);
    pos += 50;
  }
}

/// Given the dialog setting state, it draws and manage the dialog
void settings_dialog(fur::gui::GuiSettingsDialogState* settings) {
  if (!settings->show) return;
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);
  settings->show = !GuiWindowBox({200, 50, 400, 400}, "#198# Settings dialog");
  // TODO: Add furrent settings
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
  if (!state->show) return;
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                DIALOG_BACKGROUND_COLOR);
  // When clicking on the button close
  auto close = GuiWindowBox({200, 50, 400, 200}, "#198# Confirm action");
  // Draw the message
  GuiDrawText(state->message.c_str(), {200, 75, 400, 100}, TEXT_ALIGN_CENTER,
              GRAY);
  // Draw the buttons
  auto button_yes = GuiButton({250, 175, 100, 50}, state->button_yes.c_str());
  auto button_no = GuiButton({450, 175, 100, 50}, state->button_no.c_str());
  // Se è stato cliccato un bottone chiudo la finestra e aggiorno lo stato
  if (close || button_yes || button_no) {
    state->show = false;
    state->clicked = true;
    state->confirm = button_yes;
  }
}
}  // namespace fur::gui