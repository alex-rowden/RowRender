#version 430

#define MAX_ROUTERS 20

out vec4 Force;

in vec2 TexCoord;

uniform sampler2D normal_tex;
uniform sampler2D tangent_tex;
uniform sampler2D fragPos_tex;
uniform int ellipsoid_index_offset, num_ellipsoids;

uniform mat4 ellipsoid_transform;
uniform vec3 radius_stretch;
uniform float extent;

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

	float ellipsoid_dist = ellipsoidDistance(fragPos, modified_coords, ellipsoid);

	if (ellipsoid_dist / extent < .01) {
		return vec4(vec3(0), 0);
	}
	vec3 ellipsoid_coords = fragPos - modified_coords;
	if (ellipsoid_dist < extent) {
		vec3 force = (fragPos - ellipsoid_coords);
		vec3 direction = force / (length(force));
		direction = worldToWallCoords * direction;
		direction.z = 0;
		direction = wallToWorldCoords * direction;
		return vec4(direction, 1);
	}
	return vec4(vec3(0), -1);

}


void main()
{
	vec3 force = vec3(0);
	bool found = true;
	vec3 fragPos = texture(fragPos_tex, TexCoord).xyz;
	vec3 normal = texture(normal_tex, TexCoord).xyz;
	vec3 tangent = texture(tangent_tex, TexCoord).xyz;
	vec3 bitangent = cross(normal, tangent);
	mat3 wallToWorldCoords = mat3(tangent, bitangent, normal);
	mat3 worldToWallCoords = transpose(wallToWorldCoords);

	Ellipsoid ellipsoid;
	for(int i = 0; i < num_ellipsoids; i++){
		ellipsoid = Ellipsoids[i + ellipsoid_index_offset];
		vec4 direction = calculateForce(fragPos, ellipsoid, worldToWallCoords, wallToWorldCoords);
		if(direction.a > 0){
			force += direction.xyz;
			found = true;
		}
		if(length(direction) == 0){
			force = vec3(0);
			break;
		}
	}
	if(!found)
		Force = vec4(-1);
	else
		Force = vec4(force, 1);
}