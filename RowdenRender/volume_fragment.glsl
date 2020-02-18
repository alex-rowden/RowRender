//FRAGMENT SHADER

#version 330 core

out vec4 FragColor;

in vec2 TexCoord;



//uniform sampler2D texture_diffuse1;
uniform sampler2D volume0;
uniform sampler2D volume1;
uniform sampler2D volume2;
uniform sampler2D volume3;
uniform sampler2D volume4;
uniform sampler2D volume5;
uniform sampler2D fhp;
uniform sampler2D bhp;

uniform vec3 viewPos;
uniform vec2 IsoValRange;
uniform float StepSize;
uniform float increment;
uniform vec3 volume_size;
uniform vec3 box_min;
uniform vec3 box_max;
uniform float base_opac;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 color3;
uniform vec3 color4;
uniform vec3 color5;
uniform vec3 color6;
uniform int numTex;

uniform int enabledVolumes;

#define EPSILON 1e-4

void main() {
	vec3 front = (texture(fhp, TexCoord).xyz  * 50.0f + box_min);
	vec3 back = (texture(bhp, TexCoord).xyz * 50.0f + box_min);
	//vec3 view_dir = vec3(0);
	vec3 start = vec3(0);
	vec3 end = vec3(0);
	if (back.x < EPSILON || back.y < EPSILON || back.z < EPSILON) {
		FragColor = vec4(0, 0, 1, 0);
		return;
	}

	if (front.x < EPSILON || front.y < EPSILON || front.z < EPSILON) {
		start = viewPos;
		end = back;
		//FragColor = vec4(vec3(1,0,0), 1.0);
	}
	else {
		start = front;
		end = back;
		//FragColor = vec4(vec3(0, 1, 0), 1.0);
	}
	//FragColor = vec4((back - box_min)/50.00, 1.0);
	//return;
	
	//view_dir = normalize(FragPos - viewPos);
	vec3 color_composited = vec3(0, 0, 0);
	float opaque_composited = 0;
	float i = 0;
	bool debug = false;


	vec3 view_dir = normalize(back - start);
	float distance = sqrt(dot(end - start, end - start));
	FragColor = vec4(vec3(distance / 75.0f), 1.0);
	//FragColor = vec4(viewPos / 50.0f, 1);
	//return;
	for (i; i < distance; i += StepSize) {
		vec3 texPoint =  start + view_dir * i;
		float vol_u = (texPoint - box_min).x / (volume_size.x);
		float vol_v = (texPoint - box_min).y / (volume_size.y);
		float vol_w = (texPoint - box_min).z / (volume_size.z);
		
		
		//sampler2D volume;
		vec3 color;
		float volume_sample;
		for (int i = 0; i < numTex; i++) {
			if ((enabledVolumes & (1 << i)) < 1)
				continue;

			bool shade_intersection = false;
			switch (i) {
			case 0:
				color = vec3(color1);
				volume_sample = texture(volume0, vec2(vol_u, vol_v)).r - vol_w * increment;;
				//color = make_float4(0, 0, 1, 1.0f);
				break;
			case 1:
				color = vec3(color2);
				volume_sample = texture(volume1, vec2(vol_u, vol_v)).r - vol_w * increment;;
				break;
			case 2:
				color = vec3(color3);
				volume_sample = texture(volume2, vec2(vol_u, vol_v)).r - vol_w * increment;;
				break;
			case 3:
				color = vec3(color4);
				volume_sample = texture(volume3, vec2(vol_u, vol_v)).r - vol_w * increment;;
				break;
			case 4:
				color = vec3(color5);
				volume_sample = texture(volume4, vec2(vol_u, vol_v)).r - vol_w * increment;;
				break;
			default:
				color = vec3(1.0, 1.0f, 1.0f);
			}


			vec3 color_self = vec3(0);
			float opaque_self = 0;
			if (volume_sample < IsoValRange.x || volume_sample > IsoValRange.y) {
				continue;
			}
			else {
				color_self = color;
				opaque_self = base_opac;
			}

			color_composited += ((1.f - opaque_composited) * color_self * opaque_self);
			opaque_composited += (1.f - opaque_composited) * opaque_self;

			if (opaque_composited > .99) { break; }
		}
	}
	
	if (debug) {
		FragColor = vec4(1, 0, 1, 1);
	}
	else {
		FragColor = vec4(color_composited, opaque_composited);
	}
	
}