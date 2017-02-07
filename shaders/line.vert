#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;

layout(location = 0) uniform mat4 vp;

out VertexData {
	vec4 color;
} vertexOut;

void main()
{
	vertexOut.color = vColor;
	gl_Position = vp * vPosition;
}
