#include <iostream>

#include <log/logger.hpp>
#include <furrent.hpp>

void test_worker(fur::Piece& p) {
  std::cout << "test_worker" << std::endl;
  /*
  const int piece_count = p.task.torrent.length / p.task.torrent.piece_length;

  std::cout << "Thread[" << std::this_thread::get_id() << "] is downloading piece"
            << p.task.index << " of " << piece_count << " for "
            << p.task.torrent.name << std::endl;
  */
}

int main() {

  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  logger->info("Init programm...");
  fur::Furrent fur{test_worker};
  logger->info("Adding torrent");
  fur.add_torrent("../extra/debian-11.4.0-amd64-netinst.iso.torrent");
  logger->info("Torrent added");

  // Wait for the program to finish
  std::this_thread::sleep_for(std::chrono::seconds(10));

  return 0;
}
