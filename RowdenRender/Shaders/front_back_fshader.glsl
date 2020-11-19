#version 420 core

layout(location=0)out vec4 fhp;
layout(location=1)out vec4 bhp;

in vec3 FragPos;
//out vec4 FragColor;
uniform int front;


void main() {
	//vec3 shifted = FragPos - box_min;
	if (front == 1) {
		fhp = vec4((FragPos), 1.0);
	}
	else if(front == -1){
		bhp = vec4((FragPos), 1.0);
	}

}