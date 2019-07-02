#pragma once
#include <string>
#include "RowRender.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <SOIL.h>
class Model
{
private:
	void loadModel(std::string path);
	std::vector<Mesh*> meshes;
	std::string directory;
	void processNode(aiNode* node, const aiScene* scene);
	Mesh *processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture2D> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	std::vector<Texture2D> loadEmbeddedTextures(const aiScene* scene, const std::string& path);
public:
	Model(const char* path) { loadModel(std::string(path)); }
	Model() {};
	void Render(ShaderProgram* sp);
	void addMesh(Mesh *mesh);
};

