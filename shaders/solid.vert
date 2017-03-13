#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 0) uniform mat4 vp;
out vec4 iColor;

void main()
{
	iColor = vec4(0.0f, 0.26f, 0.4f, 1.0f);
	gl_Position = vp * vPosition;
}
