#version 430 core

layout(location = 0) in vec4 point;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec2 indices;
layout(location = 4) in vec2 weights;
layout(location = 0) uniform mat4 vp;

#ifdef DYNAMIC

struct Joint {
	vec4 rotation;
	vec4 translation1;
	vec4 translation2;
};
layout(std430, binding = 3) buffer Joints {
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

vec4 getAnimatedPoint()
{
	Joint joint;
	vec4 v1 = point;
	vec4 v2 = point;

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

	v1 = weights.x*v1 + weights.y*v2;
	v1.w = 1.0f;
	return v1;
}

#endif /* DYNAMIC */
#ifdef SOLID

out vec3 intPosition;

void main()
{
	vec4 vertex = point;
	#ifdef DYNAMIC
	vertex = getAnimatedPoint();
	#endif /* DYNAMIC */
	gl_Position = vp * vertex;
	intPosition = vec3(vertex) / point.w;
}

#endif /* SOLID */
#ifdef WIREFRAME

layout(location = 1) uniform vec4 color;
out vec4 vertexColor;

void main()
{
	vec4 vertex = point;
	#ifdef DYNAMIC
	vertex = getAnimatedPoint();
	#endif /* DYNAMIC */
	vertexColor = color;
	gl_Position = vp * vertex;
}

#endif /* WIREFRAME */
#ifdef MATERIAL

out vec2 vertTexCoord;

void main()
{
	vec4 vertex = point;
	#ifdef DYNAMIC
	vertex = getAnimatedPoint();
	#endif /* DYNAMIC */
	vertTexCoord = tex;
	gl_Position = vp * vertex;
}

#endif /* MATERIAL */
#ifdef OUTLINE

void main()
{
	vec4 vertex = point;
	#ifdef DYNAMIC
	vertex = getAnimatedPoint();
	#endif /* DYNAMIC */
	gl_Position = vp * vertex;
}


#endif /* OUTLINE */
