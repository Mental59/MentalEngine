#pragma once

#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace mental
{
enum eBitmapType
{
	eBitmapType_2D,
	eBitmapType_Cube
};

enum eBitmapFormat
{
	eBitmapFormat_UnsignedByte,
	eBitmapFormat_Float,
};

struct Bitmap
{
	Bitmap() = default;
	Bitmap(int w, int h, int comp, eBitmapFormat fmt);
	Bitmap(int w, int h, int d, int comp, eBitmapFormat fmt);
	Bitmap(int w, int h, int comp, eBitmapFormat fmt, const void* ptr);

	static int getBytesPerComponent(eBitmapFormat fmt);

	void setPixel(int x, int y, const glm::vec4& c);
	glm::vec4 getPixel(int x, int y) const;

	int w_ = 0;
	int h_ = 0;
	int d_ = 1;
	int comp_ = 3;
	eBitmapFormat fmt_ = eBitmapFormat_UnsignedByte;
	eBitmapType type_ = eBitmapType_2D;
	std::vector<uint8_t> data_;

private:
	using setPixel_t = void (Bitmap::*)(int, int, const glm::vec4&);
	using getPixel_t = glm::vec4 (Bitmap::*)(int, int) const;
	setPixel_t setPixelFunc = &Bitmap::setPixelUnsignedByte;
	getPixel_t getPixelFunc = &Bitmap::getPixelUnsignedByte;

	void initGetSetFuncs();

	void setPixelFloat(int x, int y, const glm::vec4& c);
	glm::vec4 getPixelFloat(int x, int y) const;

	void setPixelUnsignedByte(int x, int y, const glm::vec4& c);
	glm::vec4 getPixelUnsignedByte(int x, int y) const;
};
}  // namespace mental
