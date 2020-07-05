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

#endif
#ifdef WIREFRAME

in vec4 vertexColor;

void main()
{
	fragmentColor = vertexColor;
}

#endif
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

#endif
