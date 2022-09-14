#pragma once

#include <cstddef>

namespace fur::config {
    
/// Where newly downloaded files are to be stored
inline const char* DOWNLOAD_FOLDER = "../output/";

/// Socket timeout seconds
inline const size_t SOCKET_TIMEOUT_SECONDS = 5;

} // namespace fur::config
