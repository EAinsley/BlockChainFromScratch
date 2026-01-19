#pragma once

#include <drogon/HttpController.h>

import core.block_chain;

namespace web {
using namespace drogon;
class BlockchainController
    : public drogon::HttpController<BlockchainController> {
public:
  METHOD_LIST_BEGIN

  ADD_METHOD_TO(BlockchainController::get_chain, "/blockchain", Get);
  ADD_METHOD_TO(BlockchainController::add_block, "/blockchain", Post);

  METHOD_LIST_END

protected:
  void get_chain(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback);
  void add_block(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback);

private:
  core::BlockChain blockchain_;
};

} // namespace web
