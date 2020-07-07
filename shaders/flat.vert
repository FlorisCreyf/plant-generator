#version 430 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex;
layout(location = 0) uniform mat4 vp;
out vec4 vertexColor;
out vec2 vertexTex;

void main()
{
	vertexColor = color;
	vertexTex = tex;
	gl_Position = vp * position;
}
