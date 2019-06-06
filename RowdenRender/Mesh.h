#pragma once
#include "Shape.h"

class Mesh
{
public:
	Mesh();
	Mesh(std::vector<Shape> shapes);
	Mesh(std::vector<glm::vec3> vertices);
	void SetData(GLenum usage = GL_STATIC_DRAW);
private:
	std::vector<glm::vec3> vertices;
	unsigned int VertexBufferObject;
};

