#pragma once

#include <stdint.h>

class VdexHeader
{
 public:
  const char* GetMagic() const { return reinterpret_cast<const char*>(magic_); }
  uint32_t GetDexSize() const { return dex_size_; }

 private:
  static constexpr uint8_t kVdexMagic[] = { 'v', 'd', 'e', 'x' };
  static constexpr uint8_t kVdexVersion[] = { '0', '1', '0', '\0' };

  uint8_t magic_[4];
  uint8_t version_[4];
  uint32_t number_of_dex_files_;
  uint32_t dex_size_;
  uint32_t verifier_deps_size_;
  uint32_t quickening_info_size_;
};

#define kVdexMagicAndVersion "vdex010"
