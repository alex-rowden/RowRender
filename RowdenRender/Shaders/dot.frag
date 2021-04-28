#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec2 mousePos;
uniform float radius, aspect;

void main()
{
	FragColor = texture2D(texture_diffuse1, vec2(TexCoords.x, 1 - TexCoords.y));
	if(distance(TexCoords * vec2(1, aspect), mousePos * vec2(1, aspect)) < radius){
		FragColor = vec4(1,0,0,1);		
	}

}