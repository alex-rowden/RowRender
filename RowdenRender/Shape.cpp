#include "Shape.h"

Shape::Shape() {
	vertices = std::vector<glm::vec3>();
}

Shape::Shape(const Shape &shape) {
	Shape(shape.getVertices());
}

Shape::Shape(std::vector<glm::vec3> _vertices) {
	vertices = std::vector<glm::vec3>(_vertices);
}

void Shape::addVertex(glm::vec3 vertex) {
	vertices.emplace_back(vertex);
}

std::vector<glm::vec3> Shape::getVertices() const{
	return vertices;
}
