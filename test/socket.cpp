#include "download/socket.hpp"

#include <cstdint>
#include <vector>

#include "asio.hpp"
#include "catch2/catch.hpp"

using namespace fur::download::socket;

TEST_CASE("[Socket] Basic") {
  // Faker on port 4002 will read 10 bytes and then write them back.

  Socket sock;
  auto ip = asio::ip::make_address_v4("127.0.0.1").to_uint();
  sock.connect(ip, 4002, std::chrono::milliseconds{50});

  const std::vector<uint8_t> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  sock.write(data, std::chrono::milliseconds{50});
  auto result = sock.read(10, std::chrono::milliseconds{50});

  REQUIRE(data == result);
}

TEST_CASE("[Socket] Timeout expire") {
  // Faker on port 4003 will read 10 bytes and then wait 1 second before
  // writing them back.

  Socket sock;
  auto ip = asio::ip::make_address_v4("127.0.0.1").to_uint();
  sock.connect(ip, 4003, std::chrono::milliseconds{50});

  const std::vector<uint8_t> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  sock.write(data, std::chrono::seconds(1));

  REQUIRE_THROWS_AS(sock.read(10, std::chrono::milliseconds{50}),
                    std::system_error);
}

TEST_CASE("[Socket] Invalid connect") {
  // There is no one listening on port 3999

  Socket sock;
  auto ip = asio::ip::make_address_v4("127.0.0.1").to_uint();
  REQUIRE_THROWS(sock.connect(ip, 3999, std::chrono::milliseconds{50}));
}
