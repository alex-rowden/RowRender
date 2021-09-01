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
		case Shaders::INSTANCE_VERT_COLOR:
		case Shaders::VOLUME_VERT_3D:
		case Shaders::VERT_ELLIPSOID:
		case Shaders::DEFFERED_RENDER_VERT:
		case Shaders::DEFFERED_RENDER_ELLIPSOID_VERT:
		case Shaders::SSAO_VERT:
		case Shaders::QUAD_RENDER_VERT:
		case Shaders::LIC_PREPASS_VERT:
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
		case Shaders::SIGNED_DISTANCE_FRAG:
		case Shaders::FRONT_FRAG:
		case Shaders::BACK_FRAG:
		case Shaders::INSTANCE_FRAG_COLOR:
		case Shaders::VOLUME_FRAG_3D:
		case Shaders::FRAG_ELLIPSOID:
		case Shaders::DEFFERED_RENDER_FRAG:
		case Shaders::DEFFERED_RENDER_ELLIPSOID_FRAG:
		case Shaders::PREPASS_SHADER:
		case Shaders::SSAO_FRAG:
		case Shaders::QUAD_RENDER_FRAG:
		case Shaders::LIC_PREPASS_FRAG:
		case Shaders::LIC_FRAG:
		case Shaders::LIC_ACCUM_FRAG:
		case Shaders::SSAO_BLUR_FRAG:
			glAttachShader(shaderProgram, fragmentShader);
			break;
		}
	}
	GLboolean is_program = glIsProgram(shaderProgram);
	GLint numShaders;
	glGetProgramiv(shaderProgram, GL_ATTACHED_SHADERS, &numShaders);
	glLinkProgram(shaderProgram);
	program_error_check(shaders[0]);
	glUseProgram(shaderProgram);
}

ShaderProgram::ShaderProgram(std::vector<std::string> shaders) {
	shaderProgram = glCreateProgram();
	for (int i = 0; i < shaders.size(); i++) {
		SetupShader(shaders.at(i), i);
		if (i == 1) {
			glAttachShader(shaderProgram, vertexShader);
		}
		else if (i == 0) {
			glAttachShader(shaderProgram, fragmentShader);
		}
	}
	GLboolean is_program = glIsProgram(shaderProgram);
	GLint numShaders;
	glGetProgramiv(shaderProgram, GL_ATTACHED_SHADERS, &numShaders);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);
}

void ShaderProgram::Use() {
	glUseProgram(shaderProgram);
}

int ShaderProgram::getFloor(glm::vec3 position, float scaling_factor) {
	int floor = 0;
	float z_coord = position.z;
	if (z_coord > .92 * scaling_factor)
		floor++;
	if (z_coord > 1.92 * scaling_factor)
		floor++;
	if (z_coord > 2.92 * scaling_factor)
		floor++;
	return floor;
}

void ShaderProgram::SetLights(Lights&lights, glm::vec3 position, int num_lights, float scaling_factor) {
	if (num_lights != -1) {
	float z_boost = -.1;
	float z_coord = position.z;
	if (z_coord > .92 * scaling_factor)
		z_boost += 1.01 * scaling_factor;
	if (z_coord > 1.92 * scaling_factor)
		z_boost += 1.01 * scaling_factor;
	if (z_coord > 2.92 * scaling_factor)
		z_boost += 1.01 * scaling_factor;
	for (int i = 0; i < lights.point_lights.size(); i++) {
		lights.point_lights.at(i).position.z += z_boost;
	}
	
		std::vector<float> distances(num_lights);
		std::fill(distances.begin(), distances.end(), std::numeric_limits<float>().max());
		std::vector<int> indices(num_lights);
		float curr_distance, swap_distance;
		int curr_index, swap_index;
		for (int i = 0; i < lights.point_lights.size(); i++) {
			curr_distance = glm::distance(lights.point_lights[i].position, position);
			curr_index = i;
			for (int j = 0; j < num_lights; j++) {
				if (curr_distance < distances[j]) {
					swap_distance = distances[j];
					swap_index = indices[j];
					distances[j] = curr_distance;
					indices[j] = curr_index;
					curr_distance = swap_distance;
					curr_index = swap_index;
				}
			}
		}
		std::vector<Lights::PointLight> new_lights(num_lights);
		for (int i = 0; i < num_lights; i++) {
			/*
			std::vector<int>::iterator it = std::find(indices.begin(), indices.end(), i);
			
			std::swap(lights.point_lights[i], lights.point_lights[indices[i]]);
			if (it != indices.end()) {
				int index = distance(indices.begin(), it);
				indices[i] = index;
			}
			*/
			new_lights[i] = lights.point_lights[indices[i]];
		}
		
		//lights.point_lights.resize(num_lights);
		(&lights)->point_lights = new_lights;
   	}
	
	SetLights(lights);
}

