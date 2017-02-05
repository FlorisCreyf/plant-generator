#version 430 core

layout (location = 0) in vec4 vPosition;
layout (location = 0) uniform mat4 vp;
out vec4 iColor;

void main()
{
	iColor = vec4(0.2f, 0.46f, 0.6f, 0.0f);
	gl_Position = vp * vPosition;
}
