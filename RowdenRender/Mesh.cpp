#include "Mesh.h"

Mesh::Mesh() {
	vertices = std::vector<float>();
	indices = std::vector<unsigned int>();
}


Mesh::Mesh(std::vector<Shape *> shapes) {
	Mesh();
	for (auto shape : shapes) {
		for (auto vertex : shape->getVertices()) {
			vertices.emplace_back(vertex.x);
			vertices.emplace_back(vertex.y);
			vertices.emplace_back(vertex.z);
		}
		for (auto index : shape->getIndices()) {
			indices.emplace_back(index.x);
			indices.emplace_back(index.y);
			indices.emplace_back(index.z);
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
	for (auto index : shape->getIndices()) {
		indices.emplace_back(index.x);
		indices.emplace_back(index.y);
		indices.emplace_back(index.z);
	}
}

Mesh::Mesh(std::vector<glm::vec3> _vertices, std::vector<glm::ivec3> _indices) {
	Mesh();
	for (auto vertex : _vertices) {
		vertices.emplace_back(vertex.x);
		vertices.emplace_back(vertex.y);
		vertices.emplace_back(vertex.z);
	}
	for (auto index : _indices) {
		indices.emplace_back(index.x);
		indices.emplace_back(index.y);
		indices.emplace_back(index.z);
	}
}

void Mesh::SetData(GLenum usage) {
	glGenVertexArrays(1, &VertexArrayObject);
	glGenBuffers(1, &VertexBufferObject);
	glGenBuffers(1, &IndexBufferArray);

	glBindVertexArray(VertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), (void*)vertices.data(), usage);

	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferArray);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], usage);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::Render() {
	glBindVertexArray(VertexArrayObject);
	glDrawElements(GL_TRIANGLES, indices.size() , GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}