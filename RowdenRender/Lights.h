#pragma once

#include "RowRender.h"


class Lights
{
public:
	struct PointLight {
		glm::vec3 position;

		float constant;
		float linear;
		float quadratic;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
	};
	void addPointLight(glm::vec3 pos, float constant, float linear, float quadratic, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
	std::vector<PointLight> getPointLights() { return point_lights; }
private:
	std::vector<PointLight> point_lights;
};

