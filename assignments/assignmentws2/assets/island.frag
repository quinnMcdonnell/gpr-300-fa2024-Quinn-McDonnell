#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;

const vec3 Rock = vec3(1.0, 0.85, 0.6);
const vec3 Ocean = vec3(0.0, 0.31, 0.85);

void main()
{
	float height = texture(depthMap, TexCoords).r;
	if(height < 0.1)
		FragColor = vec4(Ocean, 1.0);
	else if(height >= 0.1)
		FragColor = vec4(vec3(height) * Rock, 1.0);
}