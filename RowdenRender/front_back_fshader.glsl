#version 420 core

out vec4 fhp;
//out vec3 bhp;

in vec3 FragPos;
//out vec4 FragColor;
uniform bool front;

void main() {
	fhp = vec4(1, 0, 0, 1.0);
	if (true) {
		//fhp = vec3(1,0,0);
	}
	else {
		//bhp = FragPos;
	}

}