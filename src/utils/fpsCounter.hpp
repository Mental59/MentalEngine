#pragma once

class FpsCounter {
public:
  explicit FpsCounter(float avgInterval = 0.5f);

  bool tick(float deltaSeconds, bool frameRendered = true);

  inline float getFPS() const { return mCurrentFPS; }

private:
  const float mAvgInterval = 0.5f;
  unsigned int mNumFrames = 0;
  double mAccumulatedTime = 0;
  float mCurrentFPS = 0.0f;
};
