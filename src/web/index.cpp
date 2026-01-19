#include <drogon/HttpAppFramework.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/drogon.h>
#include <strings.h>
#include <trantor/utils/Logger.h>

#include "controllers/block_chain_controller.h"

import core.hash; // C++20 module
import core.block_chain;

void setupCors() {
  // Register sync advice to handle CORS preflight (OPTIONS) requests
  drogon::app().registerSyncAdvice(
      [](const drogon::HttpRequestPtr &req) -> drogon::HttpResponsePtr {
        if (req->method() == drogon::HttpMethod::Options) {
          auto resp = drogon::HttpResponse::newHttpResponse();

          // Set Access-Control-Allow-Origin header based on the Origin
          // request header
          const auto &origin = req->getHeader("Origin");
          if (!origin.empty()) {
            resp->addHeader("Access-Control-Allow-Origin", origin);
          }

          // Set Access-Control-Allow-Methods based on the requested method
          const auto &requestMethod =
              req->getHeader("Access-Control-Request-Method");
          if (!requestMethod.empty()) {
            resp->addHeader("Access-Control-Allow-Methods", requestMethod);
          }

          // Allow credentials to be included in cross-origin requests
          resp->addHeader("Access-Control-Allow-Credentials", "true");

          // Set allowed headers from the Access-Control-Request-Headers
          // header
          const auto &requestHeaders =
              req->getHeader("Access-Control-Request-Headers");
          if (!requestHeaders.empty()) {
            resp->addHeader("Access-Control-Allow-Headers", requestHeaders);
          }

          return std::move(resp);
        }
        return {};
      });
  // Register post-handling advice to add CORS headers to all responses
  drogon::app().registerPostHandlingAdvice(
      [](const drogon::HttpRequestPtr &req,
         const drogon::HttpResponsePtr &resp) -> void {
        // Set Access-Control-Allow-Origin based on the Origin request
        // header
        const auto &origin = req->getHeader("Origin");
        if (!origin.empty()) {
          resp->addHeader("Access-Control-Allow-Origin", origin);
        }

        // Reflect the requested Access-Control-Request-Method back in the
        // response
        const auto &requestMethod =
            req->getHeader("Access-Control-Request-Method");
        if (!requestMethod.empty()) {
          resp->addHeader("Access-Control-Allow-Methods", requestMethod);
        }

        // Allow credentials to be included in cross-origin requests
        resp->addHeader("Access-Control-Allow-Credentials", "true");

        // Reflect the requested Access-Control-Request-Headers back
        const auto &requestHeaders =
            req->getHeader("Access-Control-Request-Headers");
        if (!requestHeaders.empty()) {
          resp->addHeader("Access-Control-Allow-Headers", requestHeaders);
        }
      });
}

int main() {
  setupCors();
  // Hash
  drogon::app().registerHandler(
      "/hash",
      [](const drogon::HttpRequestPtr &req,
         std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
        auto json = req->getJsonObject();
        if (!json || !json->isMember("data")) {
          auto resp = drogon::HttpResponse::newHttpJsonResponse(
              Json::Value{{"error", "missing field: data"}});
          resp->setStatusCode(drogon::k400BadRequest);
          callback(resp);
          return;
        }
        const std::string input = (*json)["data"].asString();
        const auto *raw = reinterpret_cast<const std::byte *>(input.data());
        std::span<const std::byte> bytes{raw, input.size()};

        core::Sha256Digest digest = core::sha256(bytes);
        std::ostringstream oss;
        oss << digest;

        Json::Value result;
        result["hash"] = oss.str();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "POST");
        callback(resp);
      },
      {drogon::Post});

  drogon::app().registerHandler(
      "/block",
      [](const drogon::HttpRequestPtr &req,
         std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
        auto json = req->getJsonObject();
        const auto index = (*json)["index"].asUInt64();
        const auto nonce = (*json)["nonce"].asUInt64();
        const auto &data = (*json)["data"].asString();
        const core::BlockPayload payload(
            reinterpret_cast<const std::byte *>(data.data()),
            reinterpret_cast<const std::byte *>(data.data() + data.size()));
        core::Block block(index, nonce, payload);
        core::Sha256Context ctx;
        block.hash_feed(ctx);
        std::ostringstream oss;
        oss << ctx.final();
        Json::Value result;
        result["hash"] = oss.str();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "POST");
        callback(resp);
      },
      {drogon::Post});

  // drogon::app().registerHandler(
  //     "/blockchain", [](const drogon::HttpRequestPtr &req, auto &&callback) {
  //
  //   });
  LOG_INFO << "Server running on localhost:8898";
  drogon::app().addListener("127.0.0.1", 8898).run();
}
