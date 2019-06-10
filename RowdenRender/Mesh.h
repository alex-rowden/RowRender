#pragma once
#include "Shape.h"

class Mesh
{
public:
	Mesh();
	Mesh(Shape *shape);
	Mesh(std::vector<Shape *> shapes);
	Mesh(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> _indices);
	void SetUniformColor(glm::vec4 color);
	void SetColors(std::vector<glm::vec4> colors);
	void SetData(GLenum usage = GL_STATIC_DRAW);
	void Render();
private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> colors;
	unsigned int VertexBufferObject, VertexArrayObject, IndexBufferArray, ColorBufferArray;
};

