module;

#include <cstddef>
#include <iomanip>
#include <openssl/sha.h>
#include <ostream>
#include <span>

module core.hash;

namespace core {
Sha256Digest sha256(std::span<const std::byte> data) {
  Sha256Digest digest;
  SHA256(reinterpret_cast<const unsigned char *>(data.data()), data.size(),
         reinterpret_cast<unsigned char *>(digest.data()));
  return digest;
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
