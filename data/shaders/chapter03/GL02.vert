#version 460 core

layout(std140, binding = 0) uniform PerFrameData
{
	uniform mat4 MVP;
};

struct Vertex
{
	float p[3];
	float tc[2];
};

layout(std430, binding = 1) restrict readonly buffer Vertices
{
	Vertex in_Vertices[];
};

layout(std430, binding = 2) restrict readonly buffer Indices
{
	uint in_Indices[];
};

vec3 getPosition(uint i)
{
	return vec3(in_Vertices[i].p[0], in_Vertices[i].p[1], in_Vertices[i].p[2]);
}

vec2 getTexCoord(uint i)
{
	return vec2(in_Vertices[i].tc[0], in_Vertices[i].tc[1]);
}

layout (location=0) out vec2 uv;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	uint vertexIndex = in_Indices[gl_VertexID];

	vec3 pos = getPosition(vertexIndex);
	gl_Position = MVP * vec4(pos, 1.0);

	uv = getTexCoord(vertexIndex);
}
