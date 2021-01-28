#pragma once
#include "RowRender.h"

#include <fstream>
#include <map>
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
		SSAO_VERT, SSAO_BLUR_FRAG, SSAO_FRAG,
		QUAD_RENDER_VERT, QUAD_RENDER_FRAG,
		LIC_PREPASS_VERT, LIC_PREPASS_FRAG,
		LIC_FRAG, LIC_ACCUM_FRAG
	};
	void SetupShader(Shaders shader);
	ShaderProgram() {};
	ShaderProgram(std::vector<Shaders> shaders);
	
	void Use();
	

	void SetUniform(std::string uniform_name, glm::vec4 vec);
	void SetUniform(std::string uniform_name, glm::vec3 vec);
	void SetUniform(std::string uniform_name, glm::vec2 vec);
	void SetUniform(std::string uniform_name, float val);

	void SetUniform(std::string uniform_name, glm::ivec4 vec);
	void SetUniform(std::string uniform_name, glm::ivec3 vec);
	void SetUniform(std::string uniform_name, glm::ivec2 vec);
	void SetUniform(std::string uniform_name, int val);

	void SetUniform(std::string uniform_name, glm::uvec4 vec);
	void SetUniform(std::string uniform_name, glm::uvec3 vec);
	void SetUniform(std::string uniform_name, glm::uvec2 vec);
	void SetUniform(std::string uniform_name, unsigned int val);

	void SetUniform(std::string uniform_name, glm::mat4 mat, GLint transpose = GL_FALSE);
	void SetUniform(std::string uniform_name, glm::mat3 mat, GLint transpose = GL_FALSE);
	void SetUniform(std::string uniform_name, glm::mat2 mat, GLint transpose = GL_FALSE);

	void SetUniform(std::string name, bool in);

	template <typename T>
	void SetUniforms(std::map<std::string, T> dict) {
		for (const auto& pair : dict) {
			SetUniform(std::get<0>(pair), std::get<1>(pair));
		}
	};

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

