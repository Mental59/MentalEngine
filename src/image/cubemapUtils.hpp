#pragma once
#include <glm/glm.hpp>
#include "image/bitmap.hpp"

using glm::vec3;

vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize);
Bitmap convertEquirectangularMapToVerticalCross(const Bitmap& b);
Bitmap convertVerticalCrossToCubeMapFaces(const Bitmap& b);
