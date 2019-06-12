#pragma once
#include "RowRender.h"

class Shape
{
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> indices;
	std::vector<glm::vec2> texCoords;
public:
	enum class PREMADE { CUBE, QUAD };
	Shape();
	Shape(Shape::PREMADE premade);
	Shape(const Shape &shape);
	Shape(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> indices, std::vector<glm::vec2> texCoords);
	void addVertex(glm::vec3 vertex);
	void addIndex(glm::ivec3 indices);
	void addTexCoord(glm::vec2 texCoord);
	void addVertex(float x, float y, float z);
	void addIndex(int x, int y, int z);
	void addTexCoord(float x, float y);
	void addUniqueIndices();
	std::vector<glm::vec3> getVertices() const;
	std::vector<glm::ivec3> getIndices() const;
	std::vector<glm::vec2> getTexCoords() const;
	
};

