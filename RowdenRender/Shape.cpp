#include "Shape.h"

Shape::Shape() {
	vertices = std::vector<glm::vec3>();
	indices = std::vector<glm::ivec3>();
	texCoords = std::vector<glm::vec2>();
}

Shape::Shape(const Shape &shape) {
	Shape(shape.getVertices(), shape.getIndices(), shape.getTexCoords());
}

Shape::Shape(Shape::PREMADE premade) {
	switch (premade) {
	case Shape::PREMADE::CUBE:
		//Front
		addVertex(-1, 1, 1);
		addVertex(-1, -1, 1);
		addVertex(1, -1, 1);
		addVertex(1, 1, 1);
		//Left
		addVertex(-1, 1, -1);
		addVertex(-1, -1, -1);
		addVertex(-1, -1, 1);
		addVertex(-1, 1, 1);
		//Right
		addVertex(1, 1, 1);
		addVertex(1, -1, 1);
		addVertex(1, -1, -1);
		addVertex(1, 1, -1);
		//Top
		addVertex(-1, 1, -1);
		addVertex(-1, 1, 1);
		addVertex(1, 1, 1);
		addVertex(1, 1, -1);
		//Bottom
		addVertex(-1, -1, 1);
		addVertex(-1, -1, -1);
		addVertex(1, -1, -1);
		addVertex(1, -1, 1);
		//Back
		addVertex(1, 1, -1);
		addVertex(1, -1, -1);
		addVertex(-1, -1, -1);
		addVertex(-1, 1, -1);
		
		addUniqueIndices();

		//Front
		addTexCoord(.25, .25);
		addTexCoord(.25, .5);
		addTexCoord(.5, .5);
		addTexCoord(.5, .25);
		//Left
		addTexCoord(0, .25);
		addTexCoord(0, .5);
		addTexCoord(.25, .5);
		addTexCoord(.25, .25);
		//Right
		addTexCoord(.5, .25);
		addTexCoord(.5, .5);
		addTexCoord(.75, .5);
		addTexCoord(.75, .25);
		//Top
		addTexCoord(.25, 0);
		addTexCoord(.25, .25);
		addTexCoord(.5, .25);
		addTexCoord(.5, 0);
		//Bottom
		addTexCoord(.25, .5);
		addTexCoord(.25, .75);
		addTexCoord(.5, .75);
		addTexCoord(.5, .5);
		//Back
		addTexCoord(.75, .25);
		addTexCoord(.75, .5);
		addTexCoord(1.0, .5);
		addTexCoord(1.0, .25);

		break;
	case Shape::PREMADE::QUAD:
		addVertex(glm::vec3(.5, .5, 0));
		addVertex(glm::vec3(.5, -.5, 0));
		addVertex(glm::vec3(-.5, -.5, 0));
		addVertex(glm::vec3(-.5, .5, 0));

		addIndex(glm::ivec3(0, 1, 3));
		addIndex(glm::ivec3(1, 2, 3));

		addTexCoord(glm::vec2(1, 1));
		addTexCoord(glm::vec2(1, 0));
		addTexCoord(glm::vec2(0, 0));
		addTexCoord(glm::vec2(0, 1));
	}
}

Shape::Shape(std::vector<glm::vec3> _vertices, std::vector<glm::ivec3> _indices, std::vector<glm::vec2> _texCoords) {
	vertices = std::vector<glm::vec3>(_vertices);
	indices = std::vector<glm::ivec3>(_indices);
	texCoords = std::vector<glm::vec2>(_texCoords);
}

void Shape::addVertex(glm::vec3 vertex) {
	vertices.emplace_back(vertex);
}

void Shape::addIndex(glm::ivec3 index) {
	indices.emplace_back(index);
}

void Shape::addTexCoord(glm::vec2 texCoord) {
	texCoords.emplace_back(texCoord);
}

void Shape::addVertex(float x, float y, float z) {
	addVertex(glm::vec3(x, y, z));
}

void Shape::addIndex(int x, int y, int z) {
	addIndex(glm::ivec3(x, y, z));
}

void Shape::addTexCoord(float x, float y) {
	addTexCoord(glm::vec2(x, y));
}

std::vector<glm::vec3> Shape::getVertices() const{
	return vertices;
}

std::vector<glm::ivec3> Shape::getIndices() const {
	return indices;
}

std::vector<glm::vec2> Shape::getTexCoords() const {
	return texCoords;
}

void Shape::addUniqueIndices() {
	for (int i = 0; i < vertices.size(); i+=4) {
		indices.emplace_back(i, i+1, i+2);
		indices.emplace_back(i + 2, i + 3, i);
	}
}
