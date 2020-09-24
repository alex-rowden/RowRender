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
	};struct DirLight {
		glm::vec3 direction;

		glm::vec3 color;
	};
	void addPointLight(PointLight p);
	void addPointLight(glm::vec3 pos, glm::vec3 color);
	void addPointLight(glm::vec3 pos, float intensity, glm::vec3 color);
	void addPointLight(glm::vec3 pos, float constant, float linear, float quadratic, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
	void addDirLight(glm::vec3 dir, glm::vec3 color);
	std::vector<PointLight> getPointLights() { return point_lights; }
	std::vector<DirLight> getDirLights() { return dir_lights; }
	std::vector<PointLight> point_lights;
private:
	
	std::vector<DirLight> dir_lights;
};

