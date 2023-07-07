// https://github.com/CedricGuillemet/ImGuizmo
// v 1.89 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once
#include <stdint.h>

struct ImDrawList;

namespace ImGuizmo {

enum MODE
{
  LOCAL,
  WORLD
};

// call it when you want a gizmo
// Needs view and projection matrices.
// matrix parameter is the source matrix (where will be gizmo be drawn) and
// might be transformed by the function. Return deltaMatrix is optional
// translation is applied in world space
enum OPERATION
{
  TRANSLATE_X = (1u << 0),
  TRANSLATE_Y = (1u << 1),
  TRANSLATE_Z = (1u << 2),
  ROTATE_X = (1u << 3),
  ROTATE_Y = (1u << 4),
  ROTATE_Z = (1u << 5),
  ROTATE_SCREEN = (1u << 6),
  SCALE_X = (1u << 7),
  SCALE_Y = (1u << 8),
  SCALE_Z = (1u << 9),
  BOUNDS = (1u << 10),
  SCALE_XU = (1u << 11),
  SCALE_YU = (1u << 12),
  SCALE_ZU = (1u << 13),

  TRANSLATE = TRANSLATE_X | TRANSLATE_Y | TRANSLATE_Z,
  ROTATE = ROTATE_X | ROTATE_Y | ROTATE_Z | ROTATE_SCREEN,
  SCALE = SCALE_X | SCALE_Y | SCALE_Z,
  SCALEU = SCALE_XU | SCALE_YU | SCALE_ZU, // universal
  UNIVERSAL = TRANSLATE | ROTATE | SCALEU
};

inline OPERATION
operator|(OPERATION lhs, OPERATION rhs)
{
  return static_cast<OPERATION>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline OPERATION&
operator|=(OPERATION& lhs, OPERATION rhs)
{
  lhs = lhs | rhs;
  return lhs;
}

inline OPERATION
operator&(OPERATION lhs, OPERATION rhs)
{
  return static_cast<OPERATION>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

inline bool
operator!=(OPERATION lhs, int rhs)
{
  return static_cast<int>(lhs) != rhs;
}

inline bool
Intersects(OPERATION lhs, OPERATION rhs)
{
  return (lhs & rhs) != 0;
}

// True if lhs contains rhs
inline bool
Contains(OPERATION lhs, OPERATION rhs)
{
  return (lhs & rhs) == rhs;
}

void
SetRect(float x, float y, float width, float height);

bool
Manipulate(void* id,
           const float* view,
           const float* projection,
           OPERATION operation,
           MODE mode,
           float* matrix,
           float* deltaMatrix = nullptr,
           const float* snap = nullptr,
           const float* localBounds = nullptr,
           const float* boundsSnap = nullptr);

} // namespace
