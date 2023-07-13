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

#include "ImGuizmo.h"
#include "../handle/rotation.h"
#include "../handle/scale.h"
#include "../handle/translation.h"
#include "../state.h"
#include "../style.h"
#include <memory>

namespace ImGuizmo {

class ContextImpl
{
  // over frame
  recti::State mState = {};

  // current frame
  recti::CameraMouse mCameraMouse;

  // draw
  std::shared_ptr<recti::DrawList> mDrawList;
  recti::Style mStyle;

  float mRadiusSquareCenter = 0.0f;

  recti::Translation mT;
  recti::Rotation mR;
  recti::Scale mS;

  bool mIsOrthographic = false;
  bool mAllowAxisFlip = true;
  float mGizmoSizeClipSpace = 0.1f;

public:
  ContextImpl() { mDrawList = std::make_shared<recti::DrawList>(); }

  void Begin(const recti::Camera& camera, const recti::Mouse& mouse)
  {
    mDrawList->m_commands.clear();
    mCameraMouse.Initialize(camera, mouse);
  }

  bool Manipulate(int64_t actualID,
                  recti::OPERATION operation,
                  recti::MODE mode,
                  float* matrix,
                  float* deltaMatrix,
                  const float* snap)
  {
    // Scale is always local or matrix will be skewed when applying world scale
    // or oriented matrix
    recti::ModelContext mCurrent(
      actualID, operation, mode, matrix, mCameraMouse, mGizmoSizeClipSpace);

    // set delta to identity
    if (deltaMatrix) {
      ((recti::Mat4*)deltaMatrix)->SetToIdentity();
    }

    // behind camera
    recti::Vec4 camSpacePosition;
    camSpacePosition.TransformPoint({ 0.f, 0.f, 0.f }, mCurrent.mMVP);
    if (!mIsOrthographic && camSpacePosition.z < 0.001f) {
      return false;
    }

    // --
    auto resultT = mT.HandleTranslation(
      mCurrent, mAllowAxisFlip, mState, snap, matrix, deltaMatrix);
    recti::Result resultR{};
    recti::Result resultS{};
    if (!resultT.Modified) {
      resultS = mS.HandleScale(
        mCurrent, mAllowAxisFlip, mState, snap, matrix, deltaMatrix);
      if (!resultS.Modified) {
        resultR = mR.HandleRotation(
          mCurrent, mRadiusSquareCenter, mState, snap, matrix, deltaMatrix);
      }
    }

    auto type = static_cast<recti::MOVETYPE>(
      resultT.DrawType | resultR.DrawType | resultS.DrawType);

    mR.DrawRotationGizmo(mCurrent,
                         mRadiusSquareCenter,
                         mIsOrthographic,
                         type,
                         mState,
                         mStyle,
                         mDrawList);
    mT.DrawTranslationGizmo(
      mCurrent, mAllowAxisFlip, type, mState, mStyle, mDrawList);
    mS.DrawScaleGizmo(mCurrent, type, mState, mStyle, mDrawList);
    mS.DrawScaleUniveralGizmo(
      mCurrent, mAllowAxisFlip, type, mState, mStyle, mDrawList);

    return resultT.Modified || resultR.Modified || resultS.Modified;
  }

  const recti::DrawList& GetDrawList() const { return *mDrawList; }

  void DrawCubes(const float* cubes, uint32_t count)
  {
    mDrawList->DrawCubes(mCameraMouse, cubes, count, mStyle);
  }
};

//
// Context
//
Context::Context()
  : m_impl(new ContextImpl)
{
}

Context::~Context()
{
  delete m_impl;
}

void
Context::Begin(const recti::Camera& camera, const recti::Mouse& mouse)
{
  m_impl->Begin(camera, mouse);
}

bool
Context::Manipulate(void* id,
                    recti::OPERATION operation,
                    recti::MODE mode,
                    float* matrix,
                    float* deltaMatrix,
                    const float* snap)
{
  return m_impl->Manipulate(
    (int64_t)id, operation, mode, matrix, deltaMatrix, snap);
}

const recti::DrawList&
Context::End()
{
  return m_impl->GetDrawList();
}

void
Context::DrawCubes(const float* cubes, uint32_t count)
{
  m_impl->DrawCubes(cubes, count);
}

} // namespace
