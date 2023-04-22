#pragma once
#include <functional>

struct ImVec2;
namespace glr {
void
Overlay(const ImVec2& pos,
        const char* title,
        const char* popupName,
        const std::function<void()>& popup = {});
}
