#include "catch2/catch.hpp"

#include <thread>
#include <iostream>
#include <atomic>

#include <furrent.hpp>

using namespace fur;

const char* debian_torrent = "../extra/debian-11.4.0-amd64-netinst.iso.torrent";
const char* ubuntu_torrent = "../extra/ubuntu-22.04.1-desktop-amd64.iso.torrent";

TEST_CASE("[Furrent] add new torrent and count all tasks") {

  int pieces = 0;
  {
    fur::Furrent fur;
    fur.add_torrent(debian_torrent);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 1148370 is the sum of the numbers from 0 to 1516 (the number of tasks)
    while(fur.get_total_processed_pieces() != 1516);
    pieces = fur.get_total_processed_pieces();
  }

  REQUIRE(pieces == 1516);
}

/*
 * TODO: Resolving error
TEST_CASE("[Furrent] add two torrent and count all tasks") {
  int num_tasks_deb = 0;
  int num_tasks_ubt = 0;
  fur::Furrent fur{[&num_tasks_deb,&num_tasks_ubt](fur::Piece& p) {

    std::cout << p.task->torrent.name << std::endl;
  }};
  fur.add_torrent(debian_torrent);
  fur.add_torrent(ubuntu_torrent);
  // Wait for the program to finish
  std::this_thread::sleep_for(std::chrono::seconds(3));
  REQUIRE(num_tasks_deb == 1516);
  REQUIRE(num_tasks_ubt == 11712);
}
*/
