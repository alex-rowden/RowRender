#include "Mesh.h"

Mesh::Mesh() {
	verticies = std::vector<float>();
	indices = std::vector<unsigned int>();
	texCoords = std::vector<float>();
}


Mesh::Mesh(std::vector<Shape *> shapes) {
	Mesh();
	for (auto shape : shapes) {
		for (auto vertex : shape->getVertices()) {
			verticies.emplace_back(vertex.x);
			verticies.emplace_back(vertex.y);
			verticies.emplace_back(vertex.z);
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
		verticies.emplace_back(vertex.x);
		verticies.emplace_back(vertex.y);
		verticies.emplace_back(vertex.z);
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
		verticies.emplace_back(vertex.x);
		verticies.emplace_back(vertex.y);
		verticies.emplace_back(vertex.z);
	}
	for (auto index : _indices) {
		indices.emplace_back(index.x);
		indices.emplace_back(index.y);
		indices.emplace_back(index.z);
	}
}

void Mesh::SetUniformColor(glm::vec4 color) {
	for (int i = 0; i < verticies.size(); i += 4) {
		colors.emplace_back(color.r);
		colors.emplace_back(color.g);
		colors.emplace_back(color.b);
		colors.emplace_back(color.a);
	}
}

void Mesh::SetColors(std::vector<glm::vec4> _colors) {
	if (_colors.size() < verticies.size() / 3) {
		std::cout << _colors.size() << " colors allocated to " << verticies.size() / 3 << " vertices." << std::endl;
	}

	for (auto color : _colors) {
		colors.emplace_back(color.r);
		colors.emplace_back(color.g);
		colors.emplace_back(color.b);
		colors.emplace_back(color.a);
	}
}

void Mesh::SetInstanceTransforms(std::vector<glm::mat4> transforms) {
	num_instances = transforms.size();
	glGenBuffers(1, &matrixBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
	glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), &transforms[0], GL_STATIC_DRAW);

	glBindVertexArray(VertexArrayObject);
	// vertex Attributes
	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
}

void Mesh::SetInstanceTransforms(std::vector<glm::mat4> transforms, std::vector<int> color_indices) {
	num_instances = transforms.size();
	glGenBuffers(1, &matrixBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
	glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), &transforms[0], GL_STATIC_DRAW);

	


	glBindVertexArray(VertexArrayObject);
	// vertex Attributes
	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, color_indices.size() * sizeof(int), &color_indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 1, GL_INT, GL_FALSE, sizeof(int), (void*)0);
	glVertexAttribDivisor(7, 1);

	glBindVertexArray(0);
}


void Mesh::SetData(GLenum usage) {
	glGenVertexArrays(1, &VertexArrayObject);
	glGenBuffers(1, &VertexBufferObject);
	glGenBuffers(1, &NormalBuffer);
	glGenBuffers(1, &IndexBufferArray);
	glGenBuffers(1, &TexCoordBuffer);

	glBindVertexArray(VertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(float), (void*)verticies.data(), usage);

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

void Mesh::setTexture(Texture2D texture, int index) {
	textures[index] = texture;
}

void Mesh::Render(ShaderProgram *shader) {

	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	if (textures.size() == 0) {
		textures.emplace_back(Texture2D(Texture2D::COLORS::WHITE));
	}
	int counter = 0;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		 // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].name;
		if (name == "texture_diffuse") {
			number = std::to_string(diffuseNr++);
			name = name + number;
		}
		else if (name == "texture_specular") {
			number = std::to_string(specularNr++);
			name = name + number;
		}
		
		GLint texture_position = glGetUniformLocation(shader->getShader(), name.c_str());
		if(texture_position >= 0){
			glActiveTexture(GL_TEXTURE0 + counter);
			glUniform1i(texture_position, counter++);
			textures[i].Bind();
		}
		//shader.setFloat(("material." + name + number).c_str(), i);
		
	}
	
	glBindVertexArray(VertexArrayObject);
	if(num_instances == 0)
		glDrawElements(GL_TRIANGLES, indices.size() , GL_UNSIGNED_INT, 0);
	else
		glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, num_instances);

	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}