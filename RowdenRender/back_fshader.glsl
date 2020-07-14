#version 420 core

layout(location=1)out vec4 bhp;

in vec3 FragPos;
//out vec4 FragColor;
uniform int front;


void main() {
	//vec3 shifted = FragPos - box_min;
	
	bhp = vec4((FragPos), 1.0);
	

}