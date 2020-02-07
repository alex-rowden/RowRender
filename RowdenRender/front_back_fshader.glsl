#version 420 core

layout(location = 0) out vec3 fhp;
layout(location = 1) out vec3 bhp;

in vec3 FragPos;
//out vec4 FragColor;
uniform bool front;

void main() {
	fhp = vec3(1, 0, 0);
	if (true) {
		//fhp = vec3(1,0,0);
	}
	else {
		//bhp = FragPos;
	}

}