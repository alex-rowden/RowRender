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
uniform sampler2D normal0;
uniform sampler2D normal1;
uniform sampler2D normal2;
uniform sampler2D normal3;
uniform sampler2D normal4;
uniform sampler2D normal5;
uniform sampler2D fhp;
uniform sampler2D bhp;
uniform sampler2D depth_tex;
uniform sampler2D noise;

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
uniform vec3 shade_color;
uniform float shade_opac;
uniform float enable_intersection;
uniform int numTex;
uniform float zNear;
uniform float zFar;
uniform float spec_term, bubble_term, bubble_min, bubble_max, max_opac, min_opac, step_mod, tune;

uniform int enabledVolumes;

uniform vec2 sincosLightTheta;
uniform vec2 lightDirP;
uniform vec2 sincosHalfwayTheta;
uniform vec2 HalfwayVecP;
uniform float ambientStrength, diffuseStrength, specularStrength, shininess;

#define EPSILON 1e-4
#define M_PIf 3.1415926535897932384626433

float sdot(vec2 a, vec2 b) {
	return  sin(a.x) * sin(b.x) * cos(a.y - b.y) + cos(a.x) * cos(b.x);
}
//For use on a sincos vector and the normal vector. Uses precomputed sines and cosines
float sdot(vec2 sincosa, vec2 sincosnorm, float a, float phi) {
	return sincosa.x * sincosnorm.x * cos(a - phi) + sincosa.y * sincosnorm.y;
}

void main() {
	vec3 front = (texture(fhp, TexCoord).xyz);
	vec3 back = (texture(bhp, TexCoord).xyz);
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
	
	bool debug = false;


	vec3 view_dir = normalize(back - start);
	float i = EPSILON * tune * texture(noise, vec2(view_dir)).r;
	float distance = sqrt(dot(end - start, end - start));
	float raw_depth = texture(depth_tex, TexCoord).x * 2.0f - 1;
	float depth = 2.0 * zNear * zFar / (zFar + zNear - raw_depth * (zFar - zNear));
	float upperBoundStep = 5 * StepSize;
	//FragColor = vec4(vec3(distance / 75.0f), 1.0);
	//FragColor = vec4(viewPos / 50.0f, 1);
	//return;
	float nextDistance = upperBoundStep;
	for (i; i < distance; i += nextDistance) {
		nextDistance = upperBoundStep;
		vec3 texPoint =  start + view_dir * i;
		if (distance > depth) {
			FragColor = vec4(color_composited, opaque_composited);
			//FragColor = vec4(vec3(depth / 75.0f), 1.0);
			return;
		}
		float vol_u = (texPoint - box_min).x / (volume_size.x);
		float vol_v = (texPoint - box_min).y / (volume_size.y);
		float vol_w = (texPoint - box_min).z / (volume_size.z);
		
		
		//sampler2D volume;
		vec3 color;
		float volume_sample;
		vec3 normal_sample;
		bool shade_intersection = false;
		for (int i = 0; i < numTex; i++) {
			if ((enabledVolumes & (1 << i)) < 1)
				continue;

			
			switch (i) {
			case 0:
				color = vec3(color1);
				volume_sample = texture(volume0, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal0, vec2(vol_u, vol_v)).rgb;
				//color = make_float4(0, 0, 1, 1.0f);
				break;
			case 1:
				color = vec3(color2);
				volume_sample = texture(volume1, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal1, vec2(vol_u, vol_v)).rgb;
				break;
			case 2:
				color = vec3(color3);
				volume_sample = texture(volume2, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal2, vec2(vol_u, vol_v)).rgb;
				break;
			case 3:
				color = vec3(color4);
				volume_sample = texture(volume3, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal3, vec2(vol_u, vol_v)).rgb;
				break;
			case 4:
				color = vec3(color5);
				volume_sample = texture(volume4, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal4, vec2(vol_u, vol_v)).rgb;
				break;
			default:
				color = vec3(1.0, 1.0f, 1.0f);
			}


			//vec3 color_self = vec3(normal_sample);
			vec3 color_self = vec3(0);
			float opaque_self = 0;
			float phi = normal_sample.x * M_PIf;
			float theta = normal_sample.y * M_PIf * 2 - M_PIf;
			nextDistance = min(nextDistance, StepSize * (1 + step_mod * min(abs(volume_sample - IsoValRange.x), abs(volume_sample - IsoValRange.y))));
			if (volume_sample < IsoValRange.x || volume_sample > IsoValRange.y) {
				continue;
			}
			else {
				if (shade_intersection && (enable_intersection > 0)) {
					color = shade_color;
					opaque_self = shade_opac;
				}
				else {
					opaque_self = base_opac;
				}
				shade_intersection = true;
				float sinphi = sin(phi);
				vec3 normal = vec3(sinphi * cos(theta), sinphi * sin(theta), cos(phi));
				vec2 normalP = vec2(phi, theta);
				vec2 sincosnorm = vec2(sin(theta), cos(theta));
				float diffuse = diffuseStrength * max(0, sdot(sincosLightTheta, sincosnorm, lightDirP.y, phi));
				float spec = specularStrength * pow(abs(sdot(sincosHalfwayTheta, sincosnorm, HalfwayVecP.y, phi)), shininess);
				color_self = ambientStrength * color + diffuse * color + spec * vec3(1, 1, 1);
				
				float bubble_coefficient = 1 - (abs(dot(view_dir, normal)));
				//float bubble_coefficient = 1 - (fabs(sdot(CameraDirP, normalP)));
				
				
				if (opaque_self > 1e-5) {
					if (bubble_coefficient > bubble_min) {
						bubble_coefficient = bubble_min;
					}
					else if (bubble_coefficient < bubble_max) {
						bubble_coefficient = bubble_max;
					}

					float norm = (((bubble_coefficient - bubble_max) / (bubble_min - bubble_max)));
					bubble_coefficient = (norm * (max_opac - min_opac)) + min_opac;
					

					
				}

				opaque_self = opaque_self + (abs(bubble_term) * bubble_coefficient + ((spec_term) * spec));// 0.5f);
				//if (ShadingTerms.y > 0) {
					//color_self *= powf(1 - bubble_coefficient, 1 / tune);
				//}
			}

			color_composited += ((1.f - opaque_composited) * color_self * opaque_self);
			opaque_composited += (1.f - opaque_composited) * opaque_self;

			if (opaque_composited > .99) { 
				FragColor = vec4(color_composited, opaque_composited);
				return;
			}
		}
	}
	
	if (debug) {
		FragColor = vec4(1, 0, 1, 1);
	}
	else {
		FragColor = vec4(color_composited, opaque_composited);
	}
	
}