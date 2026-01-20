#include "block_chain_controller.h"
#include <drogon/HttpResponse.h>
#include <sstream>

import core.hash;
import core.block_chain;
namespace web {
void BlockchainController::get_chain(
    const drogon::HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  Json::Value result;
  Json::Value blocks_json(Json::arrayValue);
  for (const auto &[block, hash] : blockchain_.blocks()) {

    Json::Value block_json;
    block_json["index"] = static_cast<Json::UInt64>(block.index);
    block_json["nonce"] = static_cast<Json::UInt64>(block.nonce);

    std::string payload_str(
        reinterpret_cast<const char *>(block.payload.data()),
        block.payload.size());
    std::ostringstream oss;
    block_json["payload"] = payload_str;
    oss << hash;
    block_json["hash"] = oss.str();

    blocks_json.append(block_json);
  }

  result["blocks"] = blocks_json;
  result["target"] = blockchain_.target();

  auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
  resp->addHeader("Access-Control-Allow-Origin", "*");
  resp->addHeader("Access-Control-Allow-Methods", "GET");
  callback(resp);
}
void BlockchainController::add_block(
    const drogon::HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  auto json = req->getJsonObject();
  if (!json || !json->isMember("nonce") || !json->isMember("payload")) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(
        Json::Value{{"error", "missing field: nonce or payload"}});
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }

  uint64_t nonce = 0;
  try {
    nonce = (*json)["nonce"].asUInt64();
  } catch (...) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(
        Json::Value{{"error", "invalid nonce"}});
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }

  const std::string &payload_str = (*json)["payload"].asString();
  core::BlockPayload payload(
      reinterpret_cast<const std::byte *>(payload_str.data()),
      reinterpret_cast<const std::byte *>(payload_str.data() +
                                          payload_str.size()));

  bool success = blockchain_.add_block(payload, nonce);

  Json::Value result;
  result["success"] = success;
  if (!success) {
    result["message"] = "block does not meet target or invalid";
  }

  auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
  resp->addHeader("Access-Control-Allow-Origin", "*");
  resp->addHeader("Access-Control-Allow-Methods", "POST");
  callback(resp);
}

void BlockchainController::compute_block_hash(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto json = req->getJsonObject();
  if (!json || !json->isMember("index") || !json->isMember("nonce") ||
      !json->isMember("payload") || !json->isMember("prev_hash")) {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(
        Json::Value{{"error", "missing required fields"}});
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }

  uint64_t index = (*json)["index"].asUInt64();
  uint64_t nonce = (*json)["nonce"].asUInt64();
  const std::string &payloadStr = (*json)["payload"].asString();
  const std::string &prevHashStr = (*json)["prev_hash"].asString();

  core::BlockPayload payload(
      reinterpret_cast<const std::byte *>(payloadStr.data()),
      reinterpret_cast<const std::byte *>(payloadStr.data() +
                                          payloadStr.size()));

  core::Block block{index, nonce, payload};

  core::Sha256Context ctx;

  block.hash_feed(ctx);
  if (!prevHashStr.empty()) {
    std::span<const std::byte> prevHashBytes(
        reinterpret_cast<const std::byte *>(prevHashStr.data()),
        prevHashStr.size());
    ctx.update(prevHashBytes);
  } else {
    ctx.update(core::Sha256Digest{});
  }

  core::Sha256Digest digest = ctx.final();

  std::ostringstream oss;
  oss << digest;

  Json::Value result;
  result["hash"] = oss.str();

  auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
  resp->addHeader("Access-Control-Allow-Origin", "*");
  resp->addHeader("Access-Control-Allow-Methods", "POST");
  callback(resp);
}
} // namespace web
