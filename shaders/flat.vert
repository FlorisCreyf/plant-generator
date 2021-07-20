#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in float tangentScale;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec2 indices;
layout(location = 6) in vec2 weights;
layout(location = 0) uniform mat4 transform;
out vec4 vertexColor;
out vec2 vertexUV;

void main()
{
	vertexUV = uv;
	vertexColor = vec4(normal, 1.0);
	gl_Position = transform * vec4(position, 1.0);
}
