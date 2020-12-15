#version 430 core

#define MAX_POINT_LIGHTS 80
#define NR_DIR_LIGHTS 0
#define MAX_ROUTERS 20
#define NUM_STEPS 72

const float PI = 3.1415926535897932384626433832795;
const float K = -16/(PI * PI);

in vec2 TexCoord;

//deffered textures
uniform sampler2D normal_tex;
uniform sampler2D albedo_tex;
uniform sampler2D fragPos_tex;
uniform sampler2D ellipsoid_coordinates_tex;
uniform sampler2D tangent_tex;

//stored textures
uniform sampler2D frequency_tex;
uniform sampler2D wifi_colors;
uniform sampler2D text_tex;
uniform sampler2D ellipsoid_tex;
uniform sampler2D noise_tex[20];
 //uniform sampler2D tangent_tex;

uniform mat4 camera;
uniform mat4 projection;

float ellipsoid_ration = 1400.0f / 1600.0f;
float ellipsoid_offset = (1 - ellipsoid_ration) / 2.0;

out vec4 FragColor;

struct Ellipsoid {
	vec4 mu;
	vec4 r;
	mat4 axes;

};



struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};


struct DirLight {
	vec3 direction;

	vec3 color;
};


uniform float ambient_coeff, diffuse_coeff, spec_coeff,
u_stretch, v_stretch, linear_term, thickness,
delta_theta, extent, frequency, learning_rate, alpha_boost, density,
frag_pos_scale, tunable, cling;

uniform int shininess, num_point_lights, num_frequencies,
num_routers, num_contours, power;

uniform bool display_names, lic_on, multirouter,
texton_background, invert_colors, frequency_bands, use_mask,
screen_space_lic, cull_discontinuities, procedural_noise,
normalize_vectors, color_weaving, blending;

uniform mat4 ellipsoid_transform;

uniform vec3 radius_stretch, viewPos;

uniform vec3 selectedPos;

layout(std140) uniform EllipsoidBlock{
	Ellipsoid Ellipsoids[MAX_ROUTERS];
};

uniform PointLight pointLights[MAX_POINT_LIGHTS];

vec2 rotateVector(float theta, vec2 inp) {
	float rad = radians(theta);
	float cosTheta = cos(rad);
	float sinTheta = sin(rad);

	return vec2((cosTheta * inp.x - sinTheta * inp.y) * 100 * u_stretch,
		(sinTheta * inp.x + cosTheta * inp.y) * 100 * v_stretch);
}

vec3 ellipsoidCoordinates(vec3 fragPos, Ellipsoid ellipsoid) {
	vec3 modified_coords = ellipsoid.mu.xyz;
	modified_coords = (ellipsoid_transform * vec4(modified_coords.xyz, 1)).xyz;
	modified_coords = (fragPos - modified_coords);

	return modified_coords;
}

float ellipsoidDistance(vec3 fragPos, vec3 ellipsoid_coords, Ellipsoid ellipsoid) {
	vec3 modified_coords = mat3(ellipsoid.axes) * ellipsoid_coords;
	modified_coords = (modified_coords * modified_coords) / (9 * abs(mat3(ellipsoid.axes) * abs(radius_stretch * ellipsoid.r.xyz)));

	float distance = 0;
	for (int i = 0; i < 3; i++) {
		distance += modified_coords[i];
	}
	return sqrt(distance);
}

float ellipsoidDistance(vec3 fragPos, Ellipsoid ellipsoid) {
	return ellipsoidDistance(fragPos, ellipsoidCoordinates(fragPos, ellipsoid), ellipsoid);
}

float paintTextons(vec3 fragPos, int i, int num_routers_per_freq, int router_counter) {
	vec3 modified_coords = ellipsoidCoordinates(fragPos, Ellipsoids[i]);

	vec2 index = texture(ellipsoid_coordinates_tex, TexCoord).rg;//(vec2(dot(tangent, modified_coords), dot(bitangent, modified_coords)) + vec2(1)) / 2.0f;
	index = rotateVector(Ellipsoids[i].r.w * delta_theta, index);
	float offset = .5;
	if (abs(fract(index.g) - .5) < 1 / 4.0f) {
		offset = 0;
	}
	int router_num = int((fract(index.r + offset) - ellipsoid_offset) * 1 / ellipsoid_ration * num_routers_per_freq);
	float mask = texture(frequency_tex, index).r;
	//alpha_new = mask;
	if (router_num > router_counter)
		mask = 0;
	return mask;
}
/*
double dsin(double x)
{
	//minimax coefs for sin for 0..pi/2 range
	const double a3 = -1.666665709650470145824129400050267289858e-1LF;
	const double a5 = 8.333017291562218127986291618761571373087e-3LF;
	const double a7 = -1.980661520135080504411629636078917643846e-4LF;
	const double a9 = 2.600054767890361277123254766503271638682e-6LF;

	const double m_2_pi = 0.636619772367581343076LF;
	const double m_pi_2 = 1.57079632679489661923LF;

	double y = abs(x * m_2_pi);
	double q = floor(y);
	int quadrant = int(q);

	double t = (quadrant & 1) != 0 ? 1 - y + q : y - q;
	t *= m_pi_2;

	double t2 = t * t;
	double r = fma(fma(fma(fma(a9, t2, a7), t2, a5), t2, a3), t2 * t, t);

	r = x < 0 ? -r : r;

	return (quadrant & 2) != 0 ? -r : r;
}
*/

//"stolen" from thebookofshaders.com

float rand(float n) { return fract(sin(n) * 43758.5453123); }
//float rand(float p) { p = fract(p * 0.011); p *= p + 7.5; p *= p + p; return step(fract(p), density); }

//double rand(double n) { return step(fract(dsin(n) * 43758.5453123), density); }

float rand(vec2 st) {
	return step(fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123), density);
}

float rand(vec3 p3)
{
	p3 = fract(p3 * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	float ret = fract((p3.x + p3.y) * p3.z) - density;
	if (ret < 0)
		return 0.f;
	else
		return ret / (1 - density);
}
float noise(float p) {
	float fl = floor(p);
	float fc = fract(p);
	return step(mix(rand(fl), rand(fl + 1.0), fc), density);
}


float noise(vec2 n, int ellipsoid_num) {
	if (procedural_noise) {
		vec2 i = floor(n);
		vec2 f = fract(n);
		float a = rand(i);
		float b = rand(i + vec2(1.0, 0.0));
		float c = rand(i + vec2(0.0, 1.0));
		float d = rand(i + vec2(1.0, 1.0));
		vec2 u = smoothstep(0., 1., f);
		return mix(a, b, u.x) +
			(c - a) * u.y * (1.0 - u.x) +
			(d - b) * u.x * u.y;
	}
	return texture(noise_tex[ellipsoid_num], n).r;
}

float noise(vec3 x) {
	vec3 i = floor(x);
	vec3 f = fract(x);
	f = f * f * (3.0 - 2.0 * f);

	return	 mix(mix(mix(rand(i + vec3(0, 0, 0)),
						rand(i + vec3(1, 0, 0)), f.x),
					mix(rand(i + vec3(0, 1, 0)),
						rand(i + vec3(1, 1, 0)), f.x), f.y),
				mix(mix(rand(i + vec3(0, 0, 1)),
						rand(i + vec3(1, 0, 1)), f.x),
					mix(rand(i + vec3(0, 1, 1)),
						rand(i + vec3(1, 1, 1)), f.x), f.y), f.z);
} 

vec3 slerp(vec3 p0, vec3 p1, float t)
{
	float dotp = dot(normalize(p0), normalize(p1));
	if (dotp > 1){
		dotp = 1;
	}
	else if (dotp < -1) {
		dotp = -1;
	}
	float theta = acos(dotp) * t;
	vec3 P = p1 - p0 * dotp;
	P = normalize(P);
	return p0 * cos(theta) + P * sin(theta);
}

vec3 projectCoords(vec3 a, vec3 tangent, vec3 bitangent, vec3 normal) {
	return vec3(dot(tangent, a), dot(bitangent, a), dot(normal, a));
}

vec2 getScreenCoords(vec3 fragPos) {
	vec4 screenSpaceCoords = projection * camera * vec4(fragPos.xyz, 1);
	screenSpaceCoords /= screenSpaceCoords.w;
	vec2 fakeTex = screenSpaceCoords.xy * .5 + vec2(.5);
	return fakeTex;
}

float calculateMask(int dir, int j) {
	float mask = 0;
	if (use_mask) {
		mask = cos(2. * 3.1415 * (NUM_STEPS - j) / (NUM_STEPS));
		return (0.5 + 0.5 * mask);
	}return NUM_STEPS - dir * j;
}


vec4 calculateForce(vec3 fragPos , mat3 worldToWallCoords, mat3 wallToWorldCoords) {
	vec3 direction = vec3(0);
	
	bool empty = true;
	for (int i = 0; i < num_routers; i++) {
		vec3 modified_coords = ellipsoidCoordinates(fragPos, Ellipsoids[i]);
		
		float ellipsoid_dist = ellipsoidDistance(fragPos, modified_coords, Ellipsoids[i]);
		
		if (ellipsoid_dist/extent < .01) {
			return vec4(vec3(0), -1);
		}
		vec3 ellipsoid_coords = fragPos - modified_coords;
		if (ellipsoid_dist < extent) {
			empty = false;
			vec3 force = (fragPos - ellipsoid_coords);
			direction += force / (length(force) * length(force) * length(force));
		}
	}
	direction = worldToWallCoords * direction;
	//direction = normalize(direction);
	direction.z = 0;
	if (empty)
		return vec4(0, 0, 0, -1);

	vec3 ret = wallToWorldCoords * direction;
	float projected_magnitude = length(ret);

	if (projected_magnitude < tunable) {
		projected_magnitude = tunable;
	}

	//projected_magnitude += tunable;
	//magnitude += tunable * 10;
	return vec4( ret / projected_magnitude, 1);
}

vec3 calculateForce(vec3 fragPos, vec3 ellipsoid_coords, mat3 worldToWallCoords, mat3 wallToWorldCoords) {
	vec3 direction = fragPos - ellipsoid_coords;
	float magnitude = length(direction);
	direction = worldToWallCoords * direction;
	
	if (procedural_noise && !screen_space_lic) {
		direction.z = cling;
	}
	else {
		direction.z = 0;
	}
	vec3 ret = wallToWorldCoords * direction;
	float projected_magnitude = length(ret);
	
	if ( projected_magnitude < tunable) {
		projected_magnitude = tunable;
	}

	//projected_magnitude += tunable;
	//magnitude += tunable * 10;
	return ret / projected_magnitude;
}

float renderLIC(vec3 fragPos, vec3 tangent, vec3 bitangent, vec3 Normal, float ellipsoidal_distance, int i) {
	float alpha_new = 0;
	float scale = 1;
	if (screen_space_lic && !procedural_noise)
		scale = .1;
	else if (screen_space_lic && procedural_noise)
		scale = 100;
	else if (!screen_space_lic && procedural_noise)
		scale = 10;
	vec3 modified_coords = ellipsoidCoordinates(fragPos, Ellipsoids[i]);
	vec3 ellipsoid_coords = fragPos - modified_coords;
	

	vec3 oldFragPos = fragPos, oldTangent = tangent, oldNormal = Normal,
		oldBitangent = bitangent, originalFragPos = fragPos, originalTangent = tangent;
	float norm;
	float val;
	float step_size = learning_rate * .02;

	vec2 uv;
	vec3 uvt;

	vec3 projected_frag, projected_coords, newFragPos;
	int dir = -1;
	for (int num_directions = 0; num_directions < 2; num_directions++) {
		fragPos = originalFragPos;
		uvt = fragPos;
		Normal = oldNormal;
		
		tangent = originalTangent;
		oldTangent = tangent;
		bitangent = oldBitangent;
		mat3 wallToWorldCoords = mat3(tangent, bitangent, Normal);
		mat3 worldToWallCoords = transpose(wallToWorldCoords);
		uv = TexCoord;
		int j = 0;
		if (num_directions > 0)
			dir = 1;
		if (dir > 0)
			j = 1;
		for (j; j < NUM_STEPS / 2; j++) {
			if (screen_space_lic) {
				uvt = texture(ellipsoid_coordinates_tex, uv).xyz;
			}
			
			float mask = calculateMask(dir, j);// ;
			//mask *= mask;
			if (procedural_noise)
				val += noise(frag_pos_scale * scale * uvt.rgb) * mask;
			else {
				float noise_val = 0;
				noise_val += noise(frag_pos_scale * scale * uvt.rg, i);
				//noise_val = max(noise_val, noise(frag_pos_scale / 5.0f * scale * uvt.rg));
				val += noise_val * mask;
			}
			norm += mask;
			

			vec3 direction = calculateForce(fragPos, ellipsoid_coords, worldToWallCoords, wallToWorldCoords);
			float magnitude = length(direction);
			
			
			vec3 force = dir * (direction);
			
			oldFragPos = fragPos;
			fragPos += step_size * force;
			
			if (screen_space_lic) {
				uv = getScreenCoords(fragPos);
				
				if (uv.x > 1 || uv.y > 1 || uv.x < 0 || uv.y < 0)
					break;
				newFragPos = texture(fragPos_tex, uv).rgb;
				float delta = distance(newFragPos, oldFragPos);
				//check for areas of depth discontinuity
				if (distance(newFragPos, fragPos) > .005) {
					break;
				}
				
				tangent = texture(tangent_tex, uv).rgb;
				//check if we changed surfaces
				if (abs(dot(tangent, oldTangent)) < 1e-3) {
					if (cull_discontinuities) {
						float angle = acos(dot(newFragPos - oldFragPos, dir * direction * step_size) / (dir * magnitude * delta * step_size));
						float altitude = sin(angle) * delta;
						float partial_distance = sqrt(delta * delta - altitude * altitude);
						float remaining_distance = step_size - magnitude * partial_distance;
						
						fragPos = oldFragPos + partial_distance * force;
						//fragPos = texture(fragPos_tex, getScreenCoords(fragPos)).rgb;						
						
						Normal = texture (normal_tex, uv).rgb;
						bitangent = cross(Normal, tangent);
						wallToWorldCoords = mat3(tangent, bitangent, Normal);
						worldToWallCoords = transpose(wallToWorldCoords);
						vec3 new_direction = calculateForce(fragPos, ellipsoid_coords, worldToWallCoords, wallToWorldCoords);
						//if new direction and old direction point toward each other, flip the new direction
						//if (dot(force, dir * new_direction) < 0) {
							
						//	dir *= -1;
						//}
						
						fragPos += 2 * dir * remaining_distance * normalize(new_direction);
						if (any(notEqual(texture(normal_tex, getScreenCoords(fragPos)).rgb, Normal))) {
							dir *= -1; 
							fragPos += 3 * dir * remaining_distance * normalize(new_direction);
						}
						else {
							fragPos -= dir * remaining_distance * normalize(new_direction);
						}
						uv = getScreenCoords(fragPos);
						
						
						oldTangent = tangent;
						newFragPos = texture(fragPos_tex, uv).rgb;
					}
					
					
					
				}
				fragPos = newFragPos;
			}
			else {
				uvt = fragPos;
			}
			
			//projected_frag = 

		}
		dir *= -1;
	}
	alpha_new = float(alpha_boost * pow(val / norm, power));
	
	//alpha_new = sqrt(alpha_new);
	//alpha_new *= alpha_new;
	//alpha_new *= (1 - .9 * distance / extent);
	alpha_new = min(alpha_new, 1);
	return alpha_new;
}
vec4 renderLIC(vec3 fragPos, vec3 tangent, vec3 bitangent, vec3 Normal) {
	if (num_routers == 0)
		return vec4(0);
	float alpha_new = 0;
	float scale = 1;
	if (screen_space_lic && !procedural_noise)
		scale = .1;
	else if (screen_space_lic && procedural_noise)
		scale = 100;
	else if (!screen_space_lic && procedural_noise)
		scale = 10;
	bool found_color = false;
	
	vec3 color = vec3(0);
	float signal_strength = 0;
	if (!color_weaving) {
		float max_d = length(ellipsoidCoordinates(fragPos, Ellipsoids[0]));
		int index = 0;
		
		for (int i = 1; i < num_routers; i++) {
			float temp = length(ellipsoidCoordinates(fragPos, Ellipsoids[i]));

			if (temp < max_d) {
				max_d = temp;
				index = i;
			}
		}
		float color_ind = Ellipsoids[index].mu.w;
		if (invert_colors)
			color_ind = Ellipsoids[index].r.w / float(num_routers);
		color = texture(wifi_colors, vec2(0, color_ind)).rgb;
		found_color = true;
	}
	

	//vec3 projected_coords = worldToWallCoords * ellipsoid_coords.xyz;
	//vec3 projected_frag = worldToWallCoords * fragPos.xyz;
	vec3 oldFragPos = fragPos, oldTangent = tangent, oldNormal = Normal,
		oldBitangent = bitangent, originalFragPos = fragPos, originalTangent = tangent;
	float norm;
	float val;
	float step_size = learning_rate * .02;

	vec2 uv;
	vec3 uvt;

	vec3 projected_frag, projected_coords, newFragPos;
	float max_val = 0;
	int dir = -1;
	for (int num_directions = 0; num_directions < 2; num_directions++) {
		fragPos = originalFragPos;
		uvt = fragPos;
		Normal = oldNormal;

		tangent = originalTangent;
		oldTangent = tangent;
		bitangent = oldBitangent;
		mat3 wallToWorldCoords = mat3(tangent, bitangent, Normal);
		mat3 worldToWallCoords = transpose(wallToWorldCoords);
		uv = TexCoord;
		int j = 0;
		if (num_directions > 0)
			dir = 1;
		if (dir > 0)
			j = 1;
		for (j; j < NUM_STEPS / 2; j++) {
			if (screen_space_lic) {
				uvt =  texture(ellipsoid_coordinates_tex, uv).xyz;
			}
			

			//projected_frag = worldToWallCoords * fragPos.xyz;
			//projected_coords = worldToWallCoords * ellipsoid_coords.xyz;

			float mask = calculateMask(dir, j);// ;
			norm += mask;
			vec4 direction = calculateForce(fragPos, worldToWallCoords, wallToWorldCoords);
			//vec3 direction = projected_direction.xyz;
			//float theta = atan(modified_proj.y, modified_proj.x) + PI;
			//float mod_rate = learning_rate + K * (theta * theta + PI / 2.0 * theta);
			float magnitude = length(direction.xyz);
			
			if (magnitude < .01) {
				break;
			}
			//mask *= mask;
			float noise_val = 0;
			if (procedural_noise)
				noise_val += noise(frag_pos_scale * scale * uvt.rgb);
			else
				noise_val += noise(frag_pos_scale * scale * uvt.rg, 0);
			if (!found_color && noise_val > 0 &&  (max_val < noise_val)) {
				float max_d = length(ellipsoidCoordinates(fragPos, Ellipsoids[0]));
				int index = 0;
				max_val = noise_val;
				for (int i = 1; i < num_routers; i++) {
					float temp = length(ellipsoidCoordinates(fragPos, Ellipsoids[i]));

					if (temp < max_d) {
						max_d = temp;
						index = i;
					}
				}

				float color_ind = Ellipsoids[index].mu.w;
				if (invert_colors)
					color_ind = Ellipsoids[index].r.w / float(num_routers);
				if (!blending) {
					color = texture(wifi_colors, vec2(0, color_ind)).rgb;
				}else {
					signal_strength += max_d;
					color += max_d * texture(wifi_colors, vec2(0, color_ind)).rgb;
				}
			}
			//vec3 direction = (fragPos - ellipsoid_coords);
			//vec3 projected_direction = (projected_frag - projected_coords);
			val += noise_val * mask;

			
			vec3 force = dir * (direction.xyz) / (magnitude);
			//uvt += force * mod_rate;
			//projected_frag += force/3.0 * mod_rate;

			//vec3 a = slerp(force, tangent, cling);
			//vec3 b = slerp(force, bitangent, 0); 
			//force = tangent;


			//uvt += learning_rate * .1 * force/5.0;
			oldFragPos = fragPos;
			fragPos += step_size * force;
			//projected_frag += learning_rate * .1 * force / 5.0;
			//projected_frag = worldToWallCoords * fragPos.xyz;
			if (screen_space_lic) {
				uv = getScreenCoords(fragPos);
				
				if (uv.x > 1 || uv.y > 1 || uv.x < 0 || uv.y < 0)
					break;
				newFragPos = texture(fragPos_tex, uv).rgb;
				float delta = distance(newFragPos, oldFragPos);
				//check for areas of depth discontinuity
				
				if (distance(fragPos, newFragPos) > tunable) {
					break;
				}
				
				tangent = texture(tangent_tex, uv).rgb;
				//check if we changed surfaces
				if (distance(tangent, oldTangent) > 1e-3) {
					if (cull_discontinuities) {
						float angle = acos(dot(newFragPos - oldFragPos, dir * direction.xyz) / (dir * magnitude * delta));
						float altitude = sin(angle) * delta;
						float partial_distance = sqrt(delta * delta - altitude * altitude);
						float remaining_distance = step_size * magnitude - partial_distance;

						fragPos = oldFragPos + partial_distance * force.xyz;
						fragPos = texture(fragPos_tex, getScreenCoords(fragPos)).rgb;
						tangent = texture(tangent_tex, uv).rgb;

						Normal = texture (normal_tex, uv).rgb;
						bitangent = cross(Normal, tangent);
						wallToWorldCoords = mat3(tangent, bitangent, Normal);
						worldToWallCoords = transpose(wallToWorldCoords);
						vec3 new_direction = calculateForce(fragPos, worldToWallCoords, wallToWorldCoords).xyz;
						fragPos += remaining_distance * normalize(new_direction);
						uv = getScreenCoords(fragPos);
						tangent = texture(tangent_tex, uv).rgb;

						Normal = texture (normal_tex, uv).rgb;
						oldTangent = tangent;
						newFragPos = texture(fragPos_tex, uv).rgb;
					}	
				}
				fragPos = newFragPos;
				//uvt = fragPos;
			}
			else {
				uvt = fragPos;
			}
			
			//projected_frag = 

		}
		dir *= -1;
	}
	alpha_new = float(alpha_boost * pow(val / norm, power));
	
	//alpha_new = sqrt(alpha_new);
	//alpha_new *= alpha_new;
	//alpha_new *= (1 - .9 * distance / extent);
	alpha_new = min(alpha_new, 1);
	if (blending) {
		if (signal_strength != 0) {
			color = color / signal_strength;
		}
	}
	return vec4(color, alpha_new);
}


vec3 calculateColor(vec3 fragPos, vec3 Normal) {
	vec3 color = texture(albedo_tex, TexCoord).rgb;
	vec3 tangent = normalize(texture(tangent_tex, TexCoord).rgb);
	vec3 bitangent = normalize(cross(tangent, Normal));
	vec3 swap;
	//if (abs(dot(tangent, vec3(0,1,0))) > 1e-6) {
	//	swap = tangent;
	//	tangent = bitangent;
	//	bitangent = swap;
	//}
	vec3 ret = vec3(0);
	vec3 selectedFragPos = selectedPos.rgb;
	float dist = distance(fragPos, selectedFragPos);
	if (dist < .05) {
		color = vec3(1, 0, 0);
		return color;
	}
	float alpha = 1;
	int num_routers_per_freq[MAX_ROUTERS];
	int router_counter[MAX_ROUTERS];
	float distances[MAX_ROUTERS];
	if (texton_background || dot(Normal, vec3(0, 0, 1)) !=0) {
		for (int i = 0; i < min(num_frequencies, num_routers); i++) {
			num_routers_per_freq[i] = 0;
			router_counter[i] = 0;
			distances[i] = 0;
		}

		for (int i = 0; i < num_routers; i++) {
			distances[i] = ellipsoidDistance(fragPos, Ellipsoids[i]);
			if (distances[i] <= extent) {
				num_routers_per_freq[int(Ellipsoids[i].r.w)] += 1;
			}
		}
	}
	for (int i = 0; i < num_routers; i++) {
		float distance = 0;
		if (!texton_background || dot(Normal, vec3(0, 0, 1)) != 0)
			distance = ellipsoidDistance(fragPos, Ellipsoids[i]);
		else
			distance = distances[i];
		if (distance <= extent) {
			float alpha_new = 1;

			float color_ind = Ellipsoids[i].mu.w;
			if (invert_colors)
				color_ind = Ellipsoids[i].r.w / float(num_routers);
			vec3 color = texture(wifi_colors, vec2(0, color_ind)).rgb;

			if (!texton_background && (dot(Normal, vec3(0, 0, 1))) == 1) {
				alpha_new = paintTextons(fragPos, i, num_routers_per_freq[int(Ellipsoids[i].r.w)], router_counter[int(Ellipsoids[i].r.w)]);
				router_counter[int(Ellipsoids[i].r.w)]++;
			}
			else if ((dot(Normal, vec3(0, 0, 1))) == 1) {
				continue;
			}
			else if (-dot(Normal, vec3(0, 0, 1)) == 1) {
				alpha_new = 0;
			}
			else if (!lic_on) {
				//(abs(mod(linear_term * log2(distance / extent), frequency) / (frequency * thickness) - .5) > .2)
				float iso_band = mod(linear_term * log2(distance / extent), frequency);
				if (iso_band > frequency * thickness) {
					alpha_new = 0;
				}
				else {
					if (frequency_bands) {
						iso_band = (iso_band - (1 - frequency * thickness)) / (frequency * thickness);
						int freq_number = int(Ellipsoids[i].r.w);
						if (mod(iso_band * (2 * freq_number + 1), 2) > 1) {
							alpha_new = 0;
						}
					}
					if (display_names) {
						vec3 modified_coords = ellipsoidCoordinates(fragPos, Ellipsoids[i]);
						vec2 projected_coords = vec2(dot(tangent, modified_coords), dot(bitangent, modified_coords));
						float theta = atan(projected_coords.y, projected_coords.x);
						float norm_theta = max(theta / PI + 1, 0);
						if (mod(norm_theta * num_contours, 2) < 1) {
							vec2 index = vec2(mod(norm_theta * num_contours, 2), mod(linear_term * log2(distance / extent), frequency) / (frequency * thickness));
							index.y = index.y / num_routers + i / float(num_routers);
							alpha_new = texture(text_tex, index).r;
						}
					}
				}
			}
			else {
				if (distance / extent < .01) {
					if (distance / extent > .01 - .001)
						alpha_new = 1;
					else
						alpha_new = 0;
				}
				else if(!multirouter){
					alpha_new = renderLIC(fragPos, tangent, bitangent, Normal, distance, i);
				}
				else {
					alpha_new = 0;
				}
			}

			ret = alpha_new * (alpha * color) + ret;
			alpha = (1 - alpha_new) * alpha;
		}
	}
	
	if (lic_on && multirouter){
		if (dot(Normal, vec3(0,0,1)) == 0) {
			vec4 color = renderLIC(fragPos, tangent, bitangent, Normal);
			ret = color.a * (alpha * color.rgb) + ret;
			alpha = (1 - color.a) * alpha;
		}
		else {
			ret = 0 * (alpha * color) + ret;
			alpha = (1 - 0) * alpha;
		}
	}
	
	if (texton_background) {
		float alpha_new = 1;
		for (int i = 0; i < num_routers; i++){
			float distance = ellipsoidDistance(fragPos, Ellipsoids[i]);
			if (distance > extent)
				continue;
			vec3 color = texture(wifi_colors, vec2(0, Ellipsoids[i].mu.w)).rgb;;
			alpha_new = paintTextons(fragPos, i, num_routers_per_freq[int(Ellipsoids[i].r.w)], router_counter[int(Ellipsoids[i].r.w)]);
			router_counter[int(Ellipsoids[i].r.w)]++;
			ret = alpha_new * (alpha * color) + ret;
			alpha = (1 - alpha_new) * alpha;
		}
	}

	
 
	ret = alpha * color + ret;

	return ret;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	// attenuation
	float distance = length(light.position - fragPos);

	float attenuation = 1.0 / (light.constant + light.linear * distance +
		light.quadratic * (distance * distance));


	vec3 ambient = light.ambient * color;
	vec3 diffuse = light.diffuse * diff * color;
	vec3 specular = light.specular * spec * vec3(1, 1, 1);

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient * ambient_coeff + diffuse * diffuse_coeff + specular * spec_coeff);
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color)
{
	vec3 lightDir = normalize(light.direction);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);


	vec3 ambient = light.color * color;
	vec3 diffuse = light.color * diff * color;
	vec3 specular = light.color * spec * vec3(1, 1, 1);

	return (ambient * ambient_coeff + diffuse * diffuse_coeff + specular * spec_coeff);
}

void main()
{
	vec3 norm = normalize(texture(normal_tex, TexCoord).rgb);
	//tangent = normalize(texture(tangent_tex, TexCoord).rgb);
	//bitangent = normalize(cross(norm, tangent));
	vec4 FragPos4 = texture(fragPos_tex, TexCoord).xyzw;
	float mask = FragPos4.w;
	if (mask < 1)
		discard;
	vec3 FragPos = FragPos4.xyz;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 ret = vec3(0, 0, 0);
	vec3 color = calculateColor(FragPos, norm);
	//color = vec3(noise(500 * TexCoord.rg));

	for (int i = 0; i < num_point_lights; i++) {
		ret += CalcPointLight(pointLights[i], norm, FragPos, viewDir, color);
	}for (int i = 0; i < NR_DIR_LIGHTS; i++) {
		//color += CalcDirLight(dirLights[i], norm, FragPos, viewDir);
	}
	FragColor = vec4(ret, 1.0f);
}