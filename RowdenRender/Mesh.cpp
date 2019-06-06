#include "Mesh.h"

Mesh::Mesh() {
	vertices = std::vector<glm::vec3>();
	glGenBuffers(1, &VertexBufferObject);
}

Mesh::Mesh(std::vector<Shape> shapes) {
	Mesh();
	for (auto shape : shapes) {
		vertices.emplace_back(shape.getVertices());
	}
}

Mesh::Mesh(std::vector<glm::vec3> _vertices) {
	Mesh();
	vertices.emplace_back(_vertices);
}

void Mesh::SetData(GLenum usage) {
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}