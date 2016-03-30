#version 330 core

out vec3 fColor;

void main()
{
	float r = gl_FragCoord.y / 1000.0f;
	float t = gl_FragCoord.x / 1000.0f;
	vec3 color = vec3(r, r, t);
	fColor = color;
}
