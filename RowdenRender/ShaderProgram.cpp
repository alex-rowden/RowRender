#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(std::vector<Shaders> shaders) {
	shaderProgram = glCreateProgram();
	for (auto shader : shaders) {
		SetupShader(shader);
		switch (shader) {
		case Shaders::VERTEX:
		case Shaders::LIGHT_VERT:
		case Shaders::NO_LIGHT_VERT:
		case Shaders::SCREEN_VERT:
		case Shaders::SKY_VERT:
		case Shaders::VOLUME_VERT:
		case Shaders::FRONT_BACK_VERT:
		case Shaders::INSTANCE_VERT:
			glAttachShader(shaderProgram, vertexShader);
			break;
		case Shaders::FRAGMENT:
		case Shaders::LIGHT_FRAG:
		case Shaders::NO_LIGHT_FRAG:
		case Shaders::SCREEN_FRAG:
		case Shaders::SKY_FRAG:
		case Shaders::INSTANCE_FRAG:
		case Shaders::VOLUME_FRAG:
		case Shaders::FRONT_BACK_FRAG:
			glAttachShader(shaderProgram, fragmentShader);
			break;
		}
	}
	GLboolean is_program = glIsProgram(shaderProgram);
	GLint numShaders;
	glGetProgramiv(shaderProgram, GL_ATTACHED_SHADERS, &numShaders);
	glLinkProgram(shaderProgram);
	program_error_check();
	glUseProgram(shaderProgram);
}

void ShaderProgram::Use() {
	glUseProgram(shaderProgram);
}

void ShaderProgram::SetLights(Lights lights) {
	for (int i = 0; i < lights.getPointLights().size(); i++) {
		Lights::PointLight light = lights.getPointLights().at(i);
		
		std::string light_preamble = "pointLights[" + std::to_string(i) + "].";
		SetUniform3f((light_preamble + "position").c_str(), light.position);

		SetUniform1f((light_preamble + "constant").c_str(), light.constant);
		SetUniform1f((light_preamble + "linear").c_str(), light.linear);
		SetUniform1f((light_preamble + "quadratic").c_str(), light.quadratic);

		SetUniform3f((light_preamble + "ambient").c_str(), light.ambient);
		SetUniform3f((light_preamble + "diffuse").c_str(), light.diffuse);
		SetUniform3f((light_preamble + "specular").c_str(), light.specular);

	}
}

void ShaderProgram::SetUniform4f(const char *name, glm::vec4 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}
void ShaderProgram::SetUniform3f(const char* name, glm::vec3 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform3f(location, vec.x, vec.y, vec.z);
}
void ShaderProgram::SetUniform2f(const char* name, glm::vec2 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform2f(location, vec.x, vec.y);
}
void ShaderProgram::SetUniform1f(const char* name, float val) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform1f(location, val);
}

void ShaderProgram::SetUniform4i(const char* name, glm::ivec4 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform4i(location, vec.x, vec.y, vec.z, vec.w);
}
void ShaderProgram::SetUniform3i(const char* name, glm::ivec3 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform3i(location, vec.x, vec.y, vec.z);
}
void ShaderProgram::SetUniform2i(const char* name, glm::ivec2 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform2i(location, vec.x, vec.y);
}
void ShaderProgram::SetUniform1i(const char* name, int val) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform1i(location, val);
}

void ShaderProgram::SetUniform4ui(const char* name, glm::uvec4 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform4ui(location, vec.x, vec.y, vec.z, vec.w);
}
void ShaderProgram::SetUniform3ui(const char* name, glm::uvec3 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform3ui(location, vec.x, vec.y, vec.z);
}
void ShaderProgram::SetUniform2ui(const char* name, glm::uvec2 vec) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform2ui(location, vec.x, vec.y);
}
void ShaderProgram::SetUniform1ui(const char* name, unsigned int val) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniform1ui(location, val);
}

void ShaderProgram::SetUniform4fv(const char* name, glm::mat4 mat, GLint transpose) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(location, 1, transpose, glm::value_ptr(mat));
}
void ShaderProgram::SetUniform3fv(const char* name, glm::mat3 mat, GLint transpose) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniformMatrix3fv(location, 1, transpose, glm::value_ptr(mat));
}
void ShaderProgram::SetUniform2fv(const char* name, glm::mat2 mat, GLint transpose) {
	int location = glGetUniformLocation(shaderProgram, name);
	glUseProgram(shaderProgram);
	glUniformMatrix2fv(location, 1, transpose, glm::value_ptr(mat));
}


