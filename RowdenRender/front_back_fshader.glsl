#version 330 core

layout(location = 0) out vec3 fhp;
layout(location = 1) out vec3 bhp;

in vec3 FragPos;

uniform bool front;

void main() {
	if (front) {
		fhp = FragPos;
	}
	else {
		bhp = FragPos;
	}

}