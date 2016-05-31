#version 330 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vColor;
uniform mat4 vp;
out vec4 iColor;

void main()
{
	iColor = vColor;
	gl_Position = vp * vPosition;
}
