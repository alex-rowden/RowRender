#include "Mesh.h"

Mesh::Mesh() {
	vertices = std::vector<float>();
	
}


Mesh::Mesh(std::vector<Shape *> shapes) {
	Mesh();
	for (auto shape : shapes) {
		for (auto vertex : shape->getVertices()) {
			vertices.emplace_back(vertex.x);
			vertices.emplace_back(vertex.y);
			vertices.emplace_back(vertex.z);
		}
	}
}
Mesh::Mesh(Shape *shape) {
	Mesh();
	for (auto vertex : shape->getVertices()) {
		vertices.emplace_back(vertex.x);
		vertices.emplace_back(vertex.y);
		vertices.emplace_back(vertex.z);
	}
}

Mesh::Mesh(std::vector<glm::vec3> _vertices) {
	Mesh();
	for (auto vertex : _vertices) {
		vertices.emplace_back(vertex.x);
		vertices.emplace_back(vertex.y);
		vertices.emplace_back(vertex.z);
	}
}

void Mesh::SetData(GLenum usage) {
	glGenVertexArrays(1, &VertexArrayObject);
	glGenBuffers(1, &VertexBufferObject);
	glBindVertexArray(VertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), (void *)vertices.data(), usage);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::Render() {
	glBindVertexArray(VertexArrayObject);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}