#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_diffuse4;
uniform sampler2D texture_diffuse5;
uniform sampler2D texture_diffuse6;
uniform sampler2D texture_diffuse7;
uniform sampler2D texture_diffuse8;
uniform sampler2D texture_diffuse9;
uniform sampler2D texture_diffuse10;
uniform sampler2D texture_diffuse11;
uniform sampler2D texture_diffuse12;
uniform sampler2D texture_diffuse13;
uniform sampler2D texture_diffuse14;
uniform sampler2D texture_diffuse15;

uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_specular3;
uniform sampler2D texture_specular4;
uniform sampler2D texture_specular5;
uniform sampler2D texture_specular6;
uniform sampler2D texture_specular7;
uniform sampler2D texture_specular8;
uniform sampler2D texture_specular9;
uniform sampler2D texture_specular10;

uniform vec3 lightColor;

void main()
{
	FragColor = texture(texture_diffuse1, TexCoord) * vec4(lightColor, 1);
}