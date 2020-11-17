#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D text_tex;
uniform float increment;
uniform bool heatmap;


void main()
{
	if(heatmap)
		FragColor = vec4(texture(texture_diffuse1, TexCoord).r + increment, 0, 0, 1);
	else {
		float text = texture(text_tex, TexCoord).r;
		FragColor = texture(texture_diffuse1, TexCoord).rgba;
		if (false) {
			FragColor = vec4(0, 0, 0, 1);
		}
	}
}