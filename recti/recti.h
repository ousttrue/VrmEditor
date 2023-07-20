#pragma once
#include "camera_mouse.h"
#include "drawcommand.h"
#include "handle/rotation_drag.h"
#include "handle/rotation_gizmo.h"
#include "handle/scale_drag.h"
#include "handle/scale_gizmo.h"
#include "handle/translation_drag.h"
#include "handle/translation_gizmo.h"
#include "operation.h"
#include <assert.h>
#include <list>
#include <memory>

// Rect Interface
//
// originally based on https://github.com/CedricGuillemet/ImGuizmo
// * separate mouse input from ImGui
// * separate render from ImGui
//
namespace recti {

struct Screen
{
  // current frame
  CameraMouse CameraMouse;

  // hover & draw
  std::list<std::shared_ptr<IGizmo>> Gizmos = {
    std::make_shared<TranslationGizmo>(),
    std::make_shared<RotationGizmo>(),
    std::make_shared<ScaleGizmo>(),
    // std::make_shared<UniformScaleGizmo>(),
  };
  // drag & draw
  std::shared_ptr<IDragHandle> DragHandle;

  // draw
  DrawList DrawList;
  Style Style;

  float GizmoSizeClipSpace = 0.1f;

  void Begin(const Camera& camera, const Mouse& mouse)
  {
    DrawList.m_commands.clear();
    CameraMouse.Initialize(camera, mouse);
  }

  bool Manipulate(int64_t actualID,
                  OPERATION operation,
                  MODE mode,
                  float* matrix,
                  float* deltaMatrix = nullptr,
                  const float* snap = nullptr)
  {
    // Scale is always local or matrix will be skewed when applying world scale
    // or oriented matrix
    ModelContext current(
      actualID, operation, mode, matrix, CameraMouse, GizmoSizeClipSpace);

    // behind camera
    Vec4 camSpacePosition;
    camSpacePosition.TransformPoint({ 0.f, 0.f, 0.f }, current.MVP);
    if (!current.CameraMouse.Camera.IsOrthographic &&
        camSpacePosition.z < 0.001f) {
      return false;
    }

    // set delta to identity
    if (deltaMatrix) {
      ((Mat4*)deltaMatrix)->SetToIdentity();
    }

    // process drag / hover
    MOVETYPE active = MT_NONE;
    MOVETYPE hover = MT_NONE;
    auto isActive = false;
    if (DragHandle) {
      // drag
      DragHandle->Drag(current, snap, matrix, deltaMatrix);
      if (!current.CameraMouse.Mouse.LeftDown) {
        // drag end
        isActive = true;
        DragHandle = {};
      }
    } else {
      // hover
      hover = Hover(current);
    }

    // draw
    for (auto& gizmo : Gizmos) {
      if (DragHandle) {
        active = DragHandle->Type();
      }
      gizmo->Draw(current, active, hover, Style, DrawList);
    }
    if (DragHandle) {
      DragHandle->Draw(current, Style, DrawList);
    }

    return DragHandle != nullptr || isActive;
  }

  MOVETYPE Hover(const ModelContext& current)
  {
    assert(!DragHandle);
    MOVETYPE hover = MT_NONE;

    for (auto& gizmo : Gizmos) {
      auto new_hover = gizmo->Hover(current);
      if (new_hover != MT_NONE) {
        hover = new_hover;
        break;
      }
    }

    if (current.CameraMouse.Mouse.LeftDown) {
      DragHandle = CreateDrag(current, hover);
    }
    return hover;
  }

  std::shared_ptr<IDragHandle> CreateDrag(const ModelContext& current,
                                          MOVETYPE hover)
  {
    if (hover >= MT_MOVE_X && hover <= MT_MOVE_SCREEN) {
      return std::make_shared<TranslationDragHandle>(current, hover);
    }
    if (hover >= MT_ROTATE_X && hover <= MT_ROTATE_SCREEN) {
      return std::make_shared<RotationDragHandle>(current, hover);
    }
    if (Intersects(current.Operation, SCALE)) {
      if (hover >= MT_SCALE_X && hover <= MT_SCALE_XYZ) {
        return std::make_shared<ScaleDragHandle>(current, hover);
      }
    } else if (Intersects(current.Operation, SCALEU)) {
      // if (hover >= MT_SCALE_X && hover <= MT_SCALE_XYZ) {
      //   return std::make_shared<UniformScaleDragHandle>(current, hover);
      // }
    }

    return {};
  }

  void DrawCubes(const float* cubes, uint32_t count)
  {
    DrawList.DrawCubes(CameraMouse, cubes, count, Style);
  }
};

} // namespace
