#version 450

out vec4 FragColor;

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
    vec4 LightSpacePos;
}fs_in;

uniform sampler2D _ShadowMap;
uniform sampler2D _MainTex;

uniform vec3 lightPos;
uniform vec3 viewPos;


struct Material
{
	float Ka;
	float Kd;
	float Ks;
	float Shininess;
};

//uniform Material _Material;

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
     //perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    //transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    //get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(_ShadowMap, projCoords.xy).r; 

    //get depth of current fragment from light's perspective
    float currentDepth = projCoords.z - bias;
    
    //PCR
    float totalShadow = 0;
    vec2 texelOffset = 1.0 /  textureSize(_ShadowMap,0);
    
//    vec2 offsets[9] = vec2[](
//	vec2(-texelOffset, texelOffset), vec2(0.0f, texelOffset), vec2(texelOffset, texelOffset),
//	vec2(-texelOffset, 0.0f),   vec2(0.0f, 0.0f),   vec2(texelOffset, 0.0f),
//	vec2(-texelOffset, -texelOffset), vec2(0.0f, -texelOffset), vec2(texelOffset, -texelOffset)
//	);
//
//    float kernel[9] = float[](
//	1.0/9.0, 1.0/9.0, 1.0/9.0,
//	1.0/9.0, 1.0/9.0, 1.0/9.0,
//	1.0/9.0, 1.0/9.0, 1.0/9.0
//	);

    for(int y = -1; y <= 1; y++)
    {
	    for(int x = -1; x <= 1; x++)
        {
		    vec2 uv = projCoords.xy + vec2(x * texelOffset.x, y * texelOffset.y);
            totalShadow += step(texture(_ShadowMap,uv).r,currentDepth - closestDepth);
        }
    }   
       
    totalShadow /= 9.0;

    return totalShadow;
}


void main()
{
	vec3 color = texture(_MainTex, fs_in.TexCoord).rgb;
    vec3 normal = normalize(fs_in.WorldNormal);
    vec3 lightColor = vec3(1.0);

    vec3 ambient = vec3(0.3, 0.4, 0.6) * lightColor;
    
    //biases
    float minBias = 0.005;
    float maxBias = 100.0;

    vec3 lightDir = normalize(lightPos - fs_in.WorldPos);
    float bias = max(maxBias * (1.0 - dot(normal, lightDir)),minBias);

    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 viewDir = normalize(viewPos - fs_in.WorldPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    

    float shadow = ShadowCalculation(fs_in.LightSpacePos, bias);
    
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}