void ShaderProgram::SetLights(Lights lights) {
	for (int i = 0; i < lights.getPointLights().size(); i++) {
		Lights::PointLight light = lights.getPointLights().at(i);
		
		std::string light_preamble = "pointLights[" + std::to_string(i) + "].";
		SetUniform((light_preamble + "position").c_str(), light.position);

		SetUniform((light_preamble + "constant").c_str(), light.constant);
		SetUniform((light_preamble + "linear").c_str(), light.linear);
		SetUniform((light_preamble + "quadratic").c_str(), light.quadratic);

		SetUniform((light_preamble + "ambient").c_str(), light.ambient);
		SetUniform((light_preamble + "diffuse").c_str(), light.diffuse);
		SetUniform((light_preamble + "specular").c_str(), light.specular);

	}for (int i = 0; i < lights.getDirLights().size(); i++) {
		Lights::DirLight light = lights.getDirLights().at(i);
		
		std::string light_preamble = "dirLights[" + std::to_string(i) + "].";
		SetUniform((light_preamble + "direction").c_str(), light.direction);

		SetUniform((light_preamble + "color").c_str(), light.color);

	}
	SetUniform("num_point_lights", (int)lights.getPointLights().size());
}

void ShaderProgram::SetEllipsoid(Ellipsoid ellipse) {
	SetUniform("ellipsoid.mu", ellipse.mu);
	SetUniform("ellipsoid.axes", ellipse.axis);
	SetUniform("ellipsoid.r", ellipse.r);
}

void ShaderProgram::SetGaussians(std::vector<Gaussian> gaus) {
	
	for (int i = 0; i < gaus.size(); i++) {

		std::string gauss_preamble = "gaussians[" + std::to_string(i) + "].";
		SetUniform((gauss_preamble + "x_coord").c_str(), gaus[i].x_coord * 50);
		SetUniform((gauss_preamble + "y_coord").c_str(), gaus[i].y_coord * 50);
		SetUniform((gauss_preamble + "sigma").c_str(), gaus[i].sigma * 10 );
		SetUniform((gauss_preamble + "amplitude").c_str(), gaus[i].amplitude);
	}
}


void ShaderProgram::SetUniform(std::string name, glm::vec4 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}
void ShaderProgram::SetUniform(std::string name, glm::vec3 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform3f(location, vec.x, vec.y, vec.z);
}
void ShaderProgram::SetUniform(std::string name, glm::vec2 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform2f(location, vec.x, vec.y);
}
void ShaderProgram::SetUniform(std::string name, float val) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform1f(location, val);
}

void ShaderProgram::SetUniform(std::string name, glm::ivec4 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform4i(location, vec.x, vec.y, vec.z, vec.w);
}
void ShaderProgram::SetUniform(std::string name, glm::ivec3 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform3i(location, vec.x, vec.y, vec.z);
}
void ShaderProgram::SetUniform(std::string name, glm::ivec2 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform2i(location, vec.x, vec.y);
}
void ShaderProgram::SetUniform(std::string name, int val) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform1i(location, val);
}

