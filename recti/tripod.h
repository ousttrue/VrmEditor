#include "model_context.h"
#include "state.h"

namespace recti {

struct Tripod
{
  Vec4 dirAxis;
  Vec4 dirPlaneX;
  Vec4 dirPlaneY;
  bool belowAxisLimit;
  bool belowPlaneLimit;
  float mulAxis = 1;
  float mulAxisX = 1;
  float mulAxisY = 1;

  // ComputeTripodAxisAndVisibility
  Tripod(const ModelContext& mCurrent,
         bool mAllowAxisFlip,
         const int axisIndex,
         State* state);
};

}
