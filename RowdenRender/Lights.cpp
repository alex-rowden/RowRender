#include "Lights.h"

void Lights::addPointLight(PointLight p) {
	point_lights.emplace_back(p);
}

void Lights::addPointLight(glm::vec3 pos, glm::vec3 color) {
	addPointLight(pos, 1, 0, 0, color, color, glm::vec3(0, 0, 0));
}
void Lights::addPointLight(glm::vec3 pos, float intensity, glm::vec3 color) {
	addPointLight(pos, intensity, 0, 0, color, color, glm::vec3(1,1,1));
}

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

void Lights::addDirLight(glm::vec3 dir, glm::vec3 color)
{
	DirLight dl = {
		dir,
		color
	};
	dir_lights.emplace_back(dl);
}
