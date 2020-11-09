#version 420 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 Tangent;



uniform sampler2D texture_diffuse1;
uniform sampler2D wifi_colors;

layout(location = 0) out vec4 normal_tex;
layout(location = 1) out vec4 albedo_tex;
layout(location = 2) out vec4 fragPos_tex;
layout(location = 3) out vec4 ellipsoid_coordinates_tex;
layout(location = 4) out vec4 tangent_tex;

uniform float distance_mask;


uniform vec3 viewPos;


void main()
{
	vec3 fragPos = FragPos;
	vec3 norm = normalize(Normal);
	tangent_tex = vec4(normalize(Tangent), 1);
	if (distance(FragPos, viewPos) < distance_mask) {
		vec3 bitangent = cross(norm, tangent_tex.rgb);
		norm = normalize(viewPos - fragPos);
		fragPos = -norm * distance_mask + viewPos;
		
		tangent_tex = vec4(cross(norm, bitangent), 1);
	}
	

	ellipsoid_coordinates_tex = vec4(TexCoord, 0, 1);

	albedo_tex = vec4(texture(texture_diffuse1, TexCoord).rgb, 1);
	fragPos_tex = vec4(fragPos, 1);
	normal_tex = vec4(norm, 1);
	
}