#include "model_context.h"

namespace recti {

struct Tripod
{
  Vec4 Axis;
  Vec4 PlaneX;
  Vec4 PlaneY;
  bool VisibleAxis;
  bool VisiblePlane;
  Tripod(const Mat4& mvp,
         float aspectRatio,
         float screenFactor,
         bool mAllowAxisFlip,
         int axisIndex);
};

}
