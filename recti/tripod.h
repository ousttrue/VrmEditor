#include "model_context.h"
#include "state.h"

namespace recti {

void
ComputeTripodAxisAndVisibility(const ModelContext& mCurrent,
                               bool mAllowAxisFlip,
                               const int axisIndex,
                               State* state,
                               Vec4& dirAxis,
                               Vec4& dirPlaneX,
                               Vec4& dirPlaneY,
                               bool& belowAxisLimit,
                               bool& belowPlaneLimit);

}
