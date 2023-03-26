#pragma once
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <string_view>

struct Sequence {
  Sequence();
  ~Sequence();
  std::function<void(std::chrono::milliseconds)> callback;
};

class Timeline {
  std::list<std::shared_ptr<Sequence>> m_sequences;

public:
  Timeline();
  ~Timeline();

  void setGlobal(std::chrono::milliseconds time);

  std::shared_ptr<Sequence> addSequence(std::string_view name,
                                        std::chrono::milliseconds duration);
};
