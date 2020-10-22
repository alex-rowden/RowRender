#version 430 core

#define MAX_POINT_LIGHTS 80
#define NR_DIR_LIGHTS 0
#define MAX_ROUTERS 20
#define NUM_STEPS 512
const float PI = 3.1415926535897932384626433832795;

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
uniform sampler2D noise_tex;
 //uniform sampler2D tangent_tex;

float ellipsoid_ration = 1613.3333 / 2040;
float ellipsoid_offset = (1 - ellipsoid_ration) / 2.0;

out vec4 FragColor;

struct Ellipsoid {
	vec4 mu;
	vec4 r;
	mat4 axes;

};

layout(std140) uniform EllipsoidBlock{
	Ellipsoid Ellipsoids[MAX_ROUTERS];
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
uniform PointLight pointLights[MAX_POINT_LIGHTS];

struct DirLight {
	vec3 direction;

	vec3 color;
};


uniform float ambient_coeff, diffuse_coeff, spec_coeff,
u_stretch, v_stretch, linear_term, thickness,
delta_theta, extent, frequency, learning_rate;

uniform int shininess, num_point_lights, num_frequencies,
num_routers, num_contours;

uniform bool group_frequencies, display_names, lic_on;

uniform mat4 ellipsoid_transform;

uniform vec3 radius_stretch, viewPos;

uniform vec3 selectedPos;

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

float ellipsoidDistance(vec3 fragPos, Ellipsoid ellipsoid) {
	vec3 modified_coords = mat3(ellipsoid.axes) * ellipsoidCoordinates(fragPos, ellipsoid);
	modified_coords = (modified_coords * modified_coords) / (9 * abs(mat3(ellipsoid.axes) * abs(radius_stretch * ellipsoid.r.xyz)));

	float distance = 0;
	for (int i = 0; i < 3; i++) {
		distance += modified_coords[i];
	}
	return sqrt(distance);
}

vec3 calculateColor(vec3 fragPos, vec3 Normal) {
	vec3 color = texture(albedo_tex, TexCoord).rgb;
	vec3 tangent = texture(tangent_tex, TexCoord).rgb;
	vec3 bitangent = normalize(cross(tangent, Normal));
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
	for (int i = 0; i < min(num_frequencies, num_routers); i++) {
		num_routers_per_freq[i] = 0;
		router_counter[i] = 0;
	}

	for (int i = 0; i < num_routers; i++) {
		float distance = ellipsoidDistance(fragPos, Ellipsoids[i]);
		if (distance <= extent) {
			num_routers_per_freq[int(Ellipsoids[i].r.w)] += 1;
		}
	}

	for (int i = 0; i < num_routers; i++) {
		float distance = ellipsoidDistance(fragPos, Ellipsoids[i]);
		if (distance <= extent) {
			float alpha_new = 1;

			//if (abs(dot(Normal, vec3(1,0,0)))> 1e-4)
			//	mask = texture(crosshair_tex, vec2(distance / extent * frequency, radius * atan(modified_coords.z / modified_coords.y))).r;
			//if (abs(dot(Normal, vec3(0, 1, 0))) > 1e-4)
			//	mask = texture(crosshair_tex, vec2(distance / extent * frequency, radius * atan(modified_coords.x / modified_coords.z))).r;

			vec3 color = texture(wifi_colors, vec2(0, Ellipsoids[i].mu.w)).rgb;

			if ((dot(Normal, vec3(0, 0, 1))) > 1e-6) {
				
				vec3 modified_coords = ellipsoidCoordinates(fragPos, Ellipsoids[i]);

				vec2 index = texture(ellipsoid_coordinates_tex, TexCoord).rg;//(vec2(dot(tangent, modified_coords), dot(bitangent, modified_coords)) + vec2(1)) / 2.0f;
				index = rotateVector(Ellipsoids[i].r.w * delta_theta, index);
				float offset = .5;
				if (abs(fract(index.g) - .5) < 1 / 4.0f) {
					offset = 0;
				}
				int router_num = int((fract(index.r + offset) - ellipsoid_offset) * 1 / ellipsoid_ration * num_routers_per_freq[int(Ellipsoids[i].r.w)]);
				float mask = texture(frequency_tex, index).r;
				alpha_new = mask;
				if (router_num > router_counter[int(Ellipsoids[i].r.w)])
					alpha_new = 0;
				router_counter[int(Ellipsoids[i].r.w)]++;

			}
			else if (-dot(Normal, vec3(0, 0, 1)) > 1e-6) {
				alpha_new = 0;
			}
			else if (!lic_on){
				if (mod(linear_term * log2(distance / extent), frequency) < 1 - frequency * thickness) {
					alpha_new = 0;
				}
				else {
					if (abs(.5 - mod(linear_term * log2(distance / extent), frequency) / (frequency)) < thickness) {
						alpha_new = 0;
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
					alpha_new = 0;
				}
				else {
					vec3 modified_coords = ellipsoidCoordinates(fragPos, Ellipsoids[i]);
					vec3 ellipsoid_coords = fragPos - modified_coords;
					vec3 projected_coords = vec3(dot(tangent, ellipsoid_coords), dot(bitangent, ellipsoid_coords), dot(Normal, ellipsoid_coords));
					vec3 projected_frag = vec3(dot(tangent, fragPos), dot(bitangent, fragPos), dot(Normal, fragPos));
					float norm;
					float val;

					vec2 uvt = texture(ellipsoid_coordinates_tex, TexCoord).rg;;
					for (int j = 0; j < NUM_STEPS; j++) {
						//float mask = 1;//(0.5 + 0.4 * cos(2. * 3.1415 * (i - NUM_STEPS/ 2.) / NUM_STEPS));
						val += texture(noise_tex, uvt).r;
						//norm += mask;

						//float magnitude = length(projected_frag - projected_coords);
						vec3 force = ((projected_frag - projected_coords));

						uvt += learning_rate / 10.0f * force.xy;
						//modified_coords += learning_rate * -;
						projected_frag += learning_rate / 10.0f * force;

					}
					alpha_new = 1.2 * (val / NUM_STEPS);
					//float theta = atan(projected_coords.y, projected_coords.x);
					//float norm_theta = max(theta / PI + 1, 0);
					//vec2 index = vec2(theta / (2.0f * PI),  distance / extent - .01);
					//if (index.y > 0)
						//alpha_new = texture(noise_tex, index).r;
					//else {
					//	alpha_new = 0;
					//}
				}
			}
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
	//Calculate color

	// combine results

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
	// attenuation
	//float distance = length(light.position - fragPos);
	//float attenuation = 1.0 / (light.constant + light.linear * distance +
	//	light.quadratic * (distance * distance));
	// combine results

	vec3 ambient = light.color * color;
	vec3 diffuse = light.color * diff * color;
	vec3 specular = light.color * spec * vec3(1, 1, 1);
	//ambient *= attenuation;
	//diffuse *= attenuation;
	//specular *= attenuation;
	return (ambient * ambient_coeff + diffuse * diffuse_coeff + specular * spec_coeff);
}

void main()
{
	vec3 norm = normalize(texture(normal_tex, TexCoord).rgb);
	//tangent = normalize(texture(tangent_tex, TexCoord).rgb);
	//bitangent = normalize(cross(norm, tangent));
	vec4 FragPos4 = texture(fragPos_tex, TexCoord).rgba;
	float mask = FragPos4.a;
	if (mask < 1)
		discard;
	vec3 FragPos = FragPos4.rgb;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 ret = vec3(0, 0, 0);
	vec3 color = calculateColor(FragPos, norm);


	for (int i = 0; i < num_point_lights; i++) {
		ret += CalcPointLight(pointLights[i], norm, FragPos, viewDir, color);
	}for (int i = 0; i < NR_DIR_LIGHTS; i++) {
		//color += CalcDirLight(dirLights[i], norm, FragPos, viewDir);
	}
	FragColor = vec4(ret, 1.0f);
}