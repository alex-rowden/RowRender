#include "Shape.h"

Shape::Shape() {
	vertices = std::vector<glm::vec3>();
	indices = std::vector<glm::ivec3>();
}

Shape::Shape(const Shape &shape) {
	Shape(shape.getVertices(), shape.getIndices(), shape.getTexCoords());
}

Shape::Shape(std::vector<glm::vec3> _vertices, std::vector<glm::ivec3> _indices) {
	vertices = std::vector<glm::vec3>(_vertices);
	indices = std::vector<glm::ivec3>(_indices);
}

void Shape::addVertex(glm::vec3 vertex) {
	vertices.emplace_back(vertex);
}

void Shape::addIndex(glm::ivec3 index) {
	indices.emplace_back(index);
}

std::vector<glm::vec2> Shape::getTexCoords() const {
	return texCoords;
}

std::vector<glm::vec3> Shape::getVertices() const{
	return vertices;
}

std::vector<glm::ivec3> Shape::getIndices() const {
	return indices;
}
