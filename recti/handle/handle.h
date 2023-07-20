#pragma once
#include "../drawcommand.h"
#include "../model_context.h"
#include "../style.h"
#include <memory>

namespace recti {

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

struct IGizmo
{
  virtual ~IGizmo(){};

  virtual bool Enabled(OPERATION operation) const = 0;

  virtual MOVETYPE Hover(const ModelContext& current) = 0;

  virtual void Draw(const ModelContext& current,
                    MOVETYPE active,
                    MOVETYPE hover,
                    const Style& style,
                    DrawList& drawList) = 0;
};

struct IDragHandle
{
  virtual ~IDragHandle(){};

  virtual MOVETYPE Type() const = 0;

  virtual bool Drag(const ModelContext& current,
                    const float* snap,
                    float* matrix,
                    float* deltaMatrix) = 0;

  virtual void Draw(const ModelContext& current,
                    const Style& style,
                    DrawList& drawList) = 0;
};

}
