#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D volume;

uniform vec3 viewPos;
uniform vec2 IsoValRange;
uniform float StepSize;

void main(){
	vec3 view_dir = FragPos - viewPos;
	FragColor = vec4(texture(volume, TexCoord).r, 0, 0, 1);
}