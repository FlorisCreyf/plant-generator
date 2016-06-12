#version 330 core

layout (location = 0) in vec4 point;
layout (location = 1) in vec4 normal;

uniform mat4 vp;
uniform vec4 cameraPosition;

out vec3 iColor;

void main()
{
	vec4 v = normalize(cameraPosition - point);
	float d = clamp(dot(normal, v), 0.0f, 1.0f);
	d = d/3.5f + 0.55f;

	iColor.r = d;
	iColor.g = d;
	iColor.b = d;
	gl_Position = vp * point;
}
