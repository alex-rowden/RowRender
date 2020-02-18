#version 420 core

layout(location=0)out vec4 fhp;
layout(location=1)out vec4 bhp;

in vec3 FragPos;
//out vec4 FragColor;
uniform int front;
uniform vec3 box_min;
uniform vec3 volume_scale;

void main() {
	//vec3 shifted = FragPos - box_min;
	if (front > 0) {
		fhp = vec4((FragPos-box_min) / 50.0f, 1.0);
	}
	else {
		bhp = vec4((FragPos-box_min) / 50.0f, 1.0);
	}

}