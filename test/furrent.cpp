//
// Created by Z1ko on 25/08/2022.
//

#include "catch2/catch.hpp"

#include <thread>
#include <iostream>

#include <furrent.hpp>

using namespace fur;

void test_worker(Piece& p) {

  const int piece_count = p.task.torrent.length / p.task.torrent.piece_length;
  std::cout << "Thread[" << std::this_thread::get_id() << "] is downloading piece "
            << p.task.index << " of " << piece_count << " for "
            << p.task.torrent.name << std::endl;
}

TEST_CASE("Furrent base functionality") {

  Furrent fur{test_worker};
  fur.add_torrent("../extra/debian-11.4.0-amd64-netinst.iso.torrent");
  fur.add_torrent("../extra/debian-11.4.0-amd128-netinst.iso.torrent");

  std::this_thread::sleep_for(std::chrono::seconds(3));
}