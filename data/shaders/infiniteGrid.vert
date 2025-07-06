#version 460 core

#include <data/shaders/infiniteGridParams.shader>

layout (location=0) out vec2 oUV;
layout (location=1) out vec2 oCamPos;

layout(binding = 0) uniform UniformBuffer {
	mat4 mvp;
    vec3 camPos;
} ubo;

const vec3 pos[4] = vec3[4](
	vec3(-1.0, 0.0, -1.0),
	vec3( 1.0, 0.0, -1.0),
	vec3( 1.0, 0.0,  1.0),
	vec3(-1.0, 0.0,  1.0)
);

const int indices[6] = int[6](
	0, 1, 2, 2, 3, 0
);

void main() {
    vec3 position = pos[indices[gl_VertexIndex]] * gGridSize;

    position.x += ubo.camPos.x;
    position.z += ubo.camPos.z;

    gl_Position = ubo.mvp * vec4(position, 1.0);

    oUV = position.xz;
    oCamPos = ubo.camPos.xz;
}
