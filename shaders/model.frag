#version 430 core

out vec4 fragmentColor;

#ifdef SOLID

in vec3 intPosition;
layout(location = 1) uniform vec4 cameraPosition;

void main()
{
	vec3 ambientColor = vec3(0.2, 0.2, 0.2);
	vec3 diffuseColor = vec3(0.5, 0.5, 0.5);
	vec3 specColor = vec3(0.5, 0.5, 0.5);

	vec3 normal = normalize(cross(dFdx(intPosition), dFdy(intPosition)));
	vec3 lightDir = normalize(cameraPosition.xyz - intPosition);
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
uniform sampler2D tex;

void main()
{
	fragmentColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	if (thickness > 0) {
		float dx = 1.0 / viewport.x;
		float dy = 1.0 / viewport.y;
		float x = gl_FragCoord.x * dx;
		float y = gl_FragCoord.y * dy;
		float yy = y - dy * thickness;
		float v = 0;

		for (int w = 0; w <= 2 * thickness; w++) {
			float xx = x - dx * thickness;
			for (int h = 0; h <= 2 * thickness; h++) {
				float a = 1 - texture(tex, vec2(xx, yy)).r;
				if (a > v)
					v = a;
				xx += dx;
			}
			yy += dy;
		}

		if (v != 0)
			fragmentColor = vec4(0.1, 0.1, 0.1, 1.0);
		else
			discard;
	} else
		fragmentColor = vec4(1.0, 1.0, 1.0, 1.0);
}

#endif /* OUTLINE */
