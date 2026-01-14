#define CATCH_CONFIG_MAIN
#include <string_view>

#include <catch2/catch_all.hpp>

import core.hash; // C++20 module
import core.exceptions;

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

TEST_CASE("Sha256Context constructors and Assignment", "[Sha256Context]") {

  using core::Sha256Context;
  SECTION("Move constructor") {
    Sha256Context ctx1;
    std::string msg = "hello move constructor";
    ctx1.update(std::as_bytes(std::span(msg)));

    Sha256Context ctx2(std::move(ctx1)); // move constructor

    // ctx1 is invalid
    REQUIRE_FALSE(ctx1.valid());
    REQUIRE_THROWS_AS(ctx1.update(std::as_bytes(std::span(msg))),
                      core::hash_error);

    // ctx2 should work properly
    ctx2.update(std::as_bytes(std::span(" more")));
    auto digest = ctx2.final();
    REQUIRE(digest.size() == 32);
  }

  SECTION("Move assignment") {
    Sha256Context ctx1;
    std::string msg1 = "hello";
    ctx1.update(std::as_bytes(std::span(msg1)));

    Sha256Context ctx2;
    std::string msg2 = "world";
    ctx2.update(std::as_bytes(std::span(msg2)));

    ctx2 = std::move(ctx1); // move assignment

    // ctx1 should be invalid
    REQUIRE_FALSE(ctx1.valid());
    REQUIRE_THROWS_AS(ctx1.update(std::as_bytes(std::span(msg1))),
                      core::hash_error);

    // ctx2 should work properly
    ctx2.update(std::as_bytes(std::span(" appended")));
    auto digest = ctx2.final();
    REQUIRE(digest.size() == 32);
  }

  SECTION("Move after final") {
    Sha256Context ctx1;
    ctx1.update(std::as_bytes(std::span("test final")));
    auto digest1 = ctx1.final();

    Sha256Context ctx2(std::move(ctx1)); // move constructor after final
    REQUIRE_FALSE(ctx1.valid());
    REQUIRE(digest1 == ctx2.final()); // final digest preserved
  }

  SECTION("Move assignment after final") {
    Sha256Context ctx1;
    ctx1.update(std::as_bytes(std::span("test move assign final")));
    auto digest1 = ctx1.final();

    Sha256Context ctx2;
    ctx2 = std::move(ctx1); // move assignment after final
    REQUIRE_FALSE(ctx1.valid());
    REQUIRE(digest1 == ctx2.final());
  }
}

TEST_CASE("Sha256Context basic functionality", "[Sha256Context]") {
  using core::Sha256Context;
  using namespace std::literals;
  Sha256Context ctx;

  SECTION("Single update produces correct length digest") {
    std::string msg = "hello world";
    ctx.update(std::as_bytes(std::span(msg)));
    auto digest = ctx.final();
    REQUIRE(digest.size() == core::SHA256_SIZE);
    REQUIRE(ctx.finalized());
  }

  SECTION("Multiple update chunks") {
    std::string part1 = "hello ";
    std::string part2 = "world";
    ctx.update(std::as_bytes(std::span(part1)));
    ctx.update(std::as_bytes(std::span(part2)));
    auto digest1 = ctx.final();

    // Reset and compute in one shot
    ctx.reset();
    ctx.update(std::as_bytes(std::span("hello world"sv)));
    auto digest2 = ctx.final();

    REQUIRE(digest1 == digest2);
  }

  SECTION("Multiple final returns same digest") {
    ctx.update(std::as_bytes(std::span("abc"sv)));
    auto digest1 = ctx.final();
    auto digest2 = ctx.final();
    REQUIRE(digest1 == digest2);
  }

  SECTION("Reset allows reuse") {
    ctx.update(std::as_bytes(std::span("abc"sv)));
    auto digest1 = ctx.final();
    ctx.reset();
    ctx.update(std::as_bytes(std::span("abc"sv)));
    auto digest2 = ctx.final();
    REQUIRE(digest1 == digest2);
  }

  SECTION("Update after final throws") {
    ctx.update(std::as_bytes(std::span("abc"sv)));
    ctx.final();
    REQUIRE_THROWS_AS(ctx.update(std::as_bytes(std::span("def"sv))),
                      core::hash_error);
  }

  SECTION("Calling final on uninitialized context throws") {
    Sha256Context ctx2;
    ctx2.invalidate(); // set uninitialized
    REQUIRE_THROWS_AS(ctx2.final(), core::hash_error);
  }

  SECTION("Calling update on uninitialized context throws") {
    Sha256Context ctx2;
    ctx2.invalidate();
    REQUIRE_THROWS_AS(ctx2.update(std::as_bytes(std::span("abc"sv))),
                      core::hash_error);
  }
}
