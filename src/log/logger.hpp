//
// Created by Z1ko on 23/08/2022.
//

#pragma once

#include <string>

#include <spdlog/spdlog.h>

namespace fur::log {

/// Creates and registers default logger, with console and file sinks
/// TODO: Use config file
std::shared_ptr<spdlog::logger> initialize_custom_logger(
    bool do_file_sink = true);

} // namespace fur::log

