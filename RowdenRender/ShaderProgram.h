#pragma once
#include "RowRender.h"
#include <fstream>
class ShaderProgram
{
public:
	enum class Shaders { VERTEX, FRAGMENT };
	void SetupShader(Shaders shader);
	ShaderProgram(std::vector<Shaders> shaders);

private:
	unsigned int vertexShader, fragmentShader, shaderProgram;
	const char* importShaderFile(Shaders shader);
	void shader_error_check(Shaders shader);
	void program_error_check();

};

