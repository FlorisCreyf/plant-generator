#version 330 core

layout (location = 0) in vec4 point;
layout (location = 1) in vec4 normal;

uniform mat4 vp;
uniform vec4 cameraPosition;

out vec3 iColor;

void main()
{
	vec4 lo = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec4 kd = vec4(0.6f, 0.6f, 0.6f, 0.0f);
	vec4 ks = vec4(0.6f, 0.6f, 0.6f, 0.0f);
	vec4 el = vec4(0.9f, 0.9f, 0.9f, 0.0f);
	vec4 light = vec4(2.0f, 2.0f, 2.0f, 0.0f);

	vec4 v = normalize(cameraPosition - point);
	vec4 h = normalize(v + light);

	float costh = clamp(dot(normal, h), 0.0f, 1.0f);
	float costi = clamp(dot(normal, light), 0.0f, 1.0f);

	lo += kd + ks * pow(costh, 3) * el * costi;

	iColor.r = lo.r;
	iColor.g = lo.g;
	iColor.b = lo.b;
	gl_Position = vp * point;
}
