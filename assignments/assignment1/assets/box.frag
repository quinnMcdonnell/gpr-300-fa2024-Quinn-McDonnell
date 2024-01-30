#version 450

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float DivideValue;

const float offset = 1.0/300;

void main()
{
	vec2 offsets[9] = vec2[](

	vec2(-offset, offset), vec2(0.0f, offset), vec2(offset, offset),
	vec2(-offset, 0.0f),   vec2(0.0f, 0.0f),   vec2(offset, 0.0f),
	vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)
	);

	float divider = DivideValue;

	float kernel[9] = float[](
	1.0/divider, 1.0/divider, 1.0/divider,
	1.0/divider, 1.0/divider, 1.0/divider,
	1.0/divider, 1.0/divider, 1.0/divider
	);
	
	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
	}

	vec3 col = vec3(0.0);
	for(int i = 0; i < 9; i++)
	{
		col += sampleTex[i] * kernel[i];
	}

	FragColor = vec4(col, 1.0);
}