module;

#include <cstddef>
#include <iomanip>
#include <ostream>
#include <span>
#include <stdexcept>
#include <utility>

#include <openssl/evp.h>
#include <openssl/sha.h>

module core.hash;

namespace core {
Sha256Digest sha256(std::span<const std::byte> data) {
  Sha256Digest digest;
  SHA256(reinterpret_cast<const unsigned char *>(data.data()), data.size(),
         reinterpret_cast<unsigned char *>(digest.data()));
  return digest;
}

class hash_error : public std::runtime_error {
public:
  explicit hash_error(std::string msg) : std::runtime_error(std::move(msg)) {}
};

Sha256Context::Sha256Context() {
  ctx_ = EVP_MD_CTX_new();
  if (!ctx_)
    throw hash_error{"EVP_MD_CTX_new failed"};
  if (EVP_DigestInit_ex(ctx_, EVP_sha256(), nullptr) != 1) {
    EVP_MD_CTX_free(ctx_);
    throw hash_error{"EVP_DigestInit_ex failed"};
  }
}
Sha256Context::~Sha256Context() {
  if (ctx_)
    EVP_MD_CTX_free(ctx_);
}
Sha256Context::Sha256Context(Sha256Context &&other) noexcept
    : ctx_(std::exchange(other.ctx_, nullptr)) {}

Sha256Context &Sha256Context::operator=(Sha256Context &&other) noexcept {
  if (this != &other) {
    EVP_MD_CTX_free(ctx_);
    ctx_ = other.ctx_;
    other.ctx_ = nullptr;
  }
  return *this;
}
void Sha256Context::update(std::span<const std::byte> data) {
  if (!ctx_)
    throw hash_error{"Uninitialized context"};
  if (EVP_DigestUpdate(ctx_, data.data(), data.size()) != 1)
    throw hash_error{"EVP_DigestUpdate failed"};
}
Sha256Digest Sha256Context::final() {
  if (!ctx_)
    throw hash_error{"Uninitialized context"};
  Sha256Digest out{};
  unsigned int outlen = 0;
  if (EVP_DigestFinal_ex(ctx_, reinterpret_cast<unsigned char *>(out.data()),
                         &outlen) != 1)
    throw hash_error{"EVP_DigestFinal_ex failed"};
  // WARN: We should not allow update after finalize. So We should add a new
  // member to tract the state and beable to reset.
  return out;
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
