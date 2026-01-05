#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

import core.hash; // C++20 module

using std::byte;

TEST_CASE("Different inputs produce different hashes") {
  std::string input1 = "hello";
  std::string input2 = "world";

  auto hash1 = core::sha256(std::span<const byte>(
      reinterpret_cast<const byte *>(input1.data()), input1.size()));
  auto hash2 = core::sha256(std::span<const byte>(
      reinterpret_cast<const byte *>(input2.data()), input2.size()));

  REQUIRE(hash1 != hash2);
}

TEST_CASE("Same input produces consistent hash") {
  std::string input = "blockchain";

  auto hash1 = core::sha256(std::span<const byte>(
      reinterpret_cast<const byte *>(input.data()), input.size()));
  auto hash2 = core::sha256(std::span<const byte>(
      reinterpret_cast<const byte *>(input.data()), input.size()));

  REQUIRE(hash1 == hash2);
}
