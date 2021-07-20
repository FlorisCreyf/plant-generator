#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in float tangentScale;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec2 indices;
layout(location = 6) in vec2 weights;
layout(location = 0) uniform mat4 transform;

#ifdef DYNAMIC

struct Joint {
	vec4 rotation;
	vec4 translation1;
	vec4 translation2;
};

layout(std430, binding = 5) buffer Joints {
	Joint joints[];
};

vec4 multQuat(vec4 a, vec4 b)
{
	vec3 m = a.xyz;
	vec3 n = b.xyz;
	float s = a.w * b.w - dot(m, n);
	vec3 f = (cross(m, n) + b.w * m) + a.w * n;
	return vec4(f, s);
}

vec4 conjugateQuat(vec4 q)
{
	return vec4(-q.x, -q.y, -q.z, q.w);
}

vec4 getAnimatedPoint(vec4 v)
{
	Joint joint;
	vec4 v1 = v;
	vec4 v2 = v;
	joint = joints[int(indices.x)];
	v1 -= joint.translation1;
	v1 = multQuat(joint.rotation, v1);
	v1 = multQuat(v1, conjugateQuat(joint.rotation));
	v1 += joint.translation2;
	joint = joints[int(indices.y)];
	v2 -= joint.translation1;
	v2 = multQuat(joint.rotation, v2);
	v2 = multQuat(v2, conjugateQuat(joint.rotation));
	v2 += joint.translation2;
	return weights.x*v1 + weights.y*v2;
}

vec4 getAnimatedNormal(vec4 v)
{
	Joint joint;
	vec4 v1 = v;
	vec4 v2 = v;
	joint = joints[int(indices.x)];
	v1 = multQuat(joint.rotation, v1);
	v1 = multQuat(v1, conjugateQuat(joint.rotation));
	joint = joints[int(indices.y)];
	v2 = multQuat(joint.rotation, v2);
	v2 = multQuat(v2, conjugateQuat(joint.rotation));
	return weights.x*v1 + weights.y*v2;
}

#endif
#ifdef SOLID

out vec3 vertexPosition;
out vec3 vertexNormal;

void main()
{
	vec4 tp = vec4(position, 1.0);
#ifdef DYNAMIC
	tp = getAnimatedPoint(tp);
#endif
	vertexPosition = tp.xyz;
	vertexNormal = normal;
	gl_Position = transform * tp;
}

#elif defined(WIREFRAME)

out vec4 vertexColor;

void main()
{
	vec4 position2 = vec4(position, 1.0);
#ifdef DYNAMIC
	position2 = getAnimatedPoint(position2);
#endif
	vertexColor = vec4(0.13, 0.13, 0.13, 1.0);
	gl_Position = transform * position2;
}

#elif defined(SHADOW)

out vec2 vertexUV;

void main()
{
	vec4 position2 = vec4(position, 1.0);
#ifdef DYNAMIC
	position2 = getAnimatedPoint(position2);
#endif
	gl_Position = transform * position2;
	vertexUV = uv;
}

#elif defined(MATERIAL)

layout(location = 4) uniform mat4 lightTransform;
out vec4 lightFragment;
out vec3 vertexPosition;
out vec3 vertexNormal;
out vec2 vertexUV;
out mat3 surfaceBasis;

void main()
{
	vec4 position2 = vec4(position, 1.0);
	vec3 normal2 = normal;
	vec3 tangent2 = tangent;
#ifdef DYNAMIC
	position2 = getAnimatedPoint(position2);
	normal2 = getAnimatedNormal(vec4(normal2, 0.0)).xyz;
	tangent2 = getAnimatedNormal(vec4(tangent2, 0.0)).xyz;
#endif
	gl_Position = transform * position2;
	lightFragment = lightTransform * position2;
	vertexPosition = position2.xyz;
	vertexNormal = normal2;
	vertexUV = uv;
	surfaceBasis = mat3(tangent2, cross(tangent2, normal2), normal2);
}

#elif defined(OUTLINE)

layout(location = 2) uniform int thickness;

void main()
{
	vec4 position2 = vec4(position, 1.0);
#ifdef DYNAMIC
	if (thickness == 0)
		position2 = getAnimatedPoint(position2);
#endif
	gl_Position = transform * position2;
}

#endif
