#version 430

#define MAX_ROUTERS 20

layout(location = 0) out vec4 Force;
layout(location = 1) out vec4 lic_color;

in vec2 TexCoord;

uniform sampler2D normal_tex;
uniform sampler2D tangent_tex;
uniform sampler2D fragPos_tex;
uniform sampler2D wifi_colors;
uniform int ellipsoid_index_offset, num_ellipsoids, num_routers;

uniform mat4 ellipsoid_transform;
uniform vec3 radius_stretch;
uniform float extent;
uniform bool invert_colors;

struct Ellipsoid {
	vec4 mu;
	vec4 r;
	mat4 axes;

};

layout(std140) uniform EllipsoidBlock{
	Ellipsoid Ellipsoids[MAX_ROUTERS];
};

vec3 ellipsoidCoordinates(vec3 fragPos, Ellipsoid ellipsoid) {
	vec3 modified_coords = ellipsoid.mu.xyz;
	modified_coords = (ellipsoid_transform * vec4(modified_coords.xyz, 1)).xyz;
	modified_coords = (fragPos - modified_coords);

	return modified_coords;
}

float ellipsoidDistance(vec3 fragPos, vec3 ellipsoid_coords, Ellipsoid ellipsoid) {
	vec3 modified_coords = mat3(ellipsoid.axes) * ellipsoid_coords;
	modified_coords = (modified_coords * modified_coords) / (9 * abs(mat3(ellipsoid.axes) * abs(radius_stretch * ellipsoid.r.xyz)));

	float dist = 0;
	for (int i = 0; i < 3; i++) {
		dist += modified_coords[i];
	}
	return sqrt(dist);
}

float ellipsoidDistance(vec3 fragPos, Ellipsoid ellipsoid) {
	return ellipsoidDistance(fragPos, ellipsoidCoordinates(fragPos, ellipsoid), ellipsoid);
}


vec4 calculateForce(vec3 fragPos, Ellipsoid ellipsoid, 
					mat3 worldToWallCoords, mat3 wallToWorldCoords) {
	
	vec3 modified_coords = ellipsoidCoordinates(fragPos, ellipsoid);

	float ellipsoid_dist = ellipsoidDistance(fragPos, ellipsoid);

	if (ellipsoid_dist / extent < .01 - .001) {
		return vec4(vec3(0), ellipsoid_dist/extent);
	}
	vec3 ellipsoid_coords = fragPos - modified_coords;
	if (ellipsoid_dist < extent) {
		vec3 force = (fragPos - ellipsoid_coords);
		vec3 direction = force / (length(force));
		direction = worldToWallCoords * direction;
		direction.z = 0;
		direction = wallToWorldCoords * direction;
		return vec4(direction, ellipsoid_dist/extent);
	}
	return vec4(vec3(0), -ellipsoid_dist/extent);

}


void main()
{
	vec3 force = vec3(0);
	bool found = false;
	vec3 fragPos = texture(fragPos_tex, TexCoord).xyz;
	vec3 normal = texture(normal_tex, TexCoord).xyz;
	vec3 tangent = texture(tangent_tex, TexCoord).xyz;
	vec3 bitangent = cross(normal, tangent);
	mat3 wallToWorldCoords = mat3(tangent, bitangent, normal);
	mat3 worldToWallCoords = transpose(wallToWorldCoords);
	float min_alpha = 1000;
	float max_strength = 1000;
	vec3 color = vec3(0);
	Ellipsoid ellipsoid;
	for(int i = 0; i < num_ellipsoids; i++){
		ellipsoid = Ellipsoids[i + ellipsoid_index_offset];
		vec4 direction = calculateForce(fragPos, ellipsoid, worldToWallCoords, wallToWorldCoords);
		if(direction.a > 0){
			force += direction.xyz;
			min_alpha = min(direction.a, min_alpha);
			found = true;
		}
		if(length(direction.xyz) == 0){
			force = vec3(0);
		}if(abs(direction.a) < max_strength){
			max_strength = direction.a;
			float color_ind = ellipsoid.mu.w;
			if (invert_colors)
				color_ind = ellipsoid.r.w / float(num_routers);
			color = texture(wifi_colors, vec2(0, color_ind)).rgb;
		}
	}
	if(!found)
		Force = vec4(0);
	else
		Force = vec4(force, max(min_alpha, 0));
	lic_color = vec4(color, 1);
}