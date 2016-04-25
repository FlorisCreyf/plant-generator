#version 330 core

out vec3 fColor;

void main()
{
	float r = gl_FragCoord.y / 1000.0f;
	float t = gl_FragCoord.x / 800.0f;
	vec3 color = vec3(t, t, t);
	fColor = color;
}
