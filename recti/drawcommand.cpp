#include "drawcommand.h"
#include "mat4.h"
#include "model_context.h"
#include <stdlib.h>

namespace recti {

void
DrawList::DrawHatchedAxis(const ModelContext& mCurrent,
                          const recti::Vec4& axis,
                          const Style& mStyle)
{
  if (mStyle.HatchedAxisLineThickness <= 0.0f) {
    return;
  }

  for (int j = 1; j < 10; j++) {
    recti::Vec2 baseSSpace2 =
      recti::worldToPos(axis * 0.05f * (float)(j * 2) * mCurrent.mScreenFactor,
                        mCurrent.mMVP,
                        mCurrent.mCameraMouse.Camera.Viewport);
    recti::Vec2 worldDirSSpace2 = recti::worldToPos(
      axis * 0.05f * (float)(j * 2 + 1) * mCurrent.mScreenFactor,
      mCurrent.mMVP,
      mCurrent.mCameraMouse.Camera.Viewport);
    AddLine(baseSSpace2,
            worldDirSSpace2,
            mStyle.GetColorU32(HATCHED_AXIS_LINES),
            mStyle.HatchedAxisLineThickness);
  }
}

void
DrawList::DrawCubes(const CameraMouse& cameraMouse,
                    const float* matrices,
                    int matrixCount,
                    const Style& mStyle)
{
  // matrix_t viewInverse;
  // viewInverse.Inverse(*(matrix_t*)view);

  struct CubeFace
  {
    float z;
    Vec2 faceCoordsScreen[4];
    uint32_t color;
  };
  std::vector<CubeFace> faces(matrixCount * 6);

  Vec4 frustum[6];
  auto viewProjection = cameraMouse.mViewProjection;
  ComputeFrustumPlanes(frustum, &viewProjection.m00);

  int cubeFaceCount = 0;
  for (int cube = 0; cube < matrixCount; cube++) {
    const float* matrix = &matrices[cube * 16];

    auto res = *(Mat4*)matrix * cameraMouse.mViewProjection;

    for (int iFace = 0; iFace < 6; iFace++) {
      const int normalIndex = (iFace % 3);
      const int perpXIndex = (normalIndex + 1) % 3;
      const int perpYIndex = (normalIndex + 2) % 3;
      const float invert = (iFace > 2) ? -1.f : 1.f;

      const Vec4 faceCoords[4] = {
        Vec4::DirectionUnary[normalIndex] + Vec4::DirectionUnary[perpXIndex] +
          Vec4::DirectionUnary[perpYIndex],
        Vec4::DirectionUnary[normalIndex] + Vec4::DirectionUnary[perpXIndex] -
          Vec4::DirectionUnary[perpYIndex],
        Vec4::DirectionUnary[normalIndex] - Vec4::DirectionUnary[perpXIndex] -
          Vec4::DirectionUnary[perpYIndex],
        Vec4::DirectionUnary[normalIndex] - Vec4::DirectionUnary[perpXIndex] +
          Vec4::DirectionUnary[perpYIndex],
      };

      // clipping
      /*
      bool skipFace = false;
      for (unsigned int iCoord = 0; iCoord < 4; iCoord++)
      {
         vec_t camSpacePosition;
         camSpacePosition.TransformPoint(faceCoords[iCoord] * 0.5f * invert,
      res); if (camSpacePosition.z < 0.001f)
         {
            skipFace = true;
            break;
         }
      }
      if (skipFace)
      {
         continue;
      }
      */
      Vec4 centerPosition, centerPositionVP;
      centerPosition.TransformPoint(
        Vec4::DirectionUnary[normalIndex] * 0.5f * invert, *(Mat4*)matrix);
      centerPositionVP.TransformPoint(
        Vec4::DirectionUnary[normalIndex] * 0.5f * invert, res);

      bool inFrustum = true;
      for (int iFrustum = 0; iFrustum < 6; iFrustum++) {
        float dist = DistanceToPlane(centerPosition, frustum[iFrustum]);
        if (dist < 0.f) {
          inFrustum = false;
          break;
        }
      }

      if (!inFrustum) {
        continue;
      }
      CubeFace& cubeFace = faces[cubeFaceCount];

      // 3D->2D
      // ImVec2 faceCoordsScreen[4];
      for (unsigned int iCoord = 0; iCoord < 4; iCoord++) {
        cubeFace.faceCoordsScreen[iCoord] =
          worldToPos(faceCoords[iCoord] * 0.5f * invert,
                     res,
                     cameraMouse.Camera.Viewport);
      }

      auto directionColor = mStyle.GetColorU32(DIRECTION_X + normalIndex);
      cubeFace.color = directionColor | COL32(0x80, 0x80, 0x80, 0);

      cubeFace.z = centerPositionVP.z / centerPositionVP.w;
      cubeFaceCount++;
    }
  }

  qsort(faces.data(),
        cubeFaceCount,
        sizeof(CubeFace),
        [](void const* _a, void const* _b) {
          CubeFace* a = (CubeFace*)_a;
          CubeFace* b = (CubeFace*)_b;
          if (a->z < b->z) {
            return 1;
          }
          return -1;
        });
  // draw face with lighter color
  for (int iFace = 0; iFace < cubeFaceCount; iFace++) {
    const CubeFace& cubeFace = faces[iFace];
    AddConvexPolyFilled(cubeFace.faceCoordsScreen, 4, cubeFace.color);
  }
}

} // namespace
