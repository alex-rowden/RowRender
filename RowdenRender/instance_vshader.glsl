#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 3) in mat4 instanceMatrix;

out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;

void main()
{
	gl_Position = projection * view * transform * instanceMatrix * vec4(aPos, 1.0);
	TexCoord = aTexCoords;
}