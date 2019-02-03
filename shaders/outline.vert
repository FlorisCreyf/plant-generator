#version 430 core

layout(location = 0) in vec4 point;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 tex;
layout(location = 0) uniform mat4 vp;
out vec4 vertexColor;

void main()
{
	gl_Position = vp * point;
}
