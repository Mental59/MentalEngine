#pragma once
#include <glm/glm.hpp>
#include "image/bitmap.hpp"

namespace mental
{
glm::vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize);
Bitmap convertEquirectangularMapToVerticalCross(const Bitmap& b);
Bitmap convertVerticalCrossToCubeMapFaces(const Bitmap& b);
}  // namespace mental
