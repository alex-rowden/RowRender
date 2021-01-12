#pragma once
#include <string>
#include "RowRender.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <SOIL.h>

class Mesh;
class ShaderProgram;
class Texture2D;

class Model
{
private:
	void loadModel(std::string path, bool import_tangents = false);
	std::vector<Mesh*> meshes;
	std::string directory;
	void processNode(aiNode* node, const aiScene* scene, bool import_tangents);
	Mesh *processMesh(aiMesh* mesh, const aiScene* scene, bool import_tangents);
	std::vector<Texture2D> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	std::vector<Texture2D> loadEmbeddedTextures(const aiScene* scene, const std::string& path);
	
public:
	Model(const char* path, bool use_tangents = false) { loadModel(std::string(path), use_tangents); };
	Model(std::string string, bool use_tangents = false) { loadModel(string, use_tangents); }
	Model() {};
	void Render(ShaderProgram* sp, glm::mat4, glm::mat4, glm::mat4);
	void Render(ShaderProgram* sp, std::vector<Mesh*> meshes);
	void Render(ShaderProgram* sp);
	void addMesh(Mesh *mesh);
	void addModel(Model* model);
	void setModel(bool use_tangents = false);
	std::vector<Mesh*> getMeshes() { return meshes; }
};



