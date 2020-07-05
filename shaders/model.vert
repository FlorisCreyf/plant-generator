#version 430 core

layout(location = 0) in vec4 point;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 tex;
layout(location = 0) uniform mat4 vp;

#ifdef SOLID

out vec3 intPosition;

void main()
{
	gl_Position = vp * point;
	intPosition = vec3(point) / point.w;
}

#endif
#ifdef WIREFRAME

layout(location = 1) uniform vec4 color;
out vec4 vertexColor;

void main()
{
	vertexColor = color;
	gl_Position = vp * point;
}

#endif
#ifdef MATERIAL

out vec2 vertTexCoord;

void main()
{
	vertTexCoord = tex;
	gl_Position = vp * point;
}

#endif
