#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vtexCoord;

uniform mat4 _Model;
uniform mat4 _ViewProjection;
uniform mat4 _LightViewProj; //view + projection of light source camera

out vec4 LightSpacePos; //Sent to fragment shader


out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	vec4 LightSpacePos;
}vs_out;

void main()
{
	vs_out.LightSpacePos = _LightViewProj * _Model * vec4(vPos,1);
	vs_out.WorldPos = vec3(_Model * vec4(vPos,1.0));
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vtexCoord;
	gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0);
}