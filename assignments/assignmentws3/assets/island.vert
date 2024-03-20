#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoords;

uniform mat4 _Model;
uniform mat4 _ViewProjection;
uniform sampler2D depthMap;
uniform float MaxHeight;

out vec2 TexCoords;

const float offset = 1.0/300;

vec2 offsets[9] = vec2[](

	vec2(-offset, offset), vec2(0.0f, offset), vec2(offset, offset),
	vec2(-offset, 0.0f),   vec2(0.0f, 0.0f),   vec2(offset, 0.0f),
	vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)
	);


	float kernel[9] = float[](
	1.0/9.0, 1.0/9.0, 1.0/9.0,
	1.0/9.0, 1.0/9.0, 1.0/9.0,
	1.0/9.0, 1.0/9.0, 1.0/9.0
	);

void main()
{
	TexCoords = vTexCoords;
	
	float sampleTex[9];
	for(int i = 0; i < 9; i++)
	{
		sampleTex[i] = texture(depthMap, TexCoords + offsets[i]).r;
	}

	float col = 0.0;
	for(int i = 0; i < 9; i++)
	{
		col += sampleTex[i] * kernel[i];
	}
	
	float scalar = col;
	vec3 pos = vPos;
	pos.y += scalar * MaxHeight;
	gl_Position = _ViewProjection * _Model * vec4(pos, 1.0);
}