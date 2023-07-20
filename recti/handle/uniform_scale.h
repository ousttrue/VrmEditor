struct UniformScaleGizmo : IGizmo
{
  bool m_allowAxisFlip = true;

  MOVETYPE Hover(const ModelContext& current) override;

  void Draw(const ModelContext& current,
            MOVETYPE active,
            MOVETYPE hover,
            const Style& style,
            DrawList& drawList) override;
};

// struct UniformScaleDragHandle : public IDragHandle
// {
//   MOVETYPE m_type;
//   Vec4 mTranslationPlan;
//   Vec4 mTranslationPlanOrigin;
//   Vec4 mMatrixOrigin;
//   Vec4 mTranslationLastDelta;
//   Vec4 mRelativeOrigin;
//
//   Vec4 mScale;
//   Vec4 mScaleValueOrigin;
//   Vec4 mScaleLast;
//   float mSaveMousePosx;
//
//   UniformScaleDragHandle(const ModelContext& mCurrent, MOVETYPE type);
//
//   MOVETYPE Type() const override { return m_type; }
//
//   bool Drag(const ModelContext& current,
//             const float* snap,
//             float* matrix,
//             float* deltaMatrix) override;
//
//   void Draw(const ModelContext& current,
//             const Style& style,
//             DrawList& drawList) override;
// };


