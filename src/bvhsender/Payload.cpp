#include "Payload.h"
#include <iostream>

Payload::Payload() { std::cout << "Payload::Payload" << std::endl; }
Payload::~Payload() { std::cout << "Payload::~Payload" << std::endl; }

void Payload::Push(const void *begin, const void *end) {
  auto dst = buffer.size();
  auto size = std::distance((const char *)begin, (const char *)end);
  buffer.resize(dst + size);
  std::copy((const char *)begin, (const char *)end, buffer.data() + dst);
}

void Payload::SetSkeleton(std::span<srht::JointDefinition> joints) {
  buffer.clear();

  srht::SkeletonHeader header{
      // .magic = {},
      .skeletonId = 0,
      .jointCount = 27,
      .flags = {},
  };
  Push((const char *)&header, (const char *)&header + sizeof(header));
  Push(joints.data(), joints.data() + joints.size());
}

void Payload::SetFrame(std::chrono::nanoseconds time, float x, float y, float z,
                       bool usePack) {
  buffer.clear();

  srht::FrameHeader header{
      // .magic = {},
      .time = time.count(),
      .flags = usePack ? srht::FrameFlags::USE_QUAT32 : srht::FrameFlags::NONE,
      .skeletonId = 0,
      .x = x,
      .y = y,
      .z = z,
  };
  Push((const char *)&header, (const char *)&header + sizeof(header));
}
