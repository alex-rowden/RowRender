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
	void setupEmpty();
	Mesh(Shape* shape);
	Mesh(std::vector<Shape*> shapes);
	Mesh(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> _indices);
	void addTexture(Texture2D*texture);
	void SetUniformColor(glm::vec4 color);
	void SetColors(std::vector<glm::vec4> colors);
	void SetData(GLenum usage = GL_STATIC_DRAW, bool uses_tangents = false);
	void setTexture(Texture2D*tex, int index);
	void setTexture(Texture2D*texture);
	void SetAsLight() { is_light = true; }
	void SetInstanceTransforms(std::vector<glm::mat4>);
	void SetInstanceTransforms(std::vector<glm::mat4>, std::vector<float>);
	
	void Render(ShaderProgram* shader, int, std::vector<Texture2D*> = std::vector<Texture2D *>());
	void Render(ShaderProgram* shader) { Render(shader, 0); };
	bool isLight() { return is_light; }

	std::vector<float> getVerticies() { return verticies; }
	std::vector<float> getNormals() { return normals;}
	std::vector<float> getTexCoords() { return texCoords;}
	std::vector<Texture2D*> getTextures() { return textures; };
	std::vector<unsigned int> getIndices() { return indices; }
	glm::vec3 getBBoxMin() { return bbox_min; };
	glm::vec3 getBBoxMax() { return bbox_max; };
	void setBBoxMin(glm::vec3 a) { bbox_min = a; }
	void setBBoxMax(glm::vec3 a) { bbox_max = a; }
private:
	bool first_render = true;
	bool is_light = false;
	int num_instances = 0;
	std::vector<float> verticies;
	std::vector<unsigned int> indices;
	std::vector<float> colors;
	std::vector<float> normals;
	std::vector<float> texCoords;
	std::vector<Texture2D*> textures;
	std::vector<float> tangents;
	glm::vec3 bbox_min, bbox_max;
	unsigned int VertexBufferObject, VertexArrayObject, IndexBufferArray, NormalBuffer, TexCoordBuffer, matrixBuffer, colorBuffer, tangentBufferArray;
};

