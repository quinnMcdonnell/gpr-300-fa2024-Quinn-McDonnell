#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vtexCoord;

uniform mat4 _Model;
uniform mat4 _ViewProjection;
uniform mat4 _LightViewProj;

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}vs_out;

out vec4 LightSpacePos;

void main()
{
	gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0);
}