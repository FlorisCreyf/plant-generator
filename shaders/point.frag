#version 430 core

layout(location = 1) uniform sampler2D tex;

out vec4 fColor;
in vec4 iColor;

void main()
{
	fColor = texture(tex, gl_PointCoord) * iColor;
}
