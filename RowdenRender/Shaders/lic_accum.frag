#version 430

out vec4 LIC;

in vec2 TexCoord;

uniform sampler2D lic_tex[2];
uniform sampler2D lic_accum_tex[2];
uniform int lic_index, force_index;

void main()
{
	vec4 source = texture(lic_tex[lic_index], TexCoord);
	vec4 destination = texture(lic_accum_tex[force_index], TexCoord);
	LIC = source * source.a + destination * (1 - source.a); 
}