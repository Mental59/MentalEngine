#pragma once
#include "image/bitmap.hpp"
#include <glm/glm.hpp>

namespace image {
glm::vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize);
Bitmap convertEquirectangularMapToVerticalCross(const Bitmap& b);
Bitmap convertVerticalCrossToCubeMapFaces(const Bitmap& b);
} // namespace image
