#include "Model.h"
#include <glm/gtc/matrix_access.hpp>
/*
int FrustumAABBIntersect(Plane *planes, Vector &mins, Vector &maxs) {
   int    ret = INSIDE;
   Vector vmin, vmax;

   for(int i = 0; i < 6; ++i) {
	  // X axis
	  if(planes[i].normal.x > 0) {
		 vmin.x = mins.x;
		 vmax.x = maxs.x;
	  } else {
		 vmin.x = maxs.x;
		 vmax.x = mins.x;
	  }
	  // Y axis
	  if(planes[i].normal.y > 0) {
		 vmin.y = mins.y;
		 vmax.y = maxs.y;
	  } else {
		 vmin.y = maxs.y;
		 vmax.y = mins.y;
	  }
	  // Z axis
	  if(planes[i].normal.z > 0) {
		 vmin.z = mins.z;
		 vmax.z = maxs.z;
	  } else {
		 vmin.z = maxs.z;
		 vmax.z = mins.z;
	  }
	  if(Vector::DotProduct(planes[i].normal, vmin) + planes[i].d > 0)
		 return OUTSIDE;
	  if(Vector::DotProduct(planes[i].normal, vmax) + planes[i].d >= 0)
		 ret = INTERSECT;
   }
   return ret;
}
*/

bool frustum_cull(Mesh* m, const glm::vec4 p_planes[6]) {
	glm::vec3 dim, half_dim, midpoint;

	dim = m->getBBoxMax() - m->getBBoxMin();
	half_dim = dim / 2.0f;
	midpoint = m->getBBoxMin() + half_dim;


	for (int i = 0; i < 6; i++) {
		float p = abs(p_planes[i].x * half_dim.x) +
			abs(p_planes[i].y * half_dim.y) +
			abs(p_planes[i].z * half_dim.z);
		float d = dot(midpoint, glm::vec3(p_planes[i])) + p_planes[i][3];
		if (d <= -p) {
			return true;
		}
	}
	
	return false;
}

void Model::Render(ShaderProgram* sp) {
	Render(sp, meshes);
}

void Model::Render(ShaderProgram *sp, std::vector<Mesh*> render_meshes) {
	sp->Use();

	for (auto mesh : render_meshes) {
		mesh->Render(sp);
	}
}

void Model::Render(ShaderProgram* sp, glm::mat4 model, glm::mat4 view, glm::mat4 proj) {
	std::vector<Mesh*> render_meshes;
	auto comboMatrix = proj *(view * model);
	glm::vec4 p_planes[6];
	
	p_planes[0] = glm::row(comboMatrix, 3) + glm::row(comboMatrix, 0); //left
	p_planes[1] = glm::row(comboMatrix, 3) - glm::row(comboMatrix, 0); //right
	p_planes[2] = glm::row(comboMatrix, 3) - glm::row(comboMatrix, 1); //bottom
	p_planes[3] = glm::row(comboMatrix, 3) + glm::row(comboMatrix, 1); //top
	p_planes[4] = glm::row(comboMatrix, 3) + glm::row(comboMatrix, 2); //near
	p_planes[5] = glm::row(comboMatrix, 3) - glm::row(comboMatrix, 2); //far
	
	//for (int i = 0; i < 6; i++) {
	//	auto temp = glm::length(glm::vec3(p_planes[i]));
	//	p_planes[i] = p_planes[i] / temp;
	//}

	

	
	for (auto mesh : meshes) {
		if (!frustum_cull(mesh, p_planes)) {
			render_meshes.emplace_back(mesh);
		}
	}
	std::sort(render_meshes.begin(), render_meshes.end(),
		[view, model](Mesh* m1, Mesh* m2) {glm::vec3 camera_pos = glm::vec3(glm::inverse(model) * view * glm::vec4(0, 0, 1, 0)); return ((m1->getBBoxMax() + m1->getBBoxMin()) * camera_pos).z > ((m2->getBBoxMax() + m2->getBBoxMin()) * camera_pos).z; });

	Render(sp, render_meshes);
}

void Model::addMesh(Mesh* mesh) {
	meshes.emplace_back(mesh);
}
void Model::addModel(Model* model) {
	for (auto mesh : model->getMeshes()) {
		meshes.emplace_back(mesh);
	}
}


void Model::loadModel(std::string path, bool import_tangents) { 
	
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |
		aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_FixInfacingNormals |
		aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices |
		aiProcess_GenBoundingBoxes);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('\\'));

	processNode(scene->mRootNode, scene, import_tangents);
}

void Model::setModel(bool use_tangents) {
	for (auto mesh : meshes) {
		mesh->SetData(GL_STATIC_DRAW, use_tangents);
	}
}

void Model::processNode(aiNode *node, const aiScene *scene, bool import_tangents) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene, import_tangents));
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, import_tangents);
	}
}

Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene, bool import_tangents) {
	Shape shape = Shape();
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		auto temp = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		shape.addVertex(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
		shape.addNormal(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
		if (import_tangents) {
			shape.addTangents(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		}
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
	ret->setBBoxMin(glm::vec3(mesh->mAABB.mMin.x,
		mesh->mAABB.mMin.y, mesh->mAABB.mMin.z));
	ret->setBBoxMax(glm::vec3(mesh->mAABB.mMax.x,
		mesh->mAABB.mMax.y, mesh->mAABB.mMax.z));
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
	//if (textures.size() == 0) {
	//	aiColor4D color(0, 0, 0, 0);
	//	mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	//	textures.emplace_back(Texture2D(glm::vec4(color.r, color.g, color.b, color.a)));
	//}


	return textures;
}