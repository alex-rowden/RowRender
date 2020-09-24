#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec3 Tangent;

uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 camera;
uniform mat4 projection;

void main() {
	gl_Position = projection * camera * model * vec4(aPos, 1);
	vec4 fragPos4 = (model * vec4(aPos, 1));
	FragPos = fragPos4.xyz / fragPos4.w;
	TexCoord = aTexCoord;
	Normal = normalMatrix * aNormal;
	Tangent = normalMatrix * aTangent;
}