#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoords;

uniform mat4 _Model;
uniform mat4 _ViewProjection;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 WorldNormal;

void main()
{
	WorldPos = vPos;
	WorldNormal = vNormal;
	TexCoords = vTexCoords;

	gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0);
}