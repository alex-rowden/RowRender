#pragma once
#include "RowRender.h"
class RouterMath
{
public:
	RouterMath(glm::mat4 transform = glm::mat4(1),
		glm::vec3 stretch = glm::vec3(0),
		float ext = 1) :
		ellipsoid_transform{transform}, 
		radius_stretch{stretch},
		extent{ ext }{};
	bool intersect(Ellipsoid a, Ellipsoid b);
	bool inside(glm::vec3 point, Ellipsoid a);
	bool inside(Ellipsoid a, glm::vec3 point) { return inside(point, a); };
	bool outside(glm::vec3 point, Ellipsoid a) { return !inside(point, a); };
	bool outside(Ellipsoid a, glm::vec3 point) { return !inside(point, a); };
	glm::vec3 ellipsoidCoordinates(glm::vec3 point, Ellipsoid a);
	bool ellipsoidDistance(glm::vec3 point, Ellipsoid a);
private:
	glm::mat4 ellipsoid_transform;
	glm::vec3 radius_stretch;
	float extent;

};

