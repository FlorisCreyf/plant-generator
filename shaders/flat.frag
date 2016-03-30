#version 330 core

out vec4 fColor;
in vec4 color;

void main()
{
	fColor = color;
}