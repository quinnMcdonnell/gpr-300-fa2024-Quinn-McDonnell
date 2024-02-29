#version 450
out vec4 FragColor;

in vec2 TexCoord;

uniform layout(location = 0) sampler2D gPosition;
uniform layout(location = 1) sampler2D gNormal;
uniform layout(location = 2) sampler2D gAlbedo;

struct PointLight{
	vec3 position;
	float radius;
	vec3 color;
};
const int MAX_POINT_LIGHTS = 64;
uniform PointLight _PointLights[MAX_POINT_LIGHTS];

struct Material
{
	float Ka;
	float Kd;
	float Ks;
	float Shininess;
};

uniform Material _Material;
uniform vec3 _EyePos;
vec3 _LightDirection = vec3(0.0,-1.0,0.0);
vec3 _LightColor = vec3(1.0);
vec3 _AmbientColor = vec3(0.3,0.4,0.46);


//Exponential falloff
float attenuateExponential(float distance, float radius){
	float i = clamp(1.0 - pow(distance/radius,4.0),0.0,1.0);
	return i * i;
	
}

vec3 calcDirectionalLight(vec3 position, vec3 normal, vec3 albedo)
{
	vec3 toLight = -_LightDirection;
	
	float diffuseFactor = max(dot(normal,toLight),0.0);

	vec3 toEye = normalize(_EyePos - position);
	vec3 h = normalize(toLight + toEye);

	float SpecularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
	
	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * SpecularFactor) * _LightColor;
	
	return lightColor;
}

vec3 calculatePointLight(PointLight light, vec3 position, vec3 normal, vec3 albedo)
{
	vec3 diff = light.position - position;

	vec3 toLight = normalize(diff);
	
	float diffuseFactor = max(dot(normal,toLight),0.0);
	
	vec3 toEye = normalize(_EyePos - position);
	vec3 h = normalize(toLight + toEye);

	float SpecularFactor = pow(max(dot(normal,h),0.0),1.0);

	vec3 lightColor = (diffuseFactor + SpecularFactor) * light.color;
	float d = length(diff);

	lightColor *= attenuateExponential(d,light.radius);

	return lightColor;
}

void main()
{
	vec3 position = texture(gPosition, TexCoord).xyz;
	vec3 normal = texture(gNormal, TexCoord).xyz;
	vec3 albedo = texture(gAlbedo, TexCoord).rgb;

	vec3 totalLight = vec3(0);
	totalLight += calcDirectionalLight(position, normal, albedo);

	for(int i=0;i<MAX_POINT_LIGHTS;i++){
		totalLight+=calculatePointLight(_PointLights[i],position,normal,albedo);
	}

	FragColor = vec4(albedo * totalLight, 0.0);
	
}
