#version 430 core

out vec4 fragmentColor;

#ifdef SOLID

in vec3 vertexPosition;
layout(location = 1) uniform vec4 cameraPosition;

void main()
{
	vec3 ambientColor = vec3(0.2, 0.2, 0.2);
	vec3 diffuseColor = vec3(0.5, 0.5, 0.5);
	vec3 specColor = vec3(0.5, 0.5, 0.5);

	vec3 normal = cross(dFdx(vertexPosition), dFdy(vertexPosition));
	normal = normalize(normal);
	vec3 lightDir = normalize(cameraPosition.xyz - vertexPosition);
	float lambertian = max(dot(lightDir, normal), 0.1);
	float specular = 0.0;

	if (lambertian > 0.0) {
		vec3 viewDir = normalize(-cameraPosition.xyz);
		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(halfDir, normal), 0);
		specular = pow(specAngle, 16.0);
	}

	diffuseColor *= lambertian;
	specColor *= specular;
	fragmentColor = vec4(ambientColor + diffuseColor + specColor, 1.0);
}

#endif /* SOLID */
#ifdef WIREFRAME

in vec4 vertexColor;

void main()
{
	fragmentColor = vertexColor;
}

#endif /* WIREFRAME */
#ifdef MATERIAL

in vec2 vertTexCoord;
uniform sampler2D image;

void main()
{
	vec4 color = texture(image, vertTexCoord);
	if(color.a < 0.1)
		discard;
	fragmentColor = color;
}

#endif /* MATERIAL */
#ifdef OUTLINE

layout(location = 2) uniform int thickness;
layout(location = 3) uniform vec2 viewport;
layout(binding = 1) uniform sampler2D silhouette;

void drawSilhouette()
{
	fragmentColor = vec4(1.0, 0.0, 0.0, 1.0);
}

void drawOutline()
{
	float dx = 1.0 / viewport.x;
	float dy = 1.0 / viewport.y;
	float x = gl_FragCoord.x  / viewport.x;
	float y = gl_FragCoord.y  / viewport.y;
	float adjacentY = y - dy * thickness;
	float adjacentX = 0.0;
	float result = 0.0;
	vec4 color = texture(silhouette, vec2(x, y));

	for (int w = 0; w <= 2 * thickness; w++) {
		adjacentX = x - dx * thickness;
		for (int h = 0; h <= 2 * thickness; h++) {
			vec2 texCoord = vec2(adjacentX, adjacentY);
			vec4 silhouette = texture(silhouette, texCoord);
			if (result < silhouette.r)
				result = silhouette.r;
			adjacentX += dx;
		}
		adjacentY += dy;
	}

	if (color.r == 0.0)
		fragmentColor = vec4(0.102f, 0.212f, 0.6f, result);
	else if (color.r > 0.0)
		fragmentColor = vec4(0.102f, 0.212f, 0.6f, 1.0 - color.r);
	else
		fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
}

void main()
{
	if (thickness > 0)
		drawOutline();
	else
		drawSilhouette();
}

#endif /* OUTLINE */
