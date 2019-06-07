#pragma once
#include "Shape.h"

class Mesh
{
public:
	Mesh();
	Mesh(Shape *shape);
	Mesh(std::vector<Shape *> shapes);
	Mesh(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> _indices);
	void SetData(GLenum usage = GL_STATIC_DRAW);
	void Render();
private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	unsigned int VertexBufferObject, VertexArrayObject, IndexBufferArray;
};

