#version 450

layout(location = 0) in vec2 vtexCoord;

out vec2 TexCoord;

void main()
{
	TexCoord = vtexCoord;
}