#version 430 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
layout(location = 1) uniform vec2 viewport;

in VertexData {
	vec4 color;
} vertexIn[2];

out VertexData {
	vec4 color;
} vertexOut;

void main()
{
	float width = 2.0f;
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec3 ndc0 = p0.xyz / p0.w;
	vec3 ndc1 = p1.xyz / p1.w;

	vec2 direction = normalize(ndc1.xy - ndc0.xy);
	vec2 normal = vec2(-direction.y, direction.x);
	vec2 offset = (vec2(width) / viewport) * normal;

	gl_Position = vec4(p0.xy + offset*p0.w, p0.z, p0.w);
	vertexOut.color = vertexIn[1].color;
	EmitVertex();
	gl_Position = vec4(p0.xy - offset*p0.w, p0.z, p0.w);
	vertexOut.color = vertexIn[1].color;
	EmitVertex();
	gl_Position = vec4(p1.xy + offset*p1.w, p1.z, p1.w);
	vertexOut.color = vertexIn[1].color;
	EmitVertex();
	gl_Position = vec4(p1.xy - offset*p1.w, p1.z, p1.w);
	vertexOut.color = vertexIn[1].color;
	EmitVertex();

	EndPrimitive();
}
