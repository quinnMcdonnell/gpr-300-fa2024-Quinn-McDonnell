#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D water_texture;
uniform float time;

const vec3 water_color = vec3(0.0, 0.31, 0.85);
const float S1 = 0.90;
const float S2 = 0.02;
const float scale = 10;

const vec2 direction = vec2(-2.0);

void main()
{
	vec2 uv = TexCoords * scale + vec2(time);
	uv.x += (sin(uv.y * 0.5 + time * 0.5) + sin(uv.y * 0.4 + time * 0.3) + sin(uv.y * 0.4 + time * 0.3)) + sin(.0)/3.0;
	uv.y += (sin(uv.x * 1.2 + time * 0.5) + sin(uv.x * 0.1 + time * 1.5)+ sin(uv.x * 0.4 + time * 0.3)) + sin(.0)/3.0;
	
	vec4 smp1 = texture(water_texture, uv * 1.0);
	vec4 smp2 = texture(water_texture, uv * 1.0 - vec2(0.2));
	FragColor = vec4(water_color + vec3(smp1 * S1 - smp2 * S2),1.0);
}