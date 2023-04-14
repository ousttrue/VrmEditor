#pragma once
#include <chrono>
#include <span>
#include <vector>
#include <vrm/srht.h>

struct Payload
{
  std::vector<uint8_t> buffer;

  Payload(const Payload&) = delete;
  Payload& operator=(const Payload&) = delete;
  Payload();
  ~Payload();
  void Push(const void* begin, const void* end);
  template<typename T>
  void Push(const T& t)
  {
    Push((const char*)&t, (const char*)&t + sizeof(T));
  }
  void SetSkeleton(std::span<libvrm::srht::JointDefinition> joints);
  void SetFrame(std::chrono::nanoseconds time,
                float x,
                float y,
                float z,
                bool usePack);
};