void ShaderProgram::SetUniform(std::string name, glm::uvec4 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform4ui(location, vec.x, vec.y, vec.z, vec.w);
}
void ShaderProgram::SetUniform(std::string name, glm::uvec3 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform3ui(location, vec.x, vec.y, vec.z);
}
void ShaderProgram::SetUniform(std::string name, glm::uvec2 vec) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform2ui(location, vec.x, vec.y);
}
void ShaderProgram::SetUniform(std::string name, unsigned int val) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniform1ui(location, val);
}

void ShaderProgram::SetUniform(std::string name, glm::mat4 mat, GLint transpose) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(location, 1, transpose, glm::value_ptr(mat));
}
void ShaderProgram::SetUniform(std::string name, glm::mat3 mat, GLint transpose) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniformMatrix3fv(location, 1, transpose, glm::value_ptr(mat));
}
void ShaderProgram::SetUniform(std::string name, glm::mat2 mat, GLint transpose) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	glUniformMatrix2fv(location, 1, transpose, glm::value_ptr(mat));
}

void ShaderProgram::SetUniform(std::string name, bool in) {
	int location = glGetUniformLocation(shaderProgram, name.c_str());
	glUseProgram(shaderProgram);
	if (in)
		glUniform1i(location, 1);
	else
		glUniform1i(location, 0);
}


void ShaderProgram::program_error_check(Shaders shader) {
	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
		std::string filename;
		switch (shader) {
		case Shaders::VERTEX:
			filename = "shaders/vertex_shader.glsl";
			break;
		case Shaders::FRAGMENT:
			filename = "shaders/fragment_shader.glsl";
			break;
		case Shaders::LIGHT_FRAG:
			filename = "shaders/light_frag.glsl";
			break;
		case Shaders::LIGHT_VERT:
			filename = "shaders/light_vert.glsl";
			break;
		case Shaders::NO_LIGHT_FRAG:
			filename = "shaders/fragment_shader_no_light.glsl";
			break;
		case Shaders::NO_LIGHT_VERT:
			filename = "shaders/vertex_shader_no_light.glsl";
			break;
		case Shaders::SCREEN_FRAG:
			filename = "shaders/screen_fshader.glsl";
			break;
		case Shaders::SCREEN_VERT:
			filename = "shaders/screen_vshader.glsl";
			break;
		case Shaders::SKY_FRAG:
			filename = "shaders/sky_fshader.glsl";
			break;
		case Shaders::SKY_VERT:
			filename = "shaders/sky_vshader.glsl";
			break;
		case Shaders::INSTANCE_FRAG:
			filename = "shaders/instance_fshader.glsl";
			break;
		case Shaders::INSTANCE_VERT:
			filename = "shaders/instance_vshader.glsl";
			break;
		case Shaders::VOLUME_FRAG:
			filename = "shaders/volume_fragment.glsl";
			break;
		case Shaders::VOLUME_VERT:
			filename = "shaders/volume_vertex.glsl";
			break;
		case Shaders::FRONT_BACK_FRAG:
			filename = "shaders/front_back_fshader.glsl";
			break;
		case Shaders::FRONT_BACK_VERT:
			filename = "shaders/front_back_vshader.glsl";
			break;
		case Shaders::SIGNED_DISTANCE_FRAG:
			filename = "shaders/signed_distance_fragment.glsl";
			break;
		case Shaders::FRONT_FRAG:
			filename = "shaders/front_fshader.glsl";
			break;
		case Shaders::BACK_FRAG:
			filename = "shaders/back_fshader.glsl";
			break;
		case Shaders::INSTANCE_FRAG_COLOR:
			filename = "shaders/instance_fshader_color.glsl";
			break;
		case Shaders::INSTANCE_VERT_COLOR:
			filename = "shaders/instance_vshader_color.glsl";
			break;
		case Shaders::VOLUME_FRAG_3D:
			filename = "shaders/3d_volume_fragment.glsl";
			break;
		case Shaders::VOLUME_VERT_3D:
			filename = "shaders/3d_volume_vertex.glsl";
			break;
		case Shaders::FRAG_ELLIPSOID:
			filename = "shaders/fragment_shader_ellipse.glsl";
			break;
		case Shaders::VERT_ELLIPSOID:
			filename = "shaders/vertex_shader_ellipse.glsl";
			break;
		case Shaders::DEFFERED_RENDER_FRAG:
			filename = "shaders/deffered_fragment_shader.glsl";
			break;
		case Shaders::DEFFERED_RENDER_VERT:
			filename = "shaders/deffered_vertex_shader.glsl";
			break;
		case Shaders::DEFFERED_RENDER_ELLIPSOID_FRAG:
			filename = "shaders/deffered_fragment_ellipsoid_shader.glsl";
			break;
		case Shaders::DEFFERED_RENDER_ELLIPSOID_VERT:
			filename = "shaders/deffered_vertex_ellipsoid_shader.glsl";
			break;
		case Shaders::PREPASS_SHADER:
			filename = "shaders/prepass_shader.glsl";
			break;
		case Shaders::SSAO_VERT:
			filename = "shaders/ssao_vert.glsl";
			break;
		case Shaders::SSAO_FRAG:
			filename = "shaders/ssao_frag.glsl";
			break;
		case Shaders::QUAD_RENDER_VERT:
			filename = "shaders/quad_shader.vert";
			break;
		case Shaders::QUAD_RENDER_FRAG:
			filename = "shaders/quad_render.frag";
			break;
		case Shaders::LIC_PREPASS_VERT:
			filename = "shaders/lic_prepass.vert";
			break;
		case Shaders::LIC_PREPASS_FRAG:
			filename = "shaders/lic_prepass.frag";
			break;
		case Shaders::LIC_FRAG:
			filename = "shaders/lic.frag";
			break;
		case Shaders::LIC_ACCUM_FRAG:
			filename = "shaders/lic_accum.frag";
			break;
		case Shaders::SSAO_BLUR_FRAG:
			filename = "shaders/ssao_blur.frag";
			break;
		default:
			throw "Not a valid shader";
		}
		std::cerr << filename << std::endl;
	}
}

