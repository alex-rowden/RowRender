#pragma once
#include "RowRender.h"

#include <fstream>
class Lights;
class Camera;
struct Ellipsoid;

class ShaderProgram
{
public:
	enum class Shaders {
		VERTEX, FRAGMENT,
		LIGHT_VERT, LIGHT_FRAG,
		NO_LIGHT_FRAG, NO_LIGHT_VERT,
		SCREEN_VERT, SCREEN_FRAG,
		SKY_FRAG, SKY_VERT,
		INSTANCE_FRAG, INSTANCE_VERT,
		VOLUME_FRAG, VOLUME_VERT,
		FRONT_BACK_FRAG, FRONT_BACK_VERT,
		SIGNED_DISTANCE_FRAG,
		FRONT_FRAG, BACK_FRAG,
		INSTANCE_FRAG_COLOR, INSTANCE_VERT_COLOR,
		VOLUME_FRAG_3D, VOLUME_VERT_3D,
		FRAG_ELLIPSOID, VERT_ELLIPSOID,
		PREPASS_SHADER,
		DEFFERED_RENDER_VERT, DEFFERED_RENDER_FRAG,
		DEFFERED_RENDER_ELLIPSOID_VERT, DEFFERED_RENDER_ELLIPSOID_FRAG,
		SSAO_VERT, SSAO_FRAG,
		QUAD_RENDER_VERT, QUAD_RENDER_FRAG,
	};
	void SetupShader(Shaders shader);
	ShaderProgram() {};
	ShaderProgram(std::vector<Shaders> shaders);

	void Use();

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

	void SetUniform4fv(const char* uniform_name, glm::mat4 mat, GLint transpose = GL_FALSE);
	void SetUniform3fv(const char* uniform_name, glm::mat3 mat, GLint transpose = GL_FALSE);
	void SetUniform2fv(const char* uniform_name, glm::mat2 mat, GLint transpose = GL_FALSE);

	void SetUniform1b(const char* name, bool in);

	void SetLights(Lights lights);
	void SetLights(Lights&lights, glm::vec3 position, int num_lights = -1);

	void SetEllipsoid(Ellipsoid ellipse);
	void SetGaussians(std::vector<Gaussian> gauss);
	GLint getShader() { return shaderProgram; }
private:
	unsigned int vertexShader, fragmentShader, shaderProgram;
	void importShaderFile(Shaders shader, std::string *ShaderString);
	void shader_error_check(Shaders shader);
	void program_error_check(Shaders shader);

};

