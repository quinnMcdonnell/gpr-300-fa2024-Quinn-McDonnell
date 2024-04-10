#version 450

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vtexCoord;

uniform mat4 _LightViewProj; //view + projection of light source camera

out vec4 LightSpacePos; //Sent to fragment shader
out vec2 TexCoord;

void main()
{
	TexCoord = vtexCoord;
	gl_Position = vec4(vPosition,1.0);
}