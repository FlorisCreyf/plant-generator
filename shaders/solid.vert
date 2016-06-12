#version 330 core

layout (location = 0) in vec4 vPosition;
uniform mat4 vp;
out vec4 iColor;

void main()
{
	iColor = vec4(0.4f, 0.4f, 0.4f, 0.0f);
	gl_Position = vp * vPosition;
}
