#version 420

layout(location = 0) out vec4 minimap_tex;

in vec2 TexCoords;

uniform sampler2D minimap_base_tex;
uniform vec3 playerPos;
uniform float playerRadius, aspect;

uniform vec2 bl, tr;

void main(){
	vec2 pos = ((playerPos.xy - bl)/(tr - bl)).yx;
	pos.y = 1 - pos.y;
	pos.x = (1 - pos.x);
	if(distance(TexCoords * vec2(1, aspect), pos * vec2(1, aspect)) < playerRadius){
		minimap_tex = vec4(1,0,0,1);
	}else
		minimap_tex = texture2D(minimap_base_tex, vec2(TexCoords.x, 1 - TexCoords.y));	
}