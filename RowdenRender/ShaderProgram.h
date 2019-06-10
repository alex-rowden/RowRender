#pragma once
#include "RowRender.h"
#include <fstream>
class ShaderProgram
{
public:
	enum class Shaders { VERTEX, FRAGMENT };
	void SetupShader(Shaders shader);
	ShaderProgram(std::vector<Shaders> shaders);

	void SetUniform4f(const char *uniform_name, glm::vec4 vec);
	void SetUniform3f(const char* uniform_name, glm::vec3 vec);
	void SetUniform2f(const char* uniform_name, glm::vec2 vec);
	void SetUniform1f(const char* uniform_name, float val);

	void SetUniform4i(const char* uniform_name, glm::ivec4 vec);
	void SetUniform3i(const char* uniform_name, glm::ivec3 vec);
	void SetUniform2i(const char* uniform_name, glm::ivec2 vec);
	void SetUniform1i(const char* uniform_name, int val);

	void SetUniform4ui(const char* uniform_name, glm::uvec4 vec);
	void SetUniform3ui(const char* uniform_name, glm::uvec3 vec);
	void SetUniform2ui(const char* uniform_name, glm::uvec2 vec);
	void SetUniform1ui(const char* uniform_name, unsigned int val);
private:
	unsigned int vertexShader, fragmentShader, shaderProgram;
	void importShaderFile(Shaders shader, std::string *ShaderString);
	void shader_error_check(Shaders shader);
	void program_error_check();

};

