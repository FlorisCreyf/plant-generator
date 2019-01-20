#version 430 core

layout(location = 0) in vec4 point;
layout(location = 0) uniform mat4 vp;
layout(location = 1) uniform vec4 color;
out vec4 vertexColor;

void main()
{
	vertexColor = color;
	gl_Position = vp * point;
}
