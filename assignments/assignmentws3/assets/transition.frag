#version 450

out vec4 FragColor;

uniform sampler2D gradient;
uniform vec3 color; //background color

uniform float cutoff;

in vec2 TexCoords;

void main()
{
	float alpha = texture(gradient, TexCoords).r;
	if(alpha >= cutoff)
	{
		discard;
	}

	FragColor = vec4(color,1.0);
}