const char* ShaderProgram::getShaderFilename(Shaders shader) {
	const char* filename;
	switch (shader) {
	case Shaders::VERTEX:
		return "shaders/vertex_shader.glsl";
	case Shaders::FRAGMENT:
		return "shaders/fragment_shader.glsl";
	case Shaders::LIGHT_FRAG:
		return "shaders/light_frag.glsl";
	case Shaders::LIGHT_VERT:
		return "shaders/light_vert.glsl";
	case Shaders::NO_LIGHT_FRAG:
		return "shaders/fragment_shader_no_light.glsl";
	case Shaders::NO_LIGHT_VERT:
		return "shaders/vertex_shader_no_light.glsl";
	case Shaders::SCREEN_FRAG:
		return "shaders/screen_fshader.glsl";
	case Shaders::SCREEN_VERT:
		return "shaders/screen_vshader.glsl";
	case Shaders::SKY_FRAG:
		return "shaders/sky_fshader.glsl";
	case Shaders::SKY_VERT:
		return "shaders/sky_vshader.glsl";
	case Shaders::INSTANCE_FRAG:
		return "shaders/instance_fshader.glsl";
	case Shaders::INSTANCE_VERT:
		return "shaders/instance_vshader.glsl";
	case Shaders::VOLUME_FRAG:
		return "shaders/volume_fragment.glsl";
	case Shaders::VOLUME_VERT:
		return "shaders/volume_vertex.glsl";
	case Shaders::FRONT_BACK_FRAG:
		return "shaders/front_back_fshader.glsl";
	case Shaders::FRONT_BACK_VERT:
		return "shaders/front_back_vshader.glsl";
	case Shaders::SIGNED_DISTANCE_FRAG:
		return "shaders/signed_distance_fragment.glsl";
	case Shaders::FRONT_FRAG:
		return "shaders/front_fshader.glsl";
	case Shaders::BACK_FRAG:
		return "shaders/back_fshader.glsl";
	case Shaders::INSTANCE_FRAG_COLOR:
		return "shaders/instance_fshader_color.glsl";
	case Shaders::INSTANCE_VERT_COLOR:
		return "shaders/instance_vshader_color.glsl";
	case Shaders::VOLUME_FRAG_3D:
		return "shaders/3d_volume_fragment.glsl";
	case Shaders::VOLUME_VERT_3D:
		return "shaders/3d_volume_vertex.glsl";
	case Shaders::FRAG_ELLIPSOID:
		return "shaders/fragment_shader_ellipse.glsl";
	case Shaders::VERT_ELLIPSOID:
		return "shaders/vertex_shader_ellipse.glsl";
	case Shaders::DEFFERED_RENDER_FRAG:
		return "shaders/deffered_fragment_shader.glsl";
	case Shaders::DEFFERED_RENDER_VERT:
		return "shaders/deffered_vertex_shader.glsl";
	case Shaders::DEFFERED_RENDER_ELLIPSOID_FRAG:
		return "shaders/deffered_fragment_ellipsoid_shader.glsl";
	case Shaders::DEFFERED_RENDER_ELLIPSOID_VERT:
		return "shaders/deffered_vertex_ellipsoid_shader.glsl";
	case Shaders::PREPASS_SHADER:
		return "shaders/prepass_shader.glsl";
	case Shaders::SSAO_FRAG:
		return "shaders/ssao_frag.glsl";
	case Shaders::SSAO_VERT:
		return "shaders/ssao_vert.glsl";
	case Shaders::QUAD_RENDER_FRAG:
		return "shaders/quad_shader.frag";
	case Shaders::QUAD_RENDER_VERT:
		return "shaders/quad_shader.vert";
	case Shaders::LIC_PREPASS_FRAG:
		return "shaders/lic_prepass.frag";
	case Shaders::LIC_PREPASS_VERT:
		return "shaders/lic_prepass.vert";
	case Shaders::LIC_FRAG:
		return "shaders/lic.frag";
	case Shaders::LIC_ACCUM_FRAG:
		return "shaders/lic_accum.frag";
	case Shaders::SSAO_BLUR_FRAG:
		return "shaders/ssao_blur.frag";
	default:
		throw "Not a valid shader";
	}
}

