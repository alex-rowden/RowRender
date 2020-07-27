#pragma once
#include "RowRender.h"
class Shape;
class Lights;
class Texture2D;
class ShaderProgram;

class Mesh
{
public:
	Mesh();
	Mesh(Shape* shape);
	Mesh(std::vector<Shape*> shapes);
	Mesh(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> _indices);
	void addTexture(Texture2D texture);
	void SetUniformColor(glm::vec4 color);
	void SetColors(std::vector<glm::vec4> colors);
	void SetData(GLenum usage = GL_STATIC_DRAW);
	void setTexture(Texture2D tex, int index);
	void SetAsLight() { is_light = true; }
	void SetInstanceTransforms(std::vector<glm::mat4>);
	void SetInstanceTransforms(std::vector<glm::mat4>, std::vector<float>);
	
	void Render(ShaderProgram* shader, int);
	void Render(ShaderProgram* shader) { Render(shader, 0); };
	bool isLight() { return is_light; }

	std::vector<float> getVerticies() { return verticies; }
	std::vector<float> getNormals() { return normals;}
	std::vector<float> getTexCoords() { return texCoords;}
	std::vector<Texture2D> getTextures() { return textures; };
	std::vector<unsigned int> getIndices() { return indices; }
private:
	bool first_render = true;
	bool is_light = false;
	int num_instances = 0;
	std::vector<float> verticies;
	std::vector<unsigned int> indices;
	std::vector<float> colors;
	std::vector<float> normals;
	std::vector<float> texCoords;
	std::vector<Texture2D> textures;
	unsigned int VertexBufferObject, VertexArrayObject, IndexBufferArray, NormalBuffer, TexCoordBuffer, matrixBuffer, colorBuffer;
};

