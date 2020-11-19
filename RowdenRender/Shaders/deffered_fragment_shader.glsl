#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

layout(location = 0) out vec4 normal_tex;
layout(location = 1) out vec4 albedo_tex;
layout(location = 2) out vec4 fragPos_tex;

void main()
{
	normal_tex = vec4(normalize(Normal) + vec3(1), 2) / 2.0f;
	albedo_tex = texture(texture_diffuse1, TexCoord);
	fragPos_tex = vec4(FragPos, 1);
	//ssao_tex.ra = vec2(1);
}