void ShaderProgram::importShaderFile(const char *filename, std::string *ShaderString) {

	std::ifstream shader_file;

	//std::string path = __FILE__; //gets source code path, include file name
	//path = path.substr(0, 1 + path.find_last_of('\\')); //removes file name

	shader_file.open(filename);
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
	case Shaders::SIGNED_DISTANCE_FRAG:
		shader_name = "SIGNED_DISTANCE_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::FRONT_FRAG:
		shader_name = "FRONT_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::BACK_FRAG:
		shader_name = "BACK_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::INSTANCE_FRAG_COLOR:
		shader_name = "INSTANCE_FRAGMENT_COLOR_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::INSTANCE_VERT_COLOR:
		shader_name = "INSTANCE_FRAGMENT_COLOR_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::VOLUME_FRAG_3D:
		shader_name = "3D_VOLUME_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::VOLUME_VERT_3D:
		shader_name = "3D_VOLUME_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::FRAG_ELLIPSOID:
		shader_name = "ELLIPSOID_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::VERT_ELLIPSOID:
		shader_name = "ELLIPSOID_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::PREPASS_SHADER:
		shader_name = "PREPASS_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::DEFFERED_RENDER_VERT:
		shader_name = "DEFFERED_VERTEX_RENDER";
		shader_adr = &vertexShader;
		break;
	case Shaders::DEFFERED_RENDER_FRAG:
		shader_name = "DEFFERED_FRAGMENT_RENDER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::DEFFERED_RENDER_ELLIPSOID_VERT:
		shader_name = "DEFFERED_VERTEX_ELLIPSOID_RENDER";
		shader_adr = &vertexShader;
		break;
	case Shaders::DEFFERED_RENDER_ELLIPSOID_FRAG:
		shader_name = "DEFFERED_FRAGMENT_ELLIPSOID_RENDER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::SSAO_VERT:
		shader_name = "SSAO_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::SSAO_FRAG:
		shader_name = "SSAO_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::QUAD_RENDER_VERT:
		shader_name = "QUAD_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::QUAD_RENDER_FRAG:
		shader_name = "QUAD_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::LIC_PREPASS_VERT:
		shader_name = "LIC_PREPASS_VERTEX_SHADER";
		shader_adr = &vertexShader;
		break;
	case Shaders::LIC_PREPASS_FRAG:
		shader_name = "LIC_PREPASS_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::LIC_FRAG:
		shader_name = "LIC_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::LIC_ACCUM_FRAG:
		shader_name = "LIC_ACCUM_FRAGMENT_SHADER";
		shader_adr = &fragmentShader;
		break;
	case Shaders::SSAO_BLUR_FRAG:
		shader_name = "SSAO_BLUR_FRAGMENT_SHADER";
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

void ShaderProgram::SetupShader(std::string filename, int shader_num) {
	std::string shaderString;
	importShaderFile(filename.c_str(), &shaderString);
	const char* shader_source = shaderString.c_str();
	int success;
	unsigned int *shader_adr;
	if (shader_num == 1) {
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		shader_adr = &vertexShader;
	}
	else if (shader_num == 0) {
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		shader_adr = &fragmentShader;
	}
	else {
		std::cerr << "Too many shaders" << std::endl;
		return;
	}
	glShaderSource(*shader_adr, 1, &shader_source, NULL);
	glCompileShader(*shader_adr);
	glGetShaderiv(*shader_adr, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(*shader_adr, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << shaderString << "::COMPILATION_FAILED\n" << infoLog << std::endl;
	}


}

void ShaderProgram::SetupShader(Shaders shader) {
	
	std::string filename = getShaderFilename(shader);
	int shader_num = 2;
	switch (shader) {
	case Shaders::VERTEX:
	case Shaders::LIGHT_VERT:
	case Shaders::NO_LIGHT_VERT:
	case Shaders::SCREEN_VERT:
	case Shaders::SKY_VERT:
	case Shaders::INSTANCE_VERT:
	case Shaders::VOLUME_VERT:
	case Shaders::FRONT_BACK_VERT:
	case Shaders::INSTANCE_VERT_COLOR:
	case Shaders::VOLUME_VERT_3D:
	case Shaders::VERT_ELLIPSOID:
	case Shaders::DEFFERED_RENDER_VERT:
	case Shaders::DEFFERED_RENDER_ELLIPSOID_VERT:
	case Shaders::SSAO_VERT:
	case Shaders::QUAD_RENDER_VERT:
	case Shaders::LIC_PREPASS_VERT:
		shader_num = 1;
		break;
	case Shaders::FRAGMENT:
	case Shaders::LIGHT_FRAG:
	case Shaders::NO_LIGHT_FRAG:
	case Shaders::SCREEN_FRAG:
	case Shaders::SKY_FRAG:
	case Shaders::INSTANCE_FRAG:
	case Shaders::VOLUME_FRAG:
	case Shaders::FRONT_BACK_FRAG:
	case Shaders::SIGNED_DISTANCE_FRAG:
	case Shaders::FRONT_FRAG:
	case Shaders::BACK_FRAG:
	case Shaders::INSTANCE_FRAG_COLOR:
	case Shaders::VOLUME_FRAG_3D:
	case Shaders::FRAG_ELLIPSOID:
	case Shaders::DEFFERED_RENDER_FRAG:
	case Shaders::DEFFERED_RENDER_ELLIPSOID_FRAG:
	case Shaders::PREPASS_SHADER:
	case Shaders::SSAO_FRAG:
	case Shaders::QUAD_RENDER_FRAG:
	case Shaders::LIC_PREPASS_FRAG:
	case Shaders::LIC_FRAG:
	case Shaders::LIC_ACCUM_FRAG:
	case Shaders::SSAO_BLUR_FRAG:
		shader_num = 0;
	}
	SetupShader(filename, shader_num);

}