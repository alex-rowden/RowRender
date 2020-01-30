#include "Model.h"

void Model::Render(ShaderProgram *sp) {
	sp->Use();
	for (auto mesh : meshes) {
		mesh->Render(sp);
	}
}

void Model::addMesh(Mesh* mesh) {
	meshes.emplace_back(mesh);
}
void Model::addModel(Model* model) {
	for (auto mesh : model->getMeshes()) {
		meshes.emplace_back(mesh);
	}
}

void Model::loadModel(std::string path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('\\'));

	processNode(scene->mRootNode, scene);
}

void Model::setModel() {
	for (auto mesh : meshes) {
		mesh->SetData();
	}
}

void Model::processNode(aiNode *node, const aiScene *scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	Shape shape = Shape();
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		shape.addVertex(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
		shape.addNormal(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
		if (mesh->mTextureCoords[0]) {
			shape.addTexCoord(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
		}
		else {
			shape.addTexCoord(glm::vec2(0, 0));
		}
	}
	for (unsigned int i = 0; i < mesh->mNumFaces; i+=1) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j+=3) {
			shape.addIndex(face.mIndices[j], face.mIndices[j+1], face.mIndices[j+2]);
		}
	}
	Mesh* ret = new Mesh(&shape);
	if (mesh->mMaterialIndex >= 0) {
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture2D> diffuseMaps = loadMaterialTextures(material,
				aiTextureType_DIFFUSE, "texture_diffuse");
			for (auto texture : diffuseMaps) {
				ret->addTexture(texture);
			}
			std::vector<Texture2D> specularMaps = loadMaterialTextures(material,
				aiTextureType_SPECULAR, "texture_specular");
			for (auto texture : specularMaps)
				ret->addTexture(texture);
		}
	}
	for (auto texture : loadEmbeddedTextures(scene, directory)) {
		ret->addTexture(texture);
	}
	return ret;
}

std::vector<Texture2D> Model::loadEmbeddedTextures(const aiScene* scene, const std::string& path) {
	// Check if scene has textures.
	std::vector<Texture2D> textures;
	if (scene->HasTextures())
	{
		for (size_t ti = 0; ti < scene->mNumTextures; ti++)
		{
			textures.emplace_back(Texture2D(scene->mTextures[ti]));
		}
	}
	return textures;
}

std::vector<Texture2D > Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<Texture2D> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		if (str.data[0] == '*') {
			std::cout << "Embedded Texture" << std::endl;
		}
		Texture2D texture = Texture2D((directory + "\\" + std::string( str.C_Str())).c_str());
		if (type == aiTextureType_SPECULAR)
			texture.name = "texture_specular";
		textures.emplace_back(texture);
	}
	if (textures.size() == 0) {
		aiColor4D color(0, 0, 0, 0);
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		textures.emplace_back(Texture2D(glm::vec4(color.r, color.g, color.b, color.a)));
	}


	return textures;
}