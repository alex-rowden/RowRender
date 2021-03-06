//FRAGMENT SHADER

#version 420 core

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
uniform float fcp;

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
	float i = .08 * (abs(texture(noise, vec2(view_dir)).r) + .2);
	float distance = sqrt(dot(end - start, end - start));
	float raw_depth = texture(depth_tex, TexCoord).x * 2.0f - 1;
	float depth = 2.0 * zNear * zFar / (zFar + zNear - raw_depth * (zFar - zNear));
	distance = min(distance - StepSize, depth);
	float upperBoundStep = 5 * StepSize;
	//FragColor = vec4(vec3(distance / 75.0f), 1.0);
	//FragColor = vec4(viewPos / 50.0f, 1);
	//return;
	float nextDistance = upperBoundStep;
	bool above_arr[6] = { false, false, false, false, false, false };
	bool above = false;
	bool firstStep = true;
	for (i; i < distance; i += nextDistance) {
		nextDistance = upperBoundStep;
		vec3 texPoint =  start + view_dir * i;
		//if (i > depth) {
		//	FragColor = vec4(color_composited, opaque_composited);
			//FragColor = vec4(vec3(depth / 75.0f), 1.0);
		//	return;
		//}
		float vol_u = (texPoint - box_min).x / (volume_size.x);
		float vol_v = (texPoint - box_min).y / (volume_size.y);
		float vol_w = (texPoint - box_min).z / (volume_size.z);
		
		
		//sampler2D volume;
		vec3 color;
		float volume_sample;
		vec3 normal_sample;
		bool inRange = false;
		bool shade_intersection = false;
		float last_sample = 0;
		for (int i = 0; i < numTex; i++) {
			if ((enabledVolumes & (1 << i)) < 1 && !firstStep)
				continue;


			switch (i) {
			case 0:
				color = vec3(color1);
				volume_sample = texture(volume0, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal0, vec2(vol_u, vol_v)).rgb;
				if (firstStep) {
					above_arr[i] = volume_sample > IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f;

				}
				//color = make_float4(0, 0, 1, 1.0f);
				break;
			case 1:
				color = vec3(color2);
				volume_sample = texture(volume1, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal1, vec2(vol_u, vol_v)).rgb;
				if (firstStep) {
					above_arr[i] = volume_sample > IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f;
				}				break;
			case 2:
				color = vec3(color3);
				volume_sample = texture(volume2, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal2, vec2(vol_u, vol_v)).rgb;
				if (firstStep) {
					above_arr[i] = volume_sample > IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f;
				}				break;
			case 3:
				color = vec3(color4);
				volume_sample = texture(volume3, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal3, vec2(vol_u, vol_v)).rgb;
				if (firstStep) {
					above_arr[i] = volume_sample > IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f;
				}				break;
			case 4:
				color = vec3(color5);
				volume_sample = texture(volume4, vec2(vol_u, vol_v)).r - vol_w * increment;
				normal_sample = texture(normal4, vec2(vol_u, vol_v)).rgb;
				if (firstStep) {
					above_arr[i] = volume_sample > IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f;
				}				break;
			default:
				color = vec3(1.0, 1.0f, 1.0f);
			}


			//vec3 color_self = vec3(normal_sample);
			vec3 color_self = vec3(0);
			float opaque_self = 0;
			float phi = normal_sample.x * M_PIf;
			float theta = normal_sample.y * M_PIf * 2 - M_PIf;
			nextDistance = min(nextDistance, StepSize * (1 + step_mod * (IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f)));
			nextDistance = nextDistance + StepSize * tune * texture(noise, vec2(normal_sample)).r;
			above = above_arr[i];
			if ((volume_sample < IsoValRange.y && volume_sample > IsoValRange.x) && !inRange) {
				inRange = true;
				last_sample = volume_sample;
			}
				
			//volume_sample > IsoValRange.y;
			if ((above && (volume_sample < IsoValRange.x)) || (!above && volume_sample > IsoValRange.y)) {
				opaque_self = base_opac * (1 - abs(volume_sample - IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f));
				if (shade_intersection && (enable_intersection > 0)) {
					float shade_coeff = 1;//(1 - abs(volume_sample - last_sample));
					color = shade_color * shade_coeff + color * (1 - shade_coeff);
					opaque_self = base_opac * (1 - abs(volume_sample - IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f));
					opaque_self = opaque_self + shade_opac * shade_coeff;
				}
				else if ((above && (volume_sample > (IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f))) || (!above && volume_sample < (IsoValRange.x + (IsoValRange.y - IsoValRange.x) / 2.0f))) {
					if (inRange)
						shade_intersection = true;
					continue;
				}
				above_arr[i] = !above;
			}
			else {
				if (inRange) { shade_intersection = true;  }continue;
		}
			float sinphi = sin(phi);
			vec3 normal = vec3(sinphi * cos(theta), sinphi * sin(theta), cos(phi));

			vec2 normalP = vec2(phi, theta);
			vec2 sincosnorm = vec2(sin(theta), cos(theta));
			float diffuse = diffuseStrength * max(0, sdot(sincosLightTheta, sincosnorm, lightDirP.y, phi));
			float spec = specularStrength * pow(max(sdot(sincosHalfwayTheta, sincosnorm, HalfwayVecP.y, phi), 0), shininess);
			//spec = specularStrength * pow(dot(halfway))
			color_self = ambientStrength * color + diffuse * color + spec * vec3(1, 1, 1);

			float bubble_coefficient = 0;
			if(bubble_term != 0){
				bubble_coefficient = 1 - (abs(dot(view_dir, normal)));
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
				



			}
			if ( !(shade_intersection && (enable_intersection > 0)))
				opaque_self = opaque_self + (bubble_term * bubble_coefficient + ((spec_term)*spec));// 0.5f);
				
																									//if (ShadingTerms.y > 0) {
				//color_self *= powf(1 - bubble_coefficient, 1 / tune);
			//} 
			if (inRange) {
				shade_intersection = true;
			}
			
			color_composited += ((1.f - opaque_composited) * color_self * opaque_self);
			opaque_composited += (1.f - opaque_composited) * opaque_self;
			
			if (opaque_composited > .99) { 
				FragColor = vec4(color_composited, opaque_composited);
				return;
			}
		}
		
		if (firstStep) {
			nextDistance = 0;
			firstStep = false;
			continue;
		}
	}
	
	if (debug) {
		FragColor = vec4(1, 0, 1, 1);
	}
	else {
		FragColor = vec4(color_composited, opaque_composited);
	}
	
}