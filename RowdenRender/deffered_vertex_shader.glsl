#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 camera;
uniform mat4 projection;

void main() {
	
	vec4 fragPos4 = (camera * model * vec4(aPos, 1));
	FragPos = fragPos4.xyz/fragPos4.w;
	gl_Position = projection * fragPos4;
	TexCoord = aTexCoord;
	Normal = normalMatrix * aNormal;
}