#version 460 core

#include <data/shaders/infiniteGridParams.shader>

layout (location=0) in vec2 oUV;
layout (location=1) in vec2 oCamPos;
layout (location=0) out vec4 oFragColor;

float log10(float x) {
	return log(x) / log(10.0);
}

float satf(float x) {
	return clamp(x, 0.0, 1.0);
}

vec2 satv(vec2 x) {
	return clamp(x, vec2(0.0), vec2(1.0));
}

float max2(vec2 v) {
	return max(v.x, v.y);
}

vec4 gridColor(vec2 uv, vec2 camPos)
{
	vec2 dudv = vec2(
		length( vec2(dFdx(uv.x), dFdy(uv.x)) ),
		length( vec2(dFdx(uv.y), dFdy(uv.y)) )
	);

	const float lodLevel = max( 0.0, log10((length(dudv) * gGridMinPixelsBetweenCells) / gGridCellSize) + 1.0 );
	const float lodFade = fract( lodLevel );

	// cell sizes for lod0, lod1 and lod2
	const float lod0 = gGridCellSize * pow( 10.0, floor(lodLevel) );
	const float lod1 = lod0 * 10.0;
	const float lod2 = lod1 * 10.0;

	// each anti-aliased line covers up to 2 pixels
	dudv *= 2.0;

	// Update grid coordinates for subsequent alpha calculations (centers each anti-aliased line)
  	uv += dudv / 2.0;

	// calculate absolute distances to cell line centers for each lod and pick max X/Y to get coverage alpha value
	const float lod0a = max2( vec2(1.0) - abs(satv(mod(uv, lod0) / dudv) * 2.0 - vec2(1.0)) );
	const float lod1a = max2( vec2(1.0) - abs(satv(mod(uv, lod1) / dudv) * 2.0 - vec2(1.0)) );
	const float lod2a = max2( vec2(1.0) - abs(satv(mod(uv, lod2) / dudv) * 2.0 - vec2(1.0)) );

	// substract camera pos to ensure we do not run off the grid
	uv -= camPos;

	// blend between falloff colors to handle LOD transition
	// blend between LOD level alphas and scale with opacity falloff
	vec4 c = vec4(0.0);
	if (lod2a > 0.0) {
		c = gGridColorThick;
		c.a *= lod2a;
	} else if (lod1a > 0.0) {
		c = mix(gGridColorThick, gGridColorThin, lodFade);
		c.a *= lod1a;
	} else {
		c = gGridColorThin;
		c.a *= lod0a * (1.0 - lodFade);
	}

	// calculate opacity falloff based on distance to grid extents
	const float opacityFalloff = (1.0 - satf(length(uv) / gGridSize));
	c.a *= opacityFalloff;

	return c;
}

void main() {
    oFragColor = gridColor(oUV, oCamPos);
}
