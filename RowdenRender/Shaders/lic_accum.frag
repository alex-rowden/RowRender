#version 430

out vec4 LIC;

in vec2 TexCoord;

uniform sampler2D lic_tex[2];
uniform int lic_index;

void main()
{
	LIC = texture(lic_tex[lic_index], TexCoord);
}