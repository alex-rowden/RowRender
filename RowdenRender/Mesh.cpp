#include "Mesh.h"

Mesh::Mesh() {
	vertices = std::vector<glm::vec3>();
	glGenVertexArrays(1, &VertexArrayObject);
	glGenBuffers(1, &VertexBufferObject);
}


Mesh::Mesh(std::vector<Shape *> shapes) {
	Mesh();
	for (auto shape : shapes) {
		for (auto vertex : shape->getVertices()) {
			vertices.emplace_back(vertex);
		}
	}
}
Mesh::Mesh(Shape *shape) {
	Mesh();
	for (auto vertex : shape->getVertices()) {
		vertices.emplace_back(vertex);
	}
}

Mesh::Mesh(std::vector<glm::vec3> _vertices) {
	Mesh();
	for (auto vertex: _vertices)
		vertices.emplace_back(vertex);
}

void Mesh::SetData(GLenum usage) {
	glBindVertexArray(VertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), (void *)&vertices, usage);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::Render() {
	glBindVertexArray(VertexArrayObject);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}