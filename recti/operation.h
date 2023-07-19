#pragma once

namespace recti {

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

enum MOVETYPE
{
  MT_NONE,
  MT_MOVE_X,
  MT_MOVE_Y,
  MT_MOVE_Z,
  MT_MOVE_YZ,
  MT_MOVE_ZX,
  MT_MOVE_XY,
  MT_MOVE_SCREEN,
  MT_ROTATE_X,
  MT_ROTATE_Y,
  MT_ROTATE_Z,
  MT_ROTATE_SCREEN,
  MT_SCALE_X,
  MT_SCALE_Y,
  MT_SCALE_Z,
  MT_SCALE_XYZ
};
inline bool
IsTranslateType(MOVETYPE type)
{
  return type >= MT_MOVE_X && type <= MT_MOVE_SCREEN;
}
inline bool
IsRotateType(MOVETYPE type)
{
  return type >= MT_ROTATE_X && type <= MT_ROTATE_SCREEN;
}
inline bool
IsScaleType(MOVETYPE type)
{
  return type >= MT_SCALE_X && type <= MT_SCALE_XYZ;
}

struct Operation
{
  bool EnableT = false;
  bool EnableR = false;
  bool EnableS = false;
  bool IsLocalSpace = false;
};

inline OPERATION
ToOperation(const Operation& o)
{
  OPERATION operation = {};
  if (o.EnableT) {
    operation |= TRANSLATE;
  }
  if (o.EnableR) {
    operation |= ROTATE;
  }
  if (o.EnableS) {
    operation |= SCALE;
  }
  return operation;
}

inline MODE
ToMode(const Operation& o)
{
  return LOCAL;
}

} // namespace
