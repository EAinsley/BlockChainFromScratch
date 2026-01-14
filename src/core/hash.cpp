module;

#include <cstddef>
#include <iomanip>
#include <ostream>
#include <span>
#include <utility>

#include <openssl/evp.h>
#include <openssl/sha.h>

module core.hash;
import core.exceptions;

namespace core {
Sha256Digest sha256(std::span<const std::byte> data) {
  Sha256Digest digest;
  SHA256(reinterpret_cast<const unsigned char *>(data.data()), data.size(),
         reinterpret_cast<unsigned char *>(digest.data()));
  return digest;
}

Sha256Context::Sha256Context() { reset(); }

Sha256Context::~Sha256Context() {
  if (ctx_)
    EVP_MD_CTX_free(ctx_);
}

Sha256Context::Sha256Context(Sha256Context &&other) noexcept
    : ctx_(std::exchange(other.ctx_, nullptr)),
      last_digest_(std::move(other.last_digest_)),
      state_(std::exchange(other.state_, State::Uninitialized)) {}

Sha256Context &Sha256Context::operator=(Sha256Context &&other) noexcept {
  if (this != &other) {
    std::swap(ctx_, other.ctx_);
    last_digest_ = std::move(other.last_digest_);
    std::swap(state_, other.state_);
    other.invalidate();
  }
  return *this;
}

void Sha256Context::update(std::span<const std::byte> data) {
  if (state_ == State::Uninitialized)
    throw invalid_context_error{};
  if (state_ == State::Finalized)
    throw hash_error("Context is finalized. Reset first.");
  if (EVP_DigestUpdate(ctx_, data.data(), data.size()) != 1)
    throw hash_error{"EVP_DigestUpdate failed"};
}

Sha256Digest Sha256Context::final() {
  if (state_ == State::Uninitialized)
    throw invalid_context_error{};
  unsigned int outlen = 0;
  if (state_ == State::Finalized)
    return last_digest_;

  if (EVP_DigestFinal_ex(ctx_,
                         reinterpret_cast<unsigned char *>(last_digest_.data()),
                         &outlen) != 1)
    throw hash_error{"EVP_DigestFinal_ex failed"};
  state_ = State::Finalized;
  return last_digest_;
}

bool Sha256Context::finalized() const { return state_ == State::Finalized; }
void Sha256Context::invalidate() {
  if (ctx_)
    EVP_MD_CTX_free(ctx_);
  ctx_ = nullptr;
  state_ = State::Uninitialized;
}
bool Sha256Context::valid() const { return state_ != State::Uninitialized; }
void Sha256Context::reset() {
  if (!ctx_)
    ctx_ = EVP_MD_CTX_new();
  if (!ctx_)
    throw hash_error{"EVP_MD_CTX_new failed"};
  if (EVP_DigestInit_ex(ctx_, EVP_sha256(), nullptr) != 1) {
    EVP_MD_CTX_free(ctx_);
    throw hash_error{"EVP_DigestInit_ex failed"};
  }
  state_ = State::Updating;
}
} // namespace core

std::ostream &operator<<(std::ostream &os, const core::Sha256Digest &digest) {
  auto osflags(os.flags());
  auto oswidth(os.width());
  auto osfill(os.fill());

  for (auto b : digest) {

    os << std::hex << std::setw(2) << std::setfill('0')
       << std::to_integer<int>(b);
  }
  os.flags(osflags);
  os.width(oswidth);
  os.fill(osfill);

  return os;
}