void ShaderProgram::program_error_check() {
	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
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
	case Shaders::LIGHT_FRAG:
		filename = "light_frag.glsl";
		break;
	case Shaders::LIGHT_VERT:
		filename = "light_vert.glsl";
		break;
	case Shaders::NO_LIGHT_FRAG:
		filename = "fragment_shader_no_light.glsl";
		break;
	case Shaders::NO_LIGHT_VERT:
		filename = "vertex_shader_no_light.glsl";
		break;
	case Shaders::SCREEN_FRAG:
		filename = "screen_fshader.glsl";
		break;
	case Shaders::SCREEN_VERT:
		filename = "screen_vshader.glsl";
		break;
	case Shaders::SKY_FRAG:
		filename = "sky_fshader.glsl";
		break;
	case Shaders::SKY_VERT:
		filename = "sky_vshader.glsl";
		break;
	case Shaders::INSTANCE_FRAG:
		filename = "instance_fshader.glsl";
		break;
	case Shaders::INSTANCE_VERT:
		filename = "instance_vshader.glsl";
		break;
	case Shaders::VOLUME_FRAG:
		filename = "volume_fragment.glsl";
		break;
	case Shaders::VOLUME_VERT:
		filename = "volume_vertex.glsl";
		break;
	case Shaders::FRONT_BACK_FRAG:
		filename = "front_back_fshader.glsl";
		break;
	case Shaders::FRONT_BACK_VERT:
		filename = "front_back_vshader.glsl";
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
	case Shaders::LIGHT_VERT:
		shader_name = "LIGHT_VERT";
		shader_adr = &vertexShader;
		break;
	case Shaders::LIGHT_FRAG:
		shader_name = "LIGHT_FRAG";
		shader_adr = &fragmentShader;
		break;
	case Shaders::NO_LIGHT_VERT:
		shader_name = "NO_LIGHT_VERT";
		shader_adr = &vertexShader;
		break;
	case Shaders::NO_LIGHT_FRAG:
		shader_name = "NO_LIHT_FRAG";
		shader_adr = &fragmentShader;
		break;
	case Shaders::SCREEN_VERT:
		shader_name = "SCREEN_SPACE_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::SCREEN_FRAG:
		shader_name = "SCREEN_SPACE_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::SKY_VERT:
		shader_name = "SKYBOX_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::SKY_FRAG:
		shader_name = "SKYBOX_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::INSTANCE_VERT:
		shader_name = "INSTANCE_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::INSTANCE_FRAG:
		shader_name = "INSTANCE_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::VOLUME_VERT:
		shader_name = "VOLUME_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::VOLUME_FRAG:
		shader_name = "VOLUME_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::FRONT_BACK_VERT:
		shader_name = "FRONT_BACK_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::FRONT_BACK_FRAG:
		shader_name = "FRONT_BACK_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	default:
		throw("Missing definition for shader in shader_error_check");
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
	case Shaders::LIGHT_VERT:
	case Shaders::NO_LIGHT_VERT:
	case Shaders::SCREEN_VERT:
	case Shaders::SKY_VERT:
	case Shaders::INSTANCE_VERT:
	case Shaders::VOLUME_VERT:
	case Shaders::FRONT_BACK_VERT:
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &shader_source, NULL);
		glCompileShader(vertexShader);
		break;
	case Shaders::FRAGMENT:
	case Shaders::LIGHT_FRAG:
	case Shaders::NO_LIGHT_FRAG:
	case Shaders::SCREEN_FRAG:
	case Shaders::SKY_FRAG:
	case Shaders::INSTANCE_FRAG:
	case Shaders::VOLUME_FRAG:
	case Shaders::FRONT_BACK_FRAG:
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &shader_source, NULL);
		glCompileShader(fragmentShader);
		break;
	default:
		throw("Missing Shader Type definition in SetupShader");
	}
	

	shader_error_check(shader);
}