#version 430 core

out vec4 fragmentColor;

#ifdef SOLID

in vec3 vertexPosition;
in vec3 vertexNormal;
layout(location = 1) uniform vec3 cameraPosition;

void main()
{
	vec3 ambient = vec3(0.2, 0.2, 0.2);
	vec3 albedo = vec3(0.6, 0.6, 0.6);
	vec3 normal = cross(dFdx(vertexPosition), dFdy(vertexPosition));
	normal = normalize(normal);
	vec3 lightDirection = normalize(cameraPosition - vertexPosition);
	float angle = abs(dot(lightDirection, normal));
	fragmentColor = vec4(ambient + angle * albedo, 1.0);
}

#endif
#ifdef WIREFRAME

in vec4 vertexColor;

void main()
{
	fragmentColor = vertexColor;
}

#endif
#ifdef MATERIAL

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexUV;
in mat3 tbn;
layout(location = 1) uniform vec3 cameraPosition;
layout(location = 2) uniform vec3 ambient;
layout(location = 3) uniform float shininess;
layout(binding = 0) uniform sampler2D albedoMap;
layout(binding = 1) uniform sampler2D opacityMap;
layout(binding = 2) uniform sampler2D specularMap;
layout(binding = 3) uniform sampler2D normalMap;

void main()
{
	vec3 normal = texture(normalMap, vertexUV).rgb * 2.0 - 1.0;
	normal = normalize(tbn * normal);
	vec3 lightDirection = normalize(cameraPosition - vertexPosition);
	float diffuseAngle = abs(dot(lightDirection, normal));

	vec3 reflection = reflect(-lightDirection, normal);
	float specular = max(dot(lightDirection, reflection), 0.0);
	specular = pow(specular, shininess);

	vec4 color = vec4(ambient, 1.0);
	color += texture(albedoMap, vertexUV) * diffuseAngle;
	color += texture(specularMap, vertexUV) * specular;

	vec4 opacity = texture(opacityMap, vertexUV);
	color.a = opacity.r;
	if (color.a < 0.5)
		discard;
	fragmentColor = color;
}

#endif
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
		fragmentColor = vec4(0.102, 0.212, 0.6, result);
	else if (color.r > 0.0)
		fragmentColor = vec4(0.102, 0.212, 0.6, 1.0f - color.r);
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

#endif
