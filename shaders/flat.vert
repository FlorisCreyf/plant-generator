#version 330 core

layout (location = 0) in vec4 vPosition;
uniform mat4 matrix;

out vec4 color;

void main()
{
	color = vec4(0.2f, 0.4f, 0.6f, 1.0f);
	gl_Position = matrix * vPosition;
}
