#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in float tangentScale;
layout(location = 4) in vec2 uv;
layout(location = 5) in vec2 indices;
layout(location = 6) in vec2 weights;
layout(location = 0) uniform mat4 vp;

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

#endif
#ifdef SOLID

layout(location = 2) uniform mat4 view;
layout(location = 3) uniform mat4 projection;

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
	gl_Position = vp * tp;
}

#endif
#ifdef WIREFRAME

out vec4 vertexColor;

void main()
{
	vec4 tp = vec4(position, 1.0);
#ifdef DYNAMIC
	tp = getAnimatedPoint(tp);
#endif
	vertexColor = vec4(0.13, 0.13, 0.13, 1.0);
	gl_Position = vp * tp;
}

#endif
#ifdef MATERIAL

out vec3 vertexPosition;
out vec3 vertexNormal;
out vec2 vertexUV;
out mat3 tbn;

void main()
{
	vec4 tp = vec4(position, 1.0);
	vec3 tn = normal;
	vec3 tt = tangent;
#ifdef DYNAMIC
	tp = getAnimatedPoint(tp);
	tn = getAnimatedPoint(vec4(tn, 0.0)).xyz;
	tt = getAnimatedPoint(vec4(tt, 0.0)).xyz;
#endif
	gl_Position = vp * tp;
	vertexPosition = tp.xyz;
	vertexNormal = tn;
	vertexUV = uv;
	vec3 bitangent = cross(tt, tn);
	tbn = mat3(tt, bitangent, tn);
}

#endif
#ifdef OUTLINE

layout(location = 2) uniform int thickness;

void main()
{
	vec4 tp = vec4(position, 1.0);
#ifdef DYNAMIC
	if (thickness == 0)
		tp = getAnimatedPoint(tp);
#endif
	gl_Position = vp * tp;
}

#endif
