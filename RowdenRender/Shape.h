#pragma once
#include "RowRender.h"

class Shape
{
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> indices;
public:
	Shape();
	Shape(const Shape &shape);
	Shape(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> indices, std::vector<glm::vec2> texCoords);
	void addVertex(glm::vec3 vertex);
	void addIndex(glm::ivec3 indices);
	void addTexCoord(glm::vec2 texCoord);
	std::vector<glm::vec3> getVertices() const;
	std::vector<glm::ivec3> getIndices() const;
};

