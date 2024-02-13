#version 450

out vec4 FragColor;

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_in;

in vec4 LightSpacePos;
uniform sampler2D _ShadowMap;
uniform sampler2D _MainTex;
uniform vec3 _EyePos;
uniform vec3 _LightDirection;
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);


struct Material
{
	float Ka;
	float Kd;
	float Ks;
	float Shininess;
};

uniform Material _Material;

float calcShadow(sampler2D shadowMap, vec4 lightSpacePos){
	
	//Homogeneous Clip space to NDC [-w,w] to [-1,1]
    vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
    
	//Convert from [-1,1] to [0,1]
    sampleCoord = sampleCoord * 0.5 + 0.5;
	
	float myDepth = sampleCoord.z; 
	
	float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;
	
	//step(a,b) returns 1.0 if a >= b, 0.0 otherwise
	return step(shadowMapDepth,myDepth);
}


void main()
{
	vec3 normal = normalize(fs_in.WorldNormal);
	vec3 toLight = -normalize(_LightDirection);

	float shadow = calcShadow(_ShadowMap, LightSpacePos);
	
	float diffuseFactor = max(dot(normal,toLight),0.0);

	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	vec3 h = normalize(toLight + toEye);

	float SpecularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
	
	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * SpecularFactor) * _LightColor;
	lightColor += _AmbientColor * _Material.Ka * (1.0 - shadow);

	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor, 1.0);
}