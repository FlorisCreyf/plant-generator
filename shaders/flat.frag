#version 430 core

out vec4 fragmentColor;
in vec4 vertexColor;

void main()
{
	fragmentColor = vertexColor;
}
