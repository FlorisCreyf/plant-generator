#version 430 core

layout(location = 1) uniform sampler2D tex;

out vec4 fragmentColor;
in vec4 vertexColor;

void main()
{
	fragmentColor = texture(tex, gl_PointCoord) * vertexColor;
}
