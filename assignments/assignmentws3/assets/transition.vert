#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = vTexCoords;
	gl_Position = vec4(vPos, 1.0);
}