#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(std::vector<Shaders> shaders) {
	shaderProgram = glCreateProgram();
	for (auto shader : shaders) {
		SetupShader(shader);
		switch (shader) {
		case Shaders::VERTEX:
			glAttachShader(shaderProgram, vertexShader);
			break;
		case Shaders::FRAGMENT:
			glAttachShader(shaderProgram, fragmentShader);
			break;
		}
	}
	glLinkProgram(shaderProgram);
	program_error_check();
	glUseProgram(shaderProgram);
}

void ShaderProgram::program_error_check() {
	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}
}

void ShaderProgram::importShaderFile(Shaders shader, std::string *ShaderString) {
	const char* filename;
	switch (shader) {
	case Shaders::VERTEX:
		filename = "vertex_shader.glsl";
		break;
	case Shaders::FRAGMENT:
		filename = "fragment_shader.glsl";
		break;
	default:
		throw "Not a valid shader";
	}

	std::ifstream shader_file;

	std::string path = __FILE__; //gets source code path, include file name
	path = path.substr(0, 1 + path.find_last_of('\\')); //removes file name

	shader_file.open(path + filename);
	if (shader_file.is_open()) {
		char line[256];
		while (shader_file.good()) {
			shader_file.getline(line, 256);
			ShaderString->append(line);
			ShaderString->append("\n");
		}
	}
	shader_file.close();
}

void ShaderProgram::shader_error_check(Shaders shader) {
	const char* shader_name;
	unsigned int *shader_adr;

	switch (shader) {
	case Shaders::VERTEX:
		shader_name = "VERTEX";
		shader_adr = &vertexShader;
		break;
	case Shaders::FRAGMENT:
		shader_name = "FRAGMENT";
		shader_adr = &fragmentShader;
		break;
	default:
		return;
	}

	int success;
	glGetShaderiv(*shader_adr, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(*shader_adr, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::"<< shader_name <<"::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

void ShaderProgram::SetupShader(Shaders shader) {
	std::string shaderString;
	importShaderFile(shader, &shaderString);
	const char* shader_source = shaderString.c_str();
	switch (shader) {
	case Shaders::VERTEX:
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &shader_source, NULL);
		glCompileShader(vertexShader);
		break;
	case Shaders::FRAGMENT:
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &shader_source, NULL);
		glCompileShader(fragmentShader);
		break;
	}
	shader_error_check(shader);
}