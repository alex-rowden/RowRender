#include "Lights.h"

void Lights::addPointLight(glm::vec3 pos, float constant, float linear, float quadratic, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) {
	PointLight pl = {
		pos,
		constant,
		linear,
		quadratic,
		ambient,
		diffuse,
		specular
	};
	point_lights.emplace_back(pl);
}