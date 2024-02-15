#version 450

out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 WorldNormal;

struct PBRMaterial{
	sampler2D albedo;
	sampler2D metallic;
	sampler2D roughness;
	sampler2D occulusion;
	sampler2D specular;
};

uniform PBRMaterial material;
uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec3 AmbientLight;

const vec3 lightColor = vec3(1.0);
const float PI = 3.141592;

vec3 col;
float mtl;
float rgh;
float spec;
float ao;

vec3 N = normalize(WorldNormal);
vec3 V = normalize(cameraPos);
vec3 L = normalize(lightPos);
vec3 H = normalize(V + L);

float NdotH;
float NdotV;
float NdotL;
float VdotH;
float VdotL;
float VdotN;
float LdotN;

float epilson = 0.000001;

float D(float alpha)
{
	float k = pow(alpha,2);
	float denominator = PI * pow((pow(NdotH, 2.0) * (pow(alpha,2.0) - 1) + 1.0),2.0);

	return k/denominator;
}

float G1(float alpha, float x)
{
	float k = alpha/2;
	float denominator = x * (1.0 - k) + k;

	return 0.0;
}

float G(float alpha)
{
	return G1(alpha, LdotN) * G1(alpha, VdotN);
}

vec3 F(vec3 F0)
{
	return F0 + (vec3(1.0) - F0)*pow((1 - VdotN),5);
}

vec3 PBR()
{
	vec3 F0 = vec3(0.4);
	F0 = col;
	F0 = vec3(mtl);

	//Conservation of energy (glowing)

	vec3 Ks = vec3(F0);
	vec3 Kd = (vec3(1.0) -Ks) * (1.0 - mtl);

	//BDRF Diffusion
	vec3 lambert = col / PI;
	
	//BDRF Specular
	float alpha = pow(rgh, 2.0);

	vec3 cookTorrenceNumerator = D(alpha) * G(alpha) * Ks;
	float cookTorrenceDenominator = max(4.0 * VdotH * LdotN, epilson);
	vec3 cookTorrence = cookTorrenceNumerator / cookTorrenceDenominator;

	vec3 BDRF = (Kd * lambert) + cookTorrence;


	return BDRF * lightColor * LdotN;
}

void main()
{
	//pre-sample
	vec3 col = texture(material.albedo, TexCoords).rgb;
	float mtl = texture(material.metallic, TexCoords).r;
	float rgh = texture(material.roughness, TexCoords).r;
	float spec = texture(material.specular, TexCoords).r;
	float ao = texture(material.occulusion, TexCoords).r;

	//Pre-compute
	NdotH = max(dot(N,H),0.0);
	NdotV = max(dot(N,V),0.0);
	NdotL = max(dot(N,L),0.0);
	VdotH = max(dot(V,H),0.0);
	VdotN = max(dot(V,N),0.0);
	LdotN = max(dot(L,N),0.0);
	VdotL = max(dot(V,L),0.0);

	//stylization
	vec3 reflectionDir = reflect(-L, N);
	float specAmount = pow(max(dot(V,reflectionDir), 0.0),32.0);
	vec3 specular = spec * specAmount * lightColor;

	vec3 finalColor = (AmbientLight * col * ao) + PBR() + specular;
	FragColor = vec4(finalColor, 1.0);

}

