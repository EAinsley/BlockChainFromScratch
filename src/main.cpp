#include <cstddef>
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>
#include <drogon/drogon.h>
#include <sstream>
#include <trantor/utils/Logger.h>

import core.hash; // C++20 module

int main() {
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

        callback(drogon::HttpResponse::newHttpJsonResponse(result));
      },
      {drogon::Post});
  LOG_INFO << "Server running on localhost:8898";
  drogon::app().addListener("127.0.0.1", 8898).run();
}
