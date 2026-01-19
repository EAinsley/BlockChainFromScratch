module;
#include <algorithm>
#include <cstdint>
#include <span>
module core.block_chain;

namespace core {
void Block::hash_feed(Sha256Context &ctx) const {
  ctx.update(std::as_bytes(std::span{&index, 1}));
  ctx.update(std::as_bytes(std::span{&nonce, 1}));
  ctx.update(payload);
}

bool BlockChain::_block_verified(const Block &block) const { return false; }
bool BlockChain::_hash_verified(const Sha256Digest &digest) const {
  return verify_hash_difficulty(digest, target_);
}

Sha256Digest BlockChain::_block_hash(const Block &block) const {
  Sha256Context ctx;
  block.hash_feed(ctx);
  if (chain_.empty()) {
    ctx.update(Sha256Digest());
  } else {
    ctx.update(chain_.back().second);
  }
  return ctx.final();
}

bool BlockChain::add_block(const BlockPayload &payload, uint64_t nonce) {
  Block block(0, nonce, payload);
  if (!chain_.empty()) {
    const auto prev_block = chain_.back();
    block.index = prev_block.first.index + 1;
  }
  Sha256Digest block_hash = _block_hash(block);
  if (_hash_verified(block_hash)) {
    chain_.emplace_back(std::move(block), block_hash);
    return true;
  }
  return false;
}

void BlockChain::set_target(int target) {
  target_ = std::clamp(target, 0, int(SHA256_SIZE));
}
size_t BlockChain::target() const { return target_; }

size_t BlockChain::size() const { return chain_.size(); }
} // namespace core
