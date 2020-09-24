#version 330 core

out vec4 ssao_tex;
in vec2 TexCoords;

uniform sampler2D normal_tex;
uniform sampler2D fragPos_tex;
uniform sampler2D texNoise;

uniform vec2 resolution;

uniform vec3 samples[64];
uniform mat4 projection;

uniform int kernelSize;
uniform float radius, bias;

void main() {
	vec2 noiseScale = resolution / 4.0;
	vec3 fragPos = texture(fragPos_tex, TexCoords).xyz;
	vec3 normal = texture(normal_tex, TexCoords).rgb;
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0;

	for (int i = 0; i < kernelSize; i++) {
		vec3 sample_vec = TBN * samples[i]; //transform to view space
		sample_vec = fragPos + sample_vec * radius;

		vec4 offset = vec4(sample_vec, 1.0);
		offset = projection * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  

		float sampleDepth = texture(fragPos_tex, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= sample_vec.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	ssao_tex.r = occlusion;
	ssao_tex.a = 1;
}