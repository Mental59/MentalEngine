#include "fpsCounter.hpp"
#include <cassert>

FpsCounter::FpsCounter(float avgInterval) : mAvgInterval(avgInterval) {
  assert(avgInterval > 0.0f);
}

bool FpsCounter::tick(float deltaSeconds, bool frameRendered) {
  if (frameRendered) {
    mNumFrames++;
  }

  mAccumulatedTime += deltaSeconds;

  if (mAccumulatedTime > mAvgInterval) {
    mCurrentFPS = static_cast<float>(mNumFrames / mAccumulatedTime);

    mNumFrames = 0;
    mAccumulatedTime = 0.0;

    return true;
  }

  return false;
}
