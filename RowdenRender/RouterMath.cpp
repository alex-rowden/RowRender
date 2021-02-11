#include "RouterMath.h"

glm::vec3 RouterMath::ellipsoidCoordinates(glm::vec3 point, Ellipsoid a) {
	glm::vec3 modified_coords = glm::vec3(a.mu);
	modified_coords = glm::vec3(ellipsoid_transform * glm::vec4(glm::vec3(modified_coords), 1));
	modified_coords = (point - modified_coords);

	return modified_coords;
}

bool RouterMath::ellipsoidDistance(glm::vec3 point, Ellipsoid a) {
	glm::vec3 modified_coords = glm::mat3(a.axis) * ellipsoidCoordinates(point, a);
	modified_coords = (modified_coords * modified_coords) / (9.0f * abs(glm::mat3(a.axis) * abs(radius_stretch * glm::vec3(a.r))));

	float distance = 0;
	for (int i = 0; i < 3; i++) {
		distance += modified_coords[i];
	}
	return sqrt(distance);
}

bool RouterMath::inside(glm::vec3 point, Ellipsoid a) {
	return ellipsoidDistance(point, a) < extent;
}

bool RouterMath::intersect(Ellipsoid a, Ellipsoid b) {
	if (inside(a, -ellipsoidCoordinates(glm::vec3(0), b)))
		return true;
	return false; //TODO: This is innacurate and isn't really a good heuristic either
}