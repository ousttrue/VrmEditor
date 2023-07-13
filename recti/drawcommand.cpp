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

  // void DrawGrid(const float* view,
  //               const float* projection,
  //               const float* matrix,
  //               const float gridSize)
  // {
  //   matrix_t viewProjection = *(matrix_t*)view * *(matrix_t*)projection;
  //   vec_t frustum[6];
  //   ComputeFrustumPlanes(frustum, &viewProjection.m00);
  //   matrix_t res = *(matrix_t*)matrix * viewProjection;
  //
  //   for (float f = -gridSize; f <= gridSize; f += 1.f) {
  //     for (int dir = 0; dir < 2; dir++) {
  //       vec_t ptA = { dir ? -gridSize : f, 0.f, dir ? f : -gridSize };
  //       vec_t ptB = { dir ? gridSize : f, 0.f, dir ? f : gridSize };
  //       bool visible = true;
  //       for (int i = 0; i < 6; i++) {
  //         float dA = DistanceToPlane(ptA, frustum[i]);
  //         float dB = DistanceToPlane(ptB, frustum[i]);
  //         if (dA < 0.f && dB < 0.f) {
  //           visible = false;
  //           break;
  //         }
  //         if (dA > 0.f && dB > 0.f) {
  //           continue;
  //         }
  //         if (dA < 0.f) {
  //           float len = fabsf(dA - dB);
  //           float t = fabsf(dA) / len;
  //           ptA.Lerp(ptB, t);
  //         }
  //         if (dB < 0.f) {
  //           float len = fabsf(dB - dA);
  //           float t = fabsf(dB) / len;
  //           ptB.Lerp(ptA, t);
  //         }
  //       }
  //       if (visible) {
  //         ImU32 col = IM_COL32(0x80, 0x80, 0x80, 0xFF);
  //         col = (fmodf(fabsf(f), 10.f) < FLT_EPSILON)
  //                 ? IM_COL32(0x90, 0x90, 0x90, 0xFF)
  //                 : col;
  //         col =
  //           (fabsf(f) < FLT_EPSILON) ? IM_COL32(0x40, 0x40, 0x40, 0xFF) : col;
  //
  //         float thickness = 1.f;
  //         thickness = (fmodf(fabsf(f), 10.f) < FLT_EPSILON) ? 1.5f : thickness;
  //         thickness = (fabsf(f) < FLT_EPSILON) ? 2.3f : thickness;
  //
  //         mDrawList->AddLine(
  //           worldToPos(ptA, res), worldToPos(ptB, res), col, thickness);
  //       }
  //     }
  //   }
  // }
} // namespace
