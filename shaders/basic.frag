#version 430 core

in vec3 intPosition;
layout(location = 1) uniform vec4 cameraPosition;

const vec3 ambientColor = vec3(0.2, 0.2, 0.2);
const vec3 diffuseColor = vec3(0.5, 0.5, 0.5);
const vec3 specColor = vec3(0.5, 0.5, 0.5);

out vec4 finalColor;

void main()
{
	vec3 normal = normalize(cross(dFdx(intPosition), dFdy(intPosition)));
	vec3 lightDir = normalize(cameraPosition.xyz - intPosition);
	float lambertian = max(dot(lightDir, normal), 0.1);
	float specular = 0.0;

	if(lambertian > 0.0) {
		vec3 viewDir = normalize(-cameraPosition.xyz);
		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(halfDir, normal), 0);
		specular = pow(specAngle, 16.0);
	}

	finalColor = vec4(ambientColor + lambertian * diffuseColor + specular * specColor, 1.0);
}
