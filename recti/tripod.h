#include "model_context.h"

namespace recti {

struct Tripod
{
  Vec4 dirAxis;
  Vec4 dirPlaneX;
  Vec4 dirPlaneY;
  bool belowAxisLimit;
  bool belowPlaneLimit;
  Tripod(const Mat4& mvp,
         float displayRatio,
         float screenFactor,
         bool mAllowAxisFlip,
         int axisIndex);
};

}
