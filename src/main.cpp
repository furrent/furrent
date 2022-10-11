#include <log/logger.hpp>
#include <platform/io.hpp>
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
  fur::platform::io::create_subfolders(path_base, { "output" });

  fur::gui::Window window("Furrent", 800, 600);
  
  window.set_torrent_insert_fn([&](const std::string& filepath, const std::string&) -> TorrentGuiData {
    TorrentID tid = *furrent.add_torrent(filepath);
    return furrent.get_gui_data(tid).value();
  });
  
  window.set_torrent_update_fn([&](TorrentID tid) -> TorrentGuiData {
    return furrent.get_gui_data(tid).value();
  });

  window.set_torrent_remove_fn([&](const TorrentGuiData& torrent) {
    furrent.remove_torrent(torrent.tid);
  });

  window.run();

}
