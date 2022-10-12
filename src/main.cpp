#include <log/logger.hpp>
#include <platform/io.hpp>
#include <config.hpp>
#include <gui/gui.hpp>
#include <furrent.hpp>

using namespace fur;

int main(int argc, char* argv[]) {

  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  Furrent& furrent = Furrent::instance();

  const std::string argv_0(argv[0]);
  auto last_slash = argv_0.find_last_of('/');
  const std::string path_base = argv_0.substr(0, last_slash + 1);

  // Create output directory
  auto creation = fur::platform::io::create_subfolders("", { fur::config::DOWNLOAD_FOLDER });
  if (creation.valid())
    logger->info("Created base output directory at {}", *creation);
  else if (creation.error() == platform::io::IOError::DirectoryAlreadyExists)
    logger->info("Output directory already exists!");
  else {
    logger->critical("Error creating output directory!");
    return -1;
  }

  fur::gui::Window window("Furrent", 800, 600);
  
  window.set_torrent_insert_fn([&](const std::string& filepath, const std::string&) -> std::optional<TorrentGuiData> {
    auto tid = furrent.add_torrent(filepath);
    if (tid.valid())
      return furrent.get_gui_data(*tid);
    return std::nullopt;
  });
  
  window.set_torrent_update_fn([&](TorrentID tid) -> TorrentGuiData {
    return furrent.get_gui_data(tid).value();
  });

  window.set_torrent_remove_fn([&](const TorrentGuiData& torrent) {
    furrent.remove_torrent(torrent.tid);
  });

  window.run();

}
