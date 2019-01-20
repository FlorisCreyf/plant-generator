#version 430 core

out vec4 fragmentColor;
in vec4 vertexColor;
in vec2 vertexTex;

uniform sampler2D image;

void main()
{
	fragmentColor = texture(image, vertexTex);
}
