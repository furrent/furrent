#include <config.hpp>
#include <furrent.hpp>
#include <gui/gui.hpp>
#include <log/logger.hpp>
#include <stdexcept>

using namespace fur;

int main(int, char* argv[]) {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  // Create output directory

  const std::string argv_0(argv[0]);
  auto last_slash = argv_0.find_last_of('/');
  const std::string path_base = argv_0.substr(0, last_slash + 1);

  Furrent& furrent = Furrent::instance();
  if (!furrent.set_download_folder(path_base + fur::config::DOWNLOAD_FOLDER)
           .valid()) {
    throw std::logic_error("cannot create download folder");
  }

  fur::gui::Window window("Furrent", 800, 600);

  window.set_torrent_insert_fn(
      [&](const std::string& filepath,
          const std::string&) -> std::optional<TorrentGuiData> {
        auto tid = furrent.add_torrent(filepath);
        if (tid.valid())
          return furrent.get_gui_data(*tid);
        else
          return std::nullopt;
      });

  window.set_torrent_update_fn([&](TorrentID tid) -> TorrentGuiData {
    return furrent.get_gui_data(tid);
  });

  window.set_torrent_remove_fn([&](const TorrentGuiData& torrent) {
    furrent.remove_torrent(torrent.tid);
  });

  window.run();
}
