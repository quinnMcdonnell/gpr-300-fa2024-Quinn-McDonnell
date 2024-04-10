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

uniform sampler2D _ShadowMap;
uniform Material _Material;

uniform mat4 _LightViewProj;
uniform vec3 _EyePos;

//Directional Light
vec3 _LightDirection = vec3(0.0,-1.0,0.0);
vec3 _LightColor = vec3(1.0);

//vec3 _AmbientColor = vec3(0.3,0.4,0.46);
vec3 _AmbientColor = vec3(1.0,1.0,1.0);


float ShadowCalculation(vec4 fragPosLightSpace)
{
    //perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    //transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    //get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(_ShadowMap, projCoords.xy).r; 

    //get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    //check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}


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
//	vec3 position = texture(gPosition, TexCoord).xyz;
//	vec3 normal = texture(gNormal, TexCoord).xyz;
//	vec3 albedo = texture(gAlbedo, TexCoord).rgb;
//
//	vec3 totalLight = vec3(0);
//	totalLight += calcDirectionalLight(position, normal, albedo);
//

//	float shadow = ShadowCalculation(_LightViewProj * vec4(position,1));
//	FragColor = vec4(albedo * totalLight + (1.0 - shadow), 0.0);
	
	vec3 position = texture(gPosition, TexCoord).xyz;
	vec3 normal = texture(gNormal, TexCoord).xyz;
	vec3 albedo = texture(gAlbedo, TexCoord).rgb;

	vec3 lighting = _AmbientColor;
	lighting += calcDirectionalLight(position, normal, albedo) * ShadowCalculation(_LightViewProj * vec4(position,1));
	for(int i=0;i<MAX_POINT_LIGHTS;i++){
		lighting += calculatePointLight(_PointLights[i],position,normal,albedo);
	}

	FragColor = vec4(albedo * lighting, 1.0);
}
