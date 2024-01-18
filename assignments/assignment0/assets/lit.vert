#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vtexCoord;

uniform mat4 _Model;
uniform mat4 _ViewProjection;

out Surface{
	vec3 Normal;
	vec2 TexCoord;
}vs_out;

void main()
{
	vs_out.Normal = vNormal;
	vs_out.TexCoord = vtexCoord;
	gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0);
}