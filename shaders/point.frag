#version 430 core

layout(location = 1) uniform sampler2D image;
out vec4 fragmentColor;
in vec4 vertexColor;

void main()
{
	fragmentColor = texture(image, gl_PointCoord) * vertexColor;
}
