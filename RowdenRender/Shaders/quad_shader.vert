#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 camera;
uniform mat4 projection;

uniform vec2 billboardSize;
uniform vec3 quad_center;

void main() {
	vec4 pos = projection * camera * vec4(quad_center, 1);
	pos.xyz /= pos.w;
	pos.xy += aPos.xy * billboardSize;
	gl_Position = pos;
	//vec4 fragPos4 = (model * vec4(aPos, 1));
	//FragPos = fragPos4.xyz / fragPos4.w;
	TexCoord = vec2(aTexCoord.x, 1-aTexCoord.y);
}