#version 420 core

layout(location=0)out vec4 fhp;

in vec3 FragPos;
//out vec4 FragColor;


void main() {
	//vec3 shifted = FragPos - box_min;
	fhp = vec4((FragPos), 1.0);
	

}