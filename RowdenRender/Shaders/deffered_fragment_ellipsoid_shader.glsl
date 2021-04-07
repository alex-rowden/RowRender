#version 430 core

#define MAX_POINT_LIGHTS 80
#define NR_DIR_LIGHTS 0
#define MAX_ROUTERS 20

const float PI = 3.1415926535897932384626433832795;
const float K = -16/(PI * PI);

in vec2 TexCoord;

//deffered textures
uniform sampler2D normal_tex;
uniform sampler2D albedo_tex;
uniform sampler2D fragPos_tex;
uniform sampler2D ellipsoid_coordinates_tex;
uniform sampler2D tangent_tex;
uniform sampler2D lic_accum_tex;
uniform sampler2D ssao_blur_tex;

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

uniform int force_index;

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
num_routers, num_contours, power, num_lic;

uniform bool display_names, lic_on, multirouter,
texton_background, invert_colors, frequency_bands, use_mask,
screen_space_lic, cull_discontinuities, procedural_noise,
normalize_vectors, color_weaving, blending, render_textons;

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

	return vec2((cosTheta * inp.x - sinTheta * inp.y) * 10 * u_stretch,
		(sinTheta * inp.x + cosTheta * inp.y) * 10 * v_stretch);
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

float paintTextons(vec3 fragPos, int i, int num_routers_per_freq, int router_counter, vec3 normal) {
	

	
	vec2 index = texture(ellipsoid_coordinates_tex, TexCoord).rg;//(vec2(dot(tangent, modified_coords), dot(bitangent, modified_coords)) + vec2(1)) / 2.0f;
	if (abs(dot(normal, vec3(0, 0, 1))) > 1e-4) {
		index = fragPos.xy;
	}
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
	if (num_routers_per_freq == 1) {
		mask *= .5;
	}
	return mask;
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



vec3 calculateColor(vec3 fragPos, vec3 Normal) {
	
	vec3 color = texture(albedo_tex, TexCoord).rgb;
	if (any(not(equal(color, vec3(1))))) {
		return color;
	}
	if (num_routers > 0 && lic_on && abs(dot(Normal, vec3(0,0,1))) < 1 - 1e-3) {
		vec4 lic_color = texture(lic_accum_tex, TexCoord);
		if (lic_color.a < 0) 
			return color;
		vec3 ret = lic_color.a * lic_color.rgb;
		ret = ret + color * (1 - lic_color.a);
		return ret;
	}
	
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
				alpha_new = paintTextons(fragPos, i, num_routers_per_freq[int(Ellipsoids[i].r.w)], router_counter[int(Ellipsoids[i].r.w)], Normal);
				//vec2 index = (vec2(dot(abs(tangent), fragPos), dot(abs(bitangent), fragPos)));
				
				//return bitangent;
				if(!render_textons)
					alpha_new = 0;
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
				float iso_band = mod(linear_term * -log2(distance / extent), frequency);
				float contour_num = ceil(linear_term * -log2(distance / extent) / frequency);
				
				if (iso_band/frequency > thickness * pow(.5*contour_num, 3)) {
					alpha_new = 0;
				}
				else {
					alpha_new = min(1, 1.2 -  distance / extent);
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
						float num_segments = floor(10 * num_contours / (contour_num * contour_num));
						if (num_segments < 2) {
							num_segments = 0;
						}
						if (mod(norm_theta * num_segments, 2) < 1.5 &&
							mod(norm_theta * num_segments, 2) > .5) {
							vec2 index = vec2(mod(norm_theta * num_segments, 2) - .5f,
								1 - mod(linear_term * -log2(distance / extent), frequency)
								/ (frequency * (thickness * pow(.5 * contour_num, 3))));
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
				}/*
				else if(!multirouter){
					//alpha_new = renderLIC(fragPos, tangent, bitangent, Normal, distance, i);
				}*/
				else {
					alpha_new = 0;
				}
			}

			ret = alpha_new * (alpha * color) + ret;
			alpha = (1 - alpha_new) * alpha;
		}
	}
	/*
	if (lic_on && multirouter){
		if (dot(Normal, vec3(0,0,1)) == 0) {
			//vec4 color = renderLIC(fragPos, tangent, bitangent, Normal);
			//ret = color.a * (alpha * color.rgb) + ret;
			//alpha = (1 - color.a) * alpha;
		}
		else {
			ret = 0 * (alpha * color) + ret;
			alpha = (1 - 0) * alpha;
		}
	}
	*/
	
	if (texton_background) {
		float alpha_new = 1;
		for (int i = 0; i < num_routers; i++){
			float distance = ellipsoidDistance(fragPos, Ellipsoids[i]);
			if (distance > extent)
				continue;
			vec3 color = texture(wifi_colors, vec2(0, Ellipsoids[i].mu.w)).rgb;;
			alpha_new = paintTextons(fragPos, i, num_routers_per_freq[int(Ellipsoids[i].r.w)], router_counter[int(Ellipsoids[i].r.w)], Normal);
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


	vec3 ambient = light.ambient * color * texture(ssao_blur_tex, TexCoord).r;
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