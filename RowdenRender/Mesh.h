#pragma once
#include "Shape.h"

class Mesh
{
public:
	Mesh();
	Mesh(Shape *shape);
	Mesh(std::vector<Shape *> shapes);
	Mesh(std::vector<glm::vec3> vertices);
	void SetData(GLenum usage = GL_STATIC_DRAW);
	void Render();
private:
	std::vector<float> vertices;
	unsigned int VertexBufferObject, VertexArrayObject;
};

