#version 450

layout(location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D _gWorldPos;
uniform sampler2D _gWorldNormal;
uniform sampler2D _gAlbedo;

struct Light{
	vec3 Position;
	vec3 Color;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main(){

	vec3 worldPos = texture(_gWorldPos,TexCoord).rgb;	
	vec3 worldNormal = texture(_gWorldNormal,TexCoord).rgb;
	vec3 albedo = texture(_gAlbedo,TexCoord).rgb;
	float specular = texture(_gAlbedo,TexCoord).a;

	vec3 lighting = albedo * 0.1;
	vec3 viewDir = normalize(viewPos - worldPos);
	for(int i = 0; i < NR_LIGHTS; i++)
	{
		vec3 lightDir = normalize(lights[i].Position - worldPos);
		vec3 diffuse = max(dot(worldNormal, lightDir),0.0) * albedo * lights[i].Color;
		lighting += diffuse;
	}

	FragColor = vec4(lighting,1.0);
}
