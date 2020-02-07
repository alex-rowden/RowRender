//FRAGMENT SHADER

#version 330 core

out vec4 FragColor;

in vec2 TexCoord;


//uniform sampler2D texture_diffuse1;
uniform sampler2D volume;
uniform sampler2D fhp;
uniform sampler2D bhp;

uniform vec3 viewPos;
uniform vec2 IsoValRange;
uniform float StepSize;
uniform float increment;
uniform vec3 volume_size;
uniform vec3 box_min;
uniform vec3 box_max;

void main() {
	vec4 col = texture(fhp, TexCoord);
	if(true)//if (col.x != 0 && col.y != 0 && col.z != 0)
		FragColor = vec4(col.xyz, .3);
	else
		FragColor = vec4(0);
	/*
	vec3 view_dir = normalize(FragPos - viewPos);
	vec3 color_composited = vec3(0, 0, 0);
	float opaque_composited = 0;
	int i = 0;
	bool debug = false;

	vec3 fhp = FragPos;
	
	for (i; i < 1000; i++) {
		vec3 texPoint =  fhp + view_dir * StepSize * i;
		float vol_u = (texPoint - box_min).x / (volume_size.x);
		float vol_v = (texPoint - box_min).y / (volume_size.y);
		float vol_w = (texPoint - box_min).z / (volume_size.z);
		
		if (vol_u > 1 || vol_v > 1 || vol_w > 1 || vol_u < -1e-4 || vol_v < -1e-4 || vol_w < -1e-4) {
			FragColor = vec4(color_composited, opaque_composited);
			return;
		}
		float volume_sample = texture(volume, vec2(vol_u, vol_v)).r - vol_w * increment;
		vec3 color_self = vec3(0);
		float opaque_self = 0;
		if (volume_sample < IsoValRange.x || volume_sample > IsoValRange.y) {
			continue;
		}
		else {
			color_self = vec3(0, 0, 1);
			opaque_self = .1;
		}

		color_composited += ((1.f - opaque_composited) * color_self * opaque_self);
		opaque_composited += (1.f - opaque_composited) * opaque_self;

		if (opaque_composited > .99) { break; }
		
	}
	
	if (debug) {
		FragColor = vec4(1, 0, 1, 1);
	}
	else {
		FragColor = vec4(color_composited, opaque_composited);
	}
	*/
}