#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform float increment;
uniform int heatmap;


void main()
{
	if(heatmap > 0)
		FragColor = vec4(texture(texture_diffuse1, TexCoord).r - increment, 0, 0, 1);
	FragColor = texture(texture_diffuse1, TexCoord);
}