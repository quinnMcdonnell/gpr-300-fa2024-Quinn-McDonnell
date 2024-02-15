#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoords;

uniform mat4 _Model;
uniform mat4 _ViewProjection;
uniform float time;
uniform float wave_scale;

out vec2 TexCoords;

float calculateSurface(float x,float z)
{
	float y = 0.0;
	y += (sin(x * 0.5 / wave_scale + time * 0.15) + sin(x * 0.4 / wave_scale + time * 0.4) + sin(x * 0.4 / wave_scale + time * 0.2)) / 3.0;
	y += (sin(z * 0.5 / wave_scale + time * 0.3) + sin(z * 0.4 / wave_scale + time * 0.05) + sin(z * 0.4 / wave_scale + time * 0.1)) /3.0;
	return y;
}

void main()
{
	vec3 pos = vPos;
	pos.y += calculateSurface(pos.x, pos.z) * 1;
	pos.y += calculateSurface(0.0,0.0) * 1;

	TexCoords = vTexCoords;
	gl_Position = _ViewProjection * _Model * vec4(pos, 1.0);
}