#pragma once
#include "RowRender.h"

class Shape
{
private:
	std::vector<glm::vec3> vertices;
public:
	Shape();
	Shape(const Shape &shape);
	Shape(std::vector<glm::vec3>);
	void addVertex(glm::vec3 vertex);
	std::vector<glm::vec3> getVertices() const;
};

