#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec4 ourColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 camera;

void main() {
	gl_Position = camera * model * vec4(aPos, 1);
	ourColor = aColor;
	TexCoord = aTexCoord;
}