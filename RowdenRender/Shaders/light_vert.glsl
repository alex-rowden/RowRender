#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 camera;

void main()
{
	gl_Position = camera * model * vec4(aPos, 1);
}