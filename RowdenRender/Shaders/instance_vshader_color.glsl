#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in mat4 instanceMatrix;
layout(location = 7) in float colorInd;

out vec2 TexCoord;
out vec3 normal;
out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;
uniform mat4 model_transform;
uniform mat3 normalMat;

void main()
{
	gl_Position = projection * view * transform * instanceMatrix * model_transform * vec4(aPos, 1.0);
	vec4 fragPos4 = (transform * instanceMatrix * model_transform * vec4(aPos, 1));
	FragPos = fragPos4.xyz / fragPos4.w;
	normal = normalMat * aNormal;
	TexCoord = vec2(0, colorInd);
	//colorIndex = colorInd;
}