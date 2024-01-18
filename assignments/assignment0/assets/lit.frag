#version 450

out vec4 FragColor;

in Surface{
	vec3 Normal;
	vec2 TexCoord;
}fs_in;

uniform sampler2D _MainTex;

void main()
{
FragColor = texture(_MainTex, fs_in.TexCoord);
}