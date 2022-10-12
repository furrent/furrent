#pragma once

#include <cstdint>
#include <raylib/raygui.h>

namespace fur::gui {
    
// Colors
const int DARK_BROWN_HEX         = static_cast<int>(0x876645FF);
const int ORANGE_HEX             = static_cast<int>(0xD27607FF);
const int LIGHT_BACKGROUND_HEX   = static_cast<int>(0xF6F2EEFF);
const int DARK_BACKGROUND_HEX    = static_cast<int>(0xBA9978FF);
const int PRESSED_BACKGROUND_HEX = static_cast<int>(0xEDE9E0FF);
const int DOWNLOADING_COLOR_HEX  = static_cast<int>(0xC9EFFEFF);
const int STOP_COLOR_HEX         = static_cast<int>(0xFFAF7AFF);
const int DONE_COLOR_HEX         = static_cast<int>(0xABF7B1FF);
const int ERROR_COLOR_HEX        = static_cast<int>(0xFFA590FF);

// Raygui colors
const Color DARK_BROWN_COLOR         = GetColor(DARK_BROWN_HEX);
const Color DARK_BACKGROUND_COLOR    = GetColor(DARK_BACKGROUND_HEX);
const Color PRESSED_BACKGROUND_COLOR = GetColor(PRESSED_BACKGROUND_HEX);
const Color DIALOG_BACKGROUND_COLOR  = Fade(PRESSED_BACKGROUND_COLOR, 0.85f);

// Window constants
const int WINDOW_BORDER = 5;

} // namespace fur::gui
