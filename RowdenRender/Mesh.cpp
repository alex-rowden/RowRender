#include "Mesh.h"

Mesh::Mesh() {
	vertices = std::vector<float>();
	indices = std::vector<unsigned int>();
	texCoords = std::vector<float>();
}


Mesh::Mesh(std::vector<Shape *> shapes) {
	Mesh();
	for (auto shape : shapes) {
		for (auto vertex : shape->getVertices()) {
			vertices.emplace_back(vertex.x);
			vertices.emplace_back(vertex.y);
			vertices.emplace_back(vertex.z);
		}
		for (auto index : shape->getIndices()) {
			indices.emplace_back(index.x);
			indices.emplace_back(index.y);
			indices.emplace_back(index.z);
		}
		for (auto texCoord : shape->getTexCoords()) {
			texCoords.emplace_back(texCoord.x);
			texCoords.emplace_back(texCoord.y);
		}
		for (auto normal : shape->getNormals()) {
			normals.emplace_back(normal.x);
			normals.emplace_back(normal.y);
			normals.emplace_back(normal.z);
		}
	}
}
Mesh::Mesh(Shape *shape) {
	Mesh();
	for (auto vertex : shape->getVertices()) {
		vertices.emplace_back(vertex.x);
		vertices.emplace_back(vertex.y);
		vertices.emplace_back(vertex.z);
	}
	for (auto index : shape->getIndices()) {
		indices.emplace_back(index.x);
		indices.emplace_back(index.y);
		indices.emplace_back(index.z);
	}
	for (auto texCoord : shape->getTexCoords()) {
		texCoords.emplace_back(texCoord.x);
		texCoords.emplace_back(texCoord.y);
	}
	for (auto normal : shape->getNormals()) {
		normals.emplace_back(normal.x);
		normals.emplace_back(normal.y);
		normals.emplace_back(normal.z);
	}
}

Mesh::Mesh(std::vector<glm::vec3> _vertices, std::vector<glm::ivec3> _indices) {
	Mesh();
	for (auto vertex : _vertices) {
		vertices.emplace_back(vertex.x);
		vertices.emplace_back(vertex.y);
		vertices.emplace_back(vertex.z);
	}
	for (auto index : _indices) {
		indices.emplace_back(index.x);
		indices.emplace_back(index.y);
		indices.emplace_back(index.z);
	}
}

void Mesh::SetUniformColor(glm::vec4 color) {
	for (int i = 0; i < vertices.size(); i += 4) {
		colors.emplace_back(color.r);
		colors.emplace_back(color.g);
		colors.emplace_back(color.b);
		colors.emplace_back(color.a);
	}
}

void Mesh::SetColors(std::vector<glm::vec4> _colors) {
	if (_colors.size() < vertices.size() / 3) {
		std::cout << _colors.size() << " colors allocated to " << vertices.size() / 3 << " vertices." << std::endl;
	}

	for (auto color : _colors) {
		colors.emplace_back(color.r);
		colors.emplace_back(color.g);
		colors.emplace_back(color.b);
		colors.emplace_back(color.a);
	}
}


void Mesh::SetData(GLenum usage) {
	glGenVertexArrays(1, &VertexArrayObject);
	glGenBuffers(1, &VertexBufferObject);
	glGenBuffers(1, &NormalBuffer);
	glGenBuffers(1, &IndexBufferArray);
	glGenBuffers(1, &TexCoordBuffer);

	glBindVertexArray(VertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), (void*)vertices.data(), usage);

	glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), (void*)normals.data(), usage);

	glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), (void*)texCoords.data(), usage);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferArray);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), usage);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	//glBindBuffer(GL_ARRAY_BUFFER, ColorBufferArray);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
	//glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::addTexture(Texture2D texture) {
	textures.emplace_back(texture);
}

void Mesh::Render() {

	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].name;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);

		//shader.setFloat(("material." + name + number).c_str(), i);
		textures[i].Bind();
	}
	
	glBindVertexArray(VertexArrayObject);
	glDrawElements(GL_TRIANGLES, indices.size() , GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}