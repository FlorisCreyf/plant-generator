#version 430 core

out vec4 fragmentColor;
in vec4 vertexColor;
in vec2 vertexTex;

uniform sampler2D image;

void main()
{
	vec4 c = texture(image, vertexTex);
	if(c.a < 0.1)
		discard;
	fragmentColor = c;
}
