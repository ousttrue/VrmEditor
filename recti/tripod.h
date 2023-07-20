#include "model_context.h"

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
  Tripod(const int axisIndex);
  void ComputeTripodAxisAndVisibility(const ModelContext& mCurrent,
                                      bool mAllowAxisFlip);
};

}
