#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D volume;

uniform vec3 viewPos;
uniform vec2 IsoValRange;
uniform float StepSize;
uniform vec3 volume_size;
uniform vec3 box_min;
uniform vec3 box_max;

void main(){
	vec3 view_dir = normalize(FragPos - viewPos);
	vec3 color_composited = vec3(0,0,0);
	float opaque_composited = 0;
	int i = 0;
	bool debug = false;
	while (true) {
		vec3 texPoint = FragPos + view_dir * StepSize * i++;
		float vol_u = (texPoint - box_min).x / (volume_size.x);
		float vol_v = (texPoint - box_min).y / (volume_size.y);
		float vol_w = (texPoint - box_min).z / (volume_size.z);
		if (0 > vol_v) {
			FragColor = vec4(0, 0, 1, 1);
			return;
		}
		if (vol_u > 1 || vol_v > 1 || vol_w > 1) {
			if (i == 1) {
				debug = false;
				color_composited = vec3(view_dir.x, view_dir.y, view_dir.z);
				opaque_composited = 1.0;
			}
			break;
		}
		float volume_sample = texture(volume, vec2(vol_u, vol_v)).r - vol_w;
		if (volume_sample < IsoValRange.x || volume_sample > IsoValRange.y) {
			continue;
		}
		vec3 color_self = vec3(0, 0, 1);
		float opaque_self = .1;

		color_composited += ((1.f - opaque_composited) * color_self * opaque_self);
		opaque_composited += (1.f - opaque_composited) * opaque_self;
		if (opaque_composited > .99) { break; }
		if (i > 100) { //should change
			break;
		}
	}
	if (debug) {
		FragColor = vec4(1, 0, 1, 1);
	}
	else {
		FragColor = vec4(color_composited, opaque_composited);
	}
}