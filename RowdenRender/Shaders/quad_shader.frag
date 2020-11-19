#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D quad_texture;
uniform sampler2D wifi_colors;
uniform sampler2D texture_diffuse1;
uniform int num_routers;

void main()
{
	vec4 color_plus_mask = texture(texture_diffuse1, TexCoord).rgba;
	vec3 color = color_plus_mask.rgb;
	if(color_plus_mask.a < .1)
		discard;
	float mask = texture(quad_texture, TexCoord).r;
	if (mask > .1){
		color = texture(wifi_colors, vec2(0, int((1 - TexCoord.y) * num_routers)/float(num_routers + 1))).rgb;
	}

	FragColor = vec4(color, 1);
}