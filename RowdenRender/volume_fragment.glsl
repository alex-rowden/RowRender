#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D place_holder;
uniform sampler2D volume;

uniform vec3 viewPos;
uniform vec2 IsoValRange;
uniform float StepSize;

void main(){
	FragColor = vec4(FragPos.x / 50.0f, FragPos.y / 50.0f, FragPos.z / 50.0f, 1);
}