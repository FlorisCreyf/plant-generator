#version 430 core

layout(location = 0) in vec4 point;
layout(location = 1) in vec4 normal;

layout(location = 0) uniform mat4 vp;
layout(location = 1) uniform vec4 cameraPosition;

out vec3 iColor;

void main()
{
	float angle = 3.141f/5.0f;
	mat4 r = mat4(
			cos(angle), 0.0f, sin(angle), 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			-sin(angle), 0.0f, cos(angle), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

	vec4 c = r * cameraPosition * 100;
	vec4 v = normalize(c - point);
	float d = dot(normal, v);
	d = d/3.5f + 0.55f;

	iColor.r = d;
	iColor.g = d + 0.02;
	iColor.b = d + 0.08;
	gl_Position = vp * point;
}
