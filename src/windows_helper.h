#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <string>

inline std::string WideToMb(uint32_t cp, const wchar_t *src) {
  auto l = WideCharToMultiByte(cp, 0, src, -1, nullptr, 0, nullptr, nullptr);
  std::string dst;
  dst.resize(l);
  l = WideCharToMultiByte(cp, 0, src, -1, dst.data(), l, nullptr, nullptr);
  dst.push_back(0);
  return dst;
}
