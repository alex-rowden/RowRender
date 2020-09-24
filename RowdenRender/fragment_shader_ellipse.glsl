#version 420 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 Tangent;

const float PI = 3.1415926535897932384626433832795;

uniform sampler2D texture_diffuse1;
uniform sampler2D wifi_colors;

layout(location = 0) out vec4 normal_tex;
layout(location = 1) out vec4 albedo_tex;
layout(location = 2) out vec4 fragPos_tex;
layout(location = 3) out int freq_mask_tex;


uniform sampler2D ellipsoid_tex;
uniform sampler2D frequency_tex;
uniform sampler2D text_tex;
uniform float transparency, u_stretch, v_stretch,
linear_term, thickness, distance_mask, delta_theta;
uniform int num_contours;
uniform int num_routers, num_point_lights, num_freqs;
uniform bool bin_orientations, display_names;

#define MAX_ROUTERS 120

bool freq_used[12] = {
	false, false, false, false, false, false,
	false, false, false, false, false, false
};

struct Ellipsoid {
	vec4 mu;
	vec4 r;
	mat4 axes;
	
};


//uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform vec3 viewPos;
vec3 bitangent, tangent;
layout(std140) uniform EllipsoidBlock{
	Ellipsoid Ellipsoids[MAX_ROUTERS];
};
uniform mat4 ellipsoid_transform;
uniform float radius, frequency, extent, theta;
uniform vec3 radius_stretch;

vec2 rotateVector(float theta, vec2 inp) {
	float rad = radians(theta);
	float cosTheta = cos(rad);
	float sinTheta = sin(rad);

	return vec2((cosTheta * inp.x - sinTheta * inp.y) * u_stretch, (sinTheta * inp.x + cosTheta * inp.y) * v_stretch);
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

vec3 calculateColor(vec3 fragPos) {
	vec3 color = texture(texture_diffuse1, TexCoord).rgb;
	
	vec3 ret = vec3(0);
	float alpha = 1;
	for (int i = 0; i < num_routers; i++) {
		float distance = ellipsoidDistance(fragPos, Ellipsoids[i]);
		if (distance <= extent) {
			float alpha_new = 1 - distance/extent * transparency;
			
			//if (abs(dot(Normal, vec3(1,0,0)))> 1e-4)
			//	mask = texture(crosshair_tex, vec2(distance / extent * frequency, radius * atan(modified_coords.z / modified_coords.y))).r;
			//if (abs(dot(Normal, vec3(0, 1, 0))) > 1e-4)
			//	mask = texture(crosshair_tex, vec2(distance / extent * frequency, radius * atan(modified_coords.x / modified_coords.z))).r;

			vec3 color = texture(wifi_colors, vec2(0, Ellipsoids[i].mu.w)).rgb;
			if ((dot(Normal, vec3(0, 0, 1))) > 1e-6) {
				if (bin_orientations && freq_used[int(Ellipsoids[i].r.w)]) {
					continue;
				}
				else {
					freq_used[int(Ellipsoids[i].r.w)] = true;
				}
				freq_mask_tex += int(pow(ceil(num_routers), int(Ellipsoids[i].r.w)));
				vec3 modified_coords = ellipsoidCoordinates(fragPos, Ellipsoids[i]);

				vec2 index = (vec2(dot(tangent, modified_coords), dot(bitangent, modified_coords)) + vec2(1)) / 2.0f;
				index = rotateVector(theta + Ellipsoids[i].r.w * delta_theta, index);
				//float mask = texture(crosshair_tex, vec2(distance / extent * frequency, radius * atan(modified_coords.x / modified_coords.y))).r;
				float mask = texture(frequency_tex, index).r;
				alpha_new = mask;
				
			}
			else if (-dot(Normal, vec3(0, 0, 1)) > 1e-6) {
				alpha_new = 0;
			}
			else if (mod(linear_term * log2(distance / extent), frequency) >  frequency *  thickness) {
				alpha_new = 0;
			}
			else {
				if (abs(mod(linear_term * log2(distance / extent), frequency) / (frequency * thickness) - .5) > .2) {
					alpha_new = 0;
				}
				if (display_names) {
					vec3 modified_coords = ellipsoidCoordinates(fragPos, Ellipsoids[i]);
					vec2 projected_coords  = vec2(dot(Tangent, modified_coords), dot(bitangent, modified_coords));
					float theta = atan(projected_coords.y, projected_coords.x);
					float norm_theta = max(theta / PI + 1, 0);
					if (mod(norm_theta * num_contours, 2) < 1) {
						vec2 index = vec2(1 - mod(norm_theta * num_contours, 2), mod(linear_term * log2(distance / extent), frequency) / (frequency * thickness));
						index.y = index.y / num_routers + i / float(num_routers);
						alpha_new = texture(text_tex, index).r;
					}
				}
			}
			ret = alpha_new * (alpha * color) + ret;
			alpha = (1 - alpha_new) * alpha;
		} 
	}
	
	ret = alpha * color + ret;
	
	return ret;
}

void main()
{
	if (distance(FragPos, viewPos) < distance_mask) {
		discard;
	}
	vec3 norm = normalize(Normal);
	tangent = normalize(Tangent);
	bitangent = normalize(cross(norm, tangent));

	albedo_tex = vec4(calculateColor(FragPos), 1);
	fragPos_tex = vec4(FragPos, 1);
	normal_tex = vec4(norm, 1);
}