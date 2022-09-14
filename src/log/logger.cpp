//
// Created by Z1ko on 23/08/2022.
//

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <log/logger.hpp>

namespace fur::log {

namespace {
const char* FUR_LOG_FILE = "log.txt";
const char* FUR_LOG_FORMAT = "[%T][%L][%t] %v";
}  // namespace

std::shared_ptr<spdlog::logger> initialize_custom_logger(bool do_file_sink) {
  std::vector<spdlog::sink_ptr> sinks = {
      // Console with colors
      std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
  };

  if (do_file_sink) {
    // File on disk
    sinks.push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(FUR_LOG_FILE));
  }

  auto logger = std::make_shared<spdlog::logger>("custom", std::begin(sinks),
                                                 std::end(sinks));
  // TODO: Output configuration
  logger->set_pattern(FUR_LOG_FORMAT);
  logger->set_level(spdlog::level::info);

  spdlog::register_logger(logger);
  return logger;
}

}  // namespace fur::log