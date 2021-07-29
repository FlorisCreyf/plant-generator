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

#elif defined(WIREFRAME)

in vec4 vertexColor;

void main()
{
	fragmentColor = vertexColor;
}

#elif defined(SHADOW)

layout(binding = 1) uniform sampler2D opacityMap;
in vec2 vertexUV;

void main()
{
	if (texture(opacityMap, vertexUV).r < 0.5)
		discard;
}

#elif defined(MATERIAL)

layout(location = 1) uniform vec3 cameraPosition;
layout(location = 2) uniform vec3 ambient;
layout(location = 3) uniform float shininess;
layout(location = 5) uniform vec3 lightDirection;
layout(location = 6) uniform bool enableShadows;
layout(binding = 0) uniform sampler2D albedoMap;
layout(binding = 1) uniform sampler2D opacityMap;
layout(binding = 2) uniform sampler2D specularMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D shadowMap;
in vec4 lightFragment;
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexUV;
in mat3 surfaceBasis;

float getIlluminance(vec4 position)
{
	vec3 point = (position.xyz / position.w + 1.0) * 0.5;
	if (texture(shadowMap, point.xy).r == 0.0)
		return 1.0;

	float illuminance = 0.0;
	float angle = 1.0f - abs(dot(vertexNormal, lightDirection));
	float bias = max(0.005 * angle, 0.0005);
	vec2 size = 1.0 / textureSize(shadowMap, 0);
	for (float x = -1.5; x <= 1.5; x += 1.0) {
		for (float y = -1.5; y <= 1.5; y += 1.0) {
			vec2 c = point.xy + size * vec2(x, y);
			float d = texture(shadowMap, c).r;
			illuminance += point.z + bias <= d ? 0.0 : 1.0;

		}
	}
	return illuminance / 16.0;
}

void main()
{
	vec3 normal = texture(normalMap, vertexUV).rgb * 2.0 - 1.0;
	normal = normalize(surfaceBasis * normal);
	/* The normals are flipped based on the camera direction in order to
	cope with double sided leaf surfaces */
	vec3 cameraDirection = normalize(cameraPosition - vertexPosition);
	if (dot(cameraDirection, vertexNormal) > 0.0)
		normal = -normal;

	float diffuse = max(dot(lightDirection, normal), 0.0) * 0.5 + 0.5;
	vec3 reflection = reflect(-lightDirection, normal);
	float specular = max(dot(lightDirection, reflection), 0.0);
	specular = pow(specular, shininess);

	vec4 color = texture(albedoMap, vertexUV) * diffuse;
	color += texture(specularMap, vertexUV) * specular;
	if (enableShadows) {
		float l = getIlluminance(lightFragment);
		color = (0.5 + 0.5 * l) * color + vec4(ambient, 1.0);
	} else
		color += vec4(ambient, 1.0);

	vec4 opacity = texture(opacityMap, vertexUV);
	color.a = opacity.r;
	if (color.a < 0.5)
		discard;
	else
		fragmentColor = color;
}

#elif defined(OUTLINE)

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
	float x = gl_FragCoord.x / viewport.x;
	float y = gl_FragCoord.y / viewport.y;
	float adjacentY = y - dy * thickness;
	float adjacentX = 0.0;
	float result = 0.0;
	vec4 color = texture(silhouette, vec2(x, y));

	for (int w = 0; w <= 2 * thickness; w++) {
		adjacentX = x - dx * thickness;
		for (int h = 0; h <= 2 * thickness; h++) {
			vec2 texCoord = vec2(adjacentX, adjacentY);
			vec4 silhouette = texture(silhouette, texCoord);
			adjacentX += dx;
			if (result < silhouette.r)
				result = silhouette.r;
		}
		adjacentY += dy;
	}

	if (color.r == 0.0)
		fragmentColor = vec4(0.102, 0.212, 0.6, result);
	else if (color.r > 0.0)
		fragmentColor = vec4(0.102, 0.212, 0.6, 1.0 - color.r);
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
