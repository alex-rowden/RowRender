#pragma once
#include "Shape.h"
#include "Texture2D.h"

class Mesh
{
public:
	Mesh();
	Mesh(Shape *shape);
	Mesh(std::vector<Shape *> shapes);
	Mesh(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> _indices);
	void addTexture(Texture2D texture);
	void SetUniformColor(glm::vec4 color);
	void SetColors(std::vector<glm::vec4> colors);
	void SetData(GLenum usage = GL_STATIC_DRAW);
	void SetAsLight() { is_light = true; }
	void Render();
	bool isLight() { return is_light; }
private:
	bool is_light = false;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> colors;
	std::vector<float> normals;
	std::vector<float> texCoords;
	std::vector<Texture2D> textures;
	unsigned int VertexBufferObject, VertexArrayObject, IndexBufferArray, NormalBuffer, TexCoordBuffer;
};

