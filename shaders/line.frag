#version 430 core

in VertexData {
        vec4 color;
} vertexIn;

out vec4 color;

void main()
{
	color = vertexIn.color;
}
