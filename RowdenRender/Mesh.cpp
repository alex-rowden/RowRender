#include "Mesh.h"

Mesh::Mesh() {
	vertices = std::vector<glm::vec3>();
	glGenBuffers(1, &VertexBufferObject);
}

Mesh::Mesh(Shape shape) {
	std::vector<Shape> shapes = std::vector<Shape>();
	shapes.emplace_back(shape);
	Mesh();
}
Mesh::Mesh(std::vector<Shape> shapes) {
	Mesh();
	for (auto shape : shapes) {
		for (auto vertex : shape.getVertices()) {
			vertices.emplace_back(vertex);
		}
	}
}

Mesh::Mesh(std::vector<glm::vec3> _vertices) {
	Mesh();
	for (auto vertex: _vertices)
		vertices.emplace_back(vertex);
}

void Mesh::SetData(GLenum usage) {
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}