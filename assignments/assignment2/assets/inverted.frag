#version 450

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main()
{
	vec3 rgb = texture(screenTexture, TexCoords).rgb;
	FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
}