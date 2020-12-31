
#pragma once
#include "RowRender.h"
#include "WifiData.h"
#include "AVWWifiData.h"

#include "VR_Wrapper.h"
#include "DataBuffer.h"
#include "LineIntegralConvolution.h"

#include <random>
#include <fstream>
#include <iostream>
#include <chrono>
#include <time.h>
#include <numeric>

#include <glm/gtx/matrix_decompose.hpp>

#include <glm/gtc/matrix_access.hpp>

using namespace vr;

GLuint fhp_tex, bhp_tex;
bool nearest_router_on = false;
bool jittered = true;

struct gBuffer {
	GLuint frame_buffer, normal_tex, tangent_tex, 
		color_tex, frag_pos_tex, color_array_tex,  ellipsoid_coordinates_tex,
		depth_render_buf, force_framebuffer, force_renderbuffer, force_tex,
		lic_accum_framebuffer, lic_accum_tex, lic_accum_renderbuffer;
	GLuint pboIDs[2], lic_tex[2], lic_framebuffer[2], lic_renderbuffer[2];
	Texture2D color_texture, frag_pos_texture, tangent_texture,
		normal_texture, ellipsoid_coordinates_texture, force_texture,
		lic_texture[2], lic_accum_texture;
};

struct eyeBuffer {
	GLuint frame_buffer, screen_tex, depth_render_buf;
	Texture2D screenTexture;
};
 
void maxSeperatedColors(int n, std::vector<glm::vec4>& out, bool ordered = true) {
	float increment = 360.0f / (n);
	int incrementor = ceil(n / 2.0f) * increment;
	if (ordered)
		incrementor = increment;
	float offset = 0;// rand_float()* increment;
	for (int i = 0; i < n; i++) {
		float v = (incrementor * i) + offset;
		if ((n % 2 || i % 2) && !ordered)
			offset += increment;
		while (v > 360) {
			v = v - 360;
		}
		out[i] = glm::vec4(hsv2rgb(glm::vec3(v, .6f, 1.f)), 1);
	}
}

Lights setPointLights(int num_lights, float intensity, float linear, float quadratic) {
	Lights ret = Lights();
	std::vector<glm::vec3> controlPoints({
		glm::vec3(-11.45, 11.6, .8),
		glm::vec3(-11.45, 15.75, 0.8),
		glm::vec3(8.9, 15.75, 0.8),
		glm::vec3(8.9, -14.35, 0.8),
		glm::vec3(-11.45, -14.35, 0.8),
		glm::vec3(-11.45, -10.24, 0.8),
		glm::vec3(3.9, -10.24, 0.8),
		glm::vec3(3.9, 11.6, 0.8),
		glm::vec3(-11.45, 11.6, .8)
		}
	);
	std::vector<float> distances(controlPoints.size());
	distances.at(0) = 0;
	for (int i = 1; i < controlPoints.size(); i++) {
		distances[i] = glm::distance(
			controlPoints.at(i - 1),
			controlPoints.at(i)
		);
	}
	float total_distance = std::accumulate(distances.begin(), distances.end(), 0);
	float equal_distance = total_distance / (num_lights - 1);
	int curr_light = 0;
	float curr_distance = 0;
	ret.addPointLight(controlPoints.at(0), intensity, linear, quadratic,
		glm::vec3(1), glm::vec3(1), glm::vec3(1));
	curr_light++;
	int j = 1;
	while (curr_distance < total_distance) {
		if (curr_distance + distances.at(j) > equal_distance * curr_light) {
			float dist_between_points = distances.at(j);
			float dist_along_line = equal_distance * curr_light - curr_distance;
			float t = dist_along_line / dist_between_points;
			ret.addPointLight(
				glm::lerp(controlPoints.at(j - 1),
					controlPoints.at(j), 
					t),
				intensity, linear, quadratic, 
				glm::vec3(1), glm::vec3(1), glm::vec3(1)
			);
			curr_light++;
		}
		else {
			curr_distance += distances.at(j++);
		}
	}
	return ret;
}

void createFramebuffer(glm::vec2 resolution, gBuffer* buffer, bool resize) {
	//generate framebuffers during resizing
	if (!resize) {
		glGenFramebuffers(1, &buffer->frame_buffer);
		glGenTextures(1, &buffer->normal_tex);
		glGenTextures(1, &buffer->color_tex);
		glGenTextures(1, &buffer->frag_pos_tex);
		glGenBuffers(2, buffer->pboIDs);
		glGenTextures(1, &buffer->ellipsoid_coordinates_tex);
		glGenTextures(1, &buffer->tangent_tex);
		glGenRenderbuffers(1, &buffer->depth_render_buf);
		glGenFramebuffers(1, &buffer->force_framebuffer);
		glGenTextures(1, &buffer->force_tex);
		glGenRenderbuffers(1, &buffer->force_renderbuffer);
		glGenFramebuffers(2, buffer->lic_framebuffer);
		glGenTextures(2, buffer->lic_tex);
		glGenRenderbuffers(2, buffer->lic_renderbuffer);
		glGenFramebuffers(1, &buffer->lic_accum_framebuffer);
		glGenTextures(1, &buffer->lic_accum_tex);
		glGenRenderbuffers(1, &buffer->lic_accum_renderbuffer);
	}
	//Set Framebuffer Attributes
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->frame_buffer);
	
	glBindTexture(GL_TEXTURE_2D, buffer->normal_tex);
	buffer->normal_texture = Texture2D();
	buffer->normal_texture.SetTextureID(buffer->normal_tex);
	buffer->normal_texture.giveName("normal_tex");
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->normal_tex, 0);	
	
	
	glBindTexture(GL_TEXTURE_2D, buffer->color_tex);
	buffer->color_texture = Texture2D();
	buffer->color_texture.SetTextureID(buffer->color_tex);
	buffer->color_texture.giveName("albedo_tex");
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer->color_tex, 0);	
	
	glBindTexture(GL_TEXTURE_2D, buffer->frag_pos_tex);
	buffer->frag_pos_texture = Texture2D();
	buffer->frag_pos_texture.SetTextureID(buffer->frag_pos_tex);
	buffer->frag_pos_texture.giveName("fragPos_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffer->frag_pos_tex, 0);
	
	glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, resolution.x * resolution.y * 4 * sizeof(float), 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, resolution.x * resolution.y * 4 * sizeof(float), 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, buffer->ellipsoid_coordinates_tex);
	buffer->ellipsoid_coordinates_texture = Texture2D();
	buffer->ellipsoid_coordinates_texture.SetTextureID(buffer->ellipsoid_coordinates_tex);
	buffer->ellipsoid_coordinates_texture.giveName("ellipsoid_coordinates_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, buffer->ellipsoid_coordinates_tex, 0);
	
	glBindTexture(GL_TEXTURE_2D, buffer->tangent_tex);
	buffer->tangent_texture = Texture2D();
	buffer->tangent_texture.SetTextureID(buffer->tangent_tex);
	buffer->tangent_texture.giveName("tangent_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, buffer->tangent_tex, 0);
	
	const int numAttachments = 5;

	GLenum DrawBuffers[numAttachments];
	for (int i = 0; i < numAttachments; ++i)
		DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i; //Sets appropriate indices for each color buffer
	
	glDrawBuffers(numAttachments, DrawBuffers);

	
	glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_render_buf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_render_buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, buffer->force_framebuffer);

	glBindTexture(GL_TEXTURE_2D, buffer->force_tex);
	buffer->force_texture = Texture2D();
	buffer->force_texture.SetTextureID(buffer->force_tex);
	buffer->force_texture.giveName("force_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->force_tex, 0);

	GLenum drawBuffer[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->force_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->force_renderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "lic prepass framebuffer broke" << std::endl;
		return;
	}

	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, buffer->lic_framebuffer[i]);
	
		glBindTexture(GL_TEXTURE_2D, buffer->lic_tex[i]);
		buffer->lic_texture[i] = Texture2D();
		buffer->lic_texture[i].SetTextureID(buffer->lic_tex[i]);
		buffer->lic_texture[i].giveName("lic_tex[" + std::to_string(i) + "]");

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->lic_tex[i], 0);
	
		drawBuffer[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffer);

		glBindRenderbuffer(GL_RENDERBUFFER, buffer->lic_renderbuffer[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->lic_renderbuffer[i]);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "lic framebuffer broke" << std::endl;
			return;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->lic_accum_framebuffer);

	glBindTexture(GL_TEXTURE_2D, buffer->lic_accum_tex);
	buffer->lic_accum_texture = Texture2D();
	buffer->lic_accum_texture.SetTextureID(buffer->lic_accum_tex);
	buffer->lic_accum_texture.giveName("lic_accum_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->lic_accum_tex, 0);

	drawBuffer[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->lic_accum_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->lic_accum_renderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "lic accumulation framebuffer broke" << std::endl;
		return;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createEyeFramebuffer(glm::vec2 resolution, eyeBuffer* buffer, bool resize) {
	if (!resize) {
		glGenFramebuffers(1, &buffer->frame_buffer);
		glGenTextures(1, &buffer->screen_tex);
		glGenRenderbuffers(1, &buffer->depth_render_buf);

	}
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->frame_buffer);

	
	glBindTexture(GL_TEXTURE_2D, buffer->screen_tex);
	buffer->screenTexture = Texture2D();
	buffer->screenTexture.SetTextureID(buffer->screen_tex);
	//buffer->screenTexture.giveName("ScreenTexture");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->screen_tex, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_render_buf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_render_buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createFramebuffers(glm::vec2 resolution, gBuffer *buffers, eyeBuffer *eyes, bool use_vr, bool resize) {
	
	createFramebuffer(resolution, &buffers[0], resize);
	createEyeFramebuffer(resolution, &eyes[0], resize);
	if (use_vr)
		createEyeFramebuffer(resolution, &eyes[1], resize);
	eyes[0].screenTexture.giveName("screenTexture");
}

void updateNoise(LineIntegralConvolution& lic, int num_samples, int num_routers, Texture2D *noise, Texture2D *antialiased_textures, glm::uvec2 lic_texture_res, float an) {
	//lic(glm::uvec2(64, 256));
	for (int i = 0; i < num_routers; i++) {
		lic.clear();
		lic.fillWithNoise(num_samples/num_routers);
		//lic.convolve(fall_off);

		noise[i] = Texture2D(lic.getTexture().data(), lic_texture_res.y, lic_texture_res.x);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, an);
		noise[i].setTexMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		noise[i].giveName("noise_tex[" + std::to_string(i)  + "]");
		antialiased_textures[1 + i] = noise[i];
	}
	
}


void rayPicker(glm::vec3 ray, glm::vec3 ray_coord, glm::vec3 camera_pos ,glm::vec3& intersection) {
	//std::cout << glm::to_string(ray_coord + camera_pos) << std::endl;
	float plane_height = 0;
	float z_coord = ray_coord.z +camera_pos.z;
	plane_height = floor(z_coord);

	//std::cout << plane_height << std::endl;
	//std::cout << z_coord << std::endl;

	float t = (plane_height - z_coord) / ray.z;
	//std::cout << t << std::endl;
	intersection = ray_coord + camera_pos + t * ray;
	//intersection = camera_pos + ray * .1f;
	return;
}


int AVWilliamsWifiVisualization(bool use_vr) {

	
	//initialize glfw
	glfwInit();
	glfwSetErrorCallback(error_callback);
	
	VR_Wrapper vr = VR_Wrapper();

	glm::uvec2 resolution = glm::uvec2(2560, 1440);
	glm::mat4 camera_offset = glm::mat4(1);
	if (use_vr) {
		vr.initialize();
		vr.resetZeroPose();
		vr.initCompositor();
		vr.SetActionManifestPath("C:\\Users\\ARR87\\Documents\\GitHub\\RowRender\\RowdenRender\\actions.json");
		vr.setActionHandles();
		camera_offset = vr.getSeatedZeroPoseToStandingPose();
		resolution = vr.getRenderTargetSize();
	}
	//Open and setup window
	Window w = Window("AV Williams Wifi Visualization", resolution.x, resolution.y);
	//w.setFullScreen(true);
	//initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	//setup IMGUI
	setupDearIMGUI(w.window);

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	//Allow Resizing
	w.SetFramebuferSizeCallback();

	//Disable V-SYNC
	//glfwSwapInterval(0);

	//Enable Transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable Text Rendering
	TextRenderer tr = TextRenderer();
	TextRenderer popupText = TextRenderer();
	int font_size = 64;
	tr.SetCharacterSize(font_size);
	popupText.SetCharacterSize(font_size);
	float pixel_size = font_size * 109 / 72.0f;
	int font_height = ceil(
		pixel_size *
		(tr.arial_face->bbox.yMax - tr.arial_face->bbox.yMin) /
		tr.arial_face->units_per_EM
	);int font_width = ceil(
		pixel_size *
		(tr.arial_face->bbox.xMax - tr.arial_face->bbox.xMin) /
		tr.arial_face->units_per_EM
	);
	int max_chars = 6;
	

	//Load shaders
	ShaderProgram model_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::FRAG_ELLIPSOID, 
		ShaderProgram::Shaders::VERT_ELLIPSOID });
	ShaderProgram skybox_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::SKY_FRAG, 
		ShaderProgram::Shaders::SKY_VERT });
	ShaderProgram instance_shader	= ShaderProgram({ 
		ShaderProgram::Shaders::INSTANCE_FRAG_COLOR,
		ShaderProgram::Shaders::INSTANCE_VERT_COLOR });
	ShaderProgram ground_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::NO_LIGHT_FRAG, 
		ShaderProgram::Shaders::NO_LIGHT_VERT });
	ShaderProgram volume_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::VOLUME_FRAG_3D, 
		ShaderProgram::Shaders::VOLUME_VERT_3D});
	ShaderProgram deferred_shader	= ShaderProgram({ 
		ShaderProgram::Shaders::DEFFERED_RENDER_ELLIPSOID_FRAG, 
		ShaderProgram::Shaders::DEFFERED_RENDER_ELLIPSOID_VERT });
	ShaderProgram render2quad		= ShaderProgram({ 
		ShaderProgram::Shaders::SCREEN_FRAG, 
		ShaderProgram::Shaders::SCREEN_VERT });
	ShaderProgram quad_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::QUAD_RENDER_FRAG, 
		ShaderProgram::Shaders::QUAD_RENDER_VERT });
	ShaderProgram force_shader = ShaderProgram({
		ShaderProgram::Shaders::LIC_PREPASS_FRAG,
		ShaderProgram::Shaders::LIC_PREPASS_VERT });
	ShaderProgram lic_shader = ShaderProgram({
		ShaderProgram::Shaders::LIC_FRAG,
		ShaderProgram::Shaders::LIC_PREPASS_VERT });
	


	//Setup Skybox
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	std::vector<std::string> skybox_files;

	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_lf.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_rt.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_bk.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_ft.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_up.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_dn.png");

	Texture2D skybox_tex = Texture2D(skybox_files);

	Model skybox = Model("./Content/Models/cube/cube.obj");
	skybox.setModel();
	skybox.getMeshes().at(0)->setTexture(skybox_tex, 0);

	//Setup AV Willams Model
	Model AVW("./Content/Models/AVW_model.obj", true);
	AVW.setModel(true);
	Texture2D white = Texture2D(glm::vec4(1,1,1,1));
	//white.giveName("texture_diffuse1");
	AVW.getMeshes()[0]->setTexture(white, 0);
	
	//AVW.getMeshes()[0]->addTexture(text);
	glm::mat4 avw_transform(1);
	avw_transform = glm::rotate(avw_transform, glm::radians(90.f), 
		glm::vec3(1, 0, 0));
	glm::vec3 avw_scale(2, 1, 2);
	avw_transform = glm::scale(avw_transform, avw_scale);

	Texture2D popup_background("./Content/Textures/popup_background.png");
	popup_background.setTexMinMagFilter(GL_NEAREST, GL_NEAREST);
	//setup instance rendering
	Model Sphere("./Content/Models/sphere.obj");
	Sphere.setModel();
	glm::mat4 wifi_transform(1);
	wifi_transform = glm::scale(wifi_transform, glm::vec3(1,1,1));
	glm::mat4 glyph_transform(1);
	glyph_transform = glm::scale(glyph_transform, glm::vec3(1));
	bool updated_routers = true;

	//setup hand model
	Model LeftHand("./Content/Models/controller_l.fbx");
	LeftHand.setModel();
	Model RightHand("./Content/Models/controller_r.fbx");
	RightHand.setModel();

	Model Cylinder("./Content/Models/cylinder.obj");
	Cylinder.setModel();

	//Setup ground rendering
	Model quad("./Content/Models/quad/quad_centered.obj");
	quad.setModel();
	glm::mat4 ground_transform(1);
	ground_transform = glm::scale(ground_transform, glm::vec3(15, 19, 2));
	ground_transform = glm::translate(ground_transform, glm::vec3(0, 0, -.101));
	//quad.getMeshes().at(0)->setTexture(Texture2D(glm::vec4(50, 50, 50, 255) * (1 / 255.0f)), 0);
	quad.getMeshes().at(0)->setTexture(popup_background, 0);
	

	//Setup volume rendering
	Model screen_quad("./Content/Models/quad/quad_centered.obj");
	screen_quad.setModel();
	float stepSize = .01;
	
	Model BoundingSphere = Model("./Content/Models/cube/cube.obj");
	BoundingSphere.setModel();
	glm::vec3 bounding_cube_translate = glm::vec3(-1.25, .588, 2.235);
	glm::vec3 bounding_cube_scale(23, 33.588, 5);
	glm::mat4 bounding_cube_transform(1);
	bounding_cube_transform = glm::scale(bounding_cube_transform, bounding_cube_scale);
	bounding_cube_transform = glm::translate(bounding_cube_transform, bounding_cube_translate);

	//Setup Camera
	//Camera camera = Camera(glm::vec3(-11.54, 11.6, .4575),glm::vec3(-10.54, 11.6, .4575) , 60.0f, w.width / (float)w.height);
	//Camera camera = Camera(glm::vec3(.467506, 11.501509, 2.478917),glm::vec3(.467506 - .970493, 11.501509 + .020823, 2.478917 - .240227) , 60.0f, w.width / (float)w.height);
	Camera camera = Camera(glm::vec3(-.032911, 11.68143, 2.214213), glm::vec3(-.032911, 11.68143, 2.214213) + glm::vec3(.89027, -.430448, -.090891), 60.0f, w.width / (float)w.height);
	w.SetCamera(&camera);
	w.setSpeed(1);



	//Setup Light
	Lights lights = Lights();
	lights.addDirLight(glm::vec3(-1, .2, .2), glm::vec3(1, 1, 1));

	//Model Lights
	Lights modelLights = setPointLights(20, 1, .1, .01);
	int totalLights = 100;
	int numLights = 20;
	//modelLights.addPointLight(glm::vec3(-15, 15, 0), .7, glm::vec3(1));
	//modelLights.addPointLight(glm::vec3(0, 15, -15), .7, glm::vec3(1));
	//modelLights.addPointLight(glm::vec3(0,0,0), .2, glm::vec3(1,1,1));

	std::map<std::string, float> deferred_shading_floats = {
		{"ambient_coeff", .2f},
		{"spec_coeff", .0},
		{"diffuse_coeff", .7f},
	}, instance_shading_floats = {
		{"ambient_coeff", .3f},
		{"spec_coeff", .4},
		{"diffuse_coeff", .3f},
	}, lic_shading_floats;
	std::map<std::string, int> deferred_shading_ints = { 
		{"shininess", 32} 
	}, instance_shading_ints = {
		{"shininess", 16}
	}, lic_shading_ints;
	

	//send light params to model shader
	deferred_shader.SetUniforms(deferred_shading_floats);
	deferred_shader.SetUniforms(deferred_shading_ints);

	//send light params to instance shader
	instance_shader.SetUniforms(instance_shading_floats);
	instance_shader.SetUniforms(instance_shading_ints);

	
	float constant = 1, linear = .9, quadratic = .98;
	
	int odd_or_even_frame = 0;


	//load avw wifi data
	ShaderProgram router_shaders[2] = { deferred_shader, force_shader };

	AVWWifiData wifi(router_shaders, 2);
	Ellipsoid ellipsoid;
	wifi.loadEllipsoid("./Content/Data/one_router.elipsoid", ellipsoid);
	

	VolumeData volume;

	wifi.loadWifi("./Content/Data/AVW1.txt", "1");
	wifi.loadWifi("./Content/Data/AVW2.txt", "2");
	wifi.loadWifi("./Content/Data/AVW3.txt", "3");
	wifi.loadWifi("./Content/Data/AVW4.txt", "4");
	wifi.pruneEntries();
	wifi.setupStructures();
	std::vector<glm::vec4> wifi_colors(wifi.getNumWifiNames());
	maxSeperatedColors(wifi.getNumWifiNames(), wifi_colors);

	Texture2D wifi_tex = Texture2D(&wifi_colors, wifi_colors.size(), 1);
	wifi_tex.setTexMinMagFilter(GL_NEAREST, GL_NEAREST);
	wifi_tex.setTexParameterWrap(GL_CLAMP, GL_CLAMP);
	std::vector<glm::mat4> wifi_transforms;
	wifi_tex.giveName("wifi_colors");
	Sphere.getMeshes().at(0)->setTexture(wifi_tex, 0);
	Texture2D frequency_texture = Texture2D("./Content/Textures/texton_paper_aspect.png");
	frequency_texture.giveName("frequency_tex");
	float an = 0.0f;
	
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &an); 
	std::cout << an << std::endl;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, an);
	frequency_texture.setTexMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	frequency_texture.setTexParameterWrap(GL_REPEAT);
	//AVW.getMeshes().at(0)->addTexture(frequency_texture);
	quad.getMeshes().at(0)->addTexture(frequency_texture);


	//std::vector<glm::vec4> heatmap = std::vector<glm::vec4>(2);
	//heatmap[0] = glm::vec4(1, 1, 1, 1);
	//heatmap[1] = glm::vec4(0, 0, 0, 1);
	//Texture2D heatmap_tex = Texture2D(&heatmap, 2, 1);
	//heatmap_tex.giveName("ellipsoid_tex");
	//heatmap_tex.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
	//heatmap_tex.setTexParameterWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
	//AVW.getMeshes().at(0)->addTexture(heatmap_tex);
	//AVW.getMeshes().at(0)->addTexture(wifi_tex);


	//Setup ImGUI variables
	glm::vec3 wifi_scale = glm::vec3(30, 34.792, 1);
	glm::vec3 wifi_translate = glm::vec3(-15., -16.042, -.833);
	static std::vector<bool> wifinames(wifi.getNumWifiNames());
	std::fill(wifinames.begin(), wifinames.end(), true);
	wifi.setAvailableFreqs(wifi.getSelectedNames(wifinames));
	wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));
	
	static std::vector<bool> routers(wifi.getAvailablesMacs().size());
	float transparency = 1;

	std::fill(routers.begin(), routers.end(), false);

	static std::vector<bool> freqs(wifi.getAvailableFreqs().size());
	std::fill(freqs.begin(), freqs.end(), true);

	glEnable(GL_DEPTH_TEST);


	gBuffer buffer[1];
	eyeBuffer eyes[2];
	createFramebuffers(resolution, buffer, eyes, use_vr, false);
	quad.getMeshes().at(0)->addTexture(buffer[0].color_texture);
	quad.getMeshes().at(0)->addTexture(buffer[0].frag_pos_texture);
	quad.getMeshes().at(0)->addTexture(buffer[0].normal_texture);
	quad.getMeshes().at(0)->addTexture(buffer[0].ellipsoid_coordinates_texture);
	quad.getMeshes().at(0)->addTexture(buffer[0].tangent_texture);
	quad.getMeshes().at(0)->addTexture(buffer[0].force_texture);
	quad.getMeshes().at(0)->addTexture(buffer[0].lic_texture[0]);
	quad.getMeshes().at(0)->addTexture(buffer[0].lic_texture[1]);
	quad.getMeshes().at(0)->addTexture(eyes[0].screenTexture);
	quad.getMeshes().at(0)->addTexture(wifi_tex);

	int num_fb = 1;
	if (use_vr)
		num_fb = 2;
	//main render loop
	
	vr::Hmd_Eye curr_eye = vr::Hmd_Eye::Eye_Left;
	glm::mat4 ProjectionMat, ViewMat;

	std::map<std::string, glm::vec4> saved_colors;
	int old_num_colors = -1;

	float linear_term = 1, thickness = .056, distance_mask = 0, billboard_scale = .4;
	
	bool send_data = false, polling_pbo = false, renderPopup = false;
	std::vector<std::string> popup_text;

	int num_routers = 1;
	int num_samples = 102400;
	
	glm::uvec2 lic_texture_res(2048 , 2048);
	int old_num_samples = num_samples;

	LineIntegralConvolution lic(lic_texture_res); 
	lic.fillWithNoise(num_samples);
	//lic.convolve(fall_off);
	Texture2D noise[20];
	noise[0] = Texture2D(lic.getTexture().data(), lic_texture_res.y, lic_texture_res.x);
	noise[0].setTexMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	noise[0].setTexParameterWrap(GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, an);
	
	noise[0].giveName("noise_tex");
	quad.getMeshes().at(0)->setTexture(noise[0]);

	glm::vec4* fragPosArray = (glm::vec4*)malloc(sizeof(glm::vec4) * resolution.x * resolution.y);
	if (!fragPosArray) {
		std::cerr << "Malloc Error" << std::endl;
	}
	GLsync block = 0;
	deferred_shading_floats.clear();
	deferred_shading_floats = {
		{ "extent", 1 },
		{ "frequency", .973 },
		{ "linear_term", 1 },
		{ "thickness", .056 },
		{ "u_stretch", 10 },
		{ "v_stretch", 10 },
		{ "delta_theta", 180.f / wifi.getActiveFreqs(freqs).size() },
		{ "learning_rate", .05 },
		{ "distance_mask", 0 },
		{ "alpha_boost", 20 },
		{ "density", .025 },
		{ "frag_pos_scale", 100},
		{ "cling", .9},
		{ "tunable", .005}
	};
	lic_shading_floats = {
		{ "alpha_boost", 20 },
		{ "density", .025 },
		{ "frag_pos_scale", 100},
		{ "learning_rate", .05},
	};
	std::map<std::string, bool> deferred_shading_bools = {
		{ "contour_on", false},
		{ "display_names", false },
		{ "lic_on", true },
		{ "group_frequencies", false },
		{ "texton_background", false },
		{ "invert_colors", false },
		{ "frequency_bands", false },
		{ "shade_instances", false},
		{ "anti_aliasing", true},
		{ "use_mask", true},
		{ "screen_space_lic", true},
		{ "cull_discontinuities", true},
		{ "multirouter", true},
		{ "color_weaving", true},
		{"blending", true}
	}, lic_shading_bools = {
		{ "screen_space_lic", true},
		{ "procedural_noise", false},
		{ "cull_discontinuities", true},
		{ "use_mask", true}, 
		{ "multirouter", true}
	};
	deferred_shading_ints = {
		{"num_point_lights", 20},
		{"num_frequencies", wifi.getActiveFreqs(freqs).size()},
		{"num_contours", 6},
		{"power", 1}
	}, lic_shading_ints = {
		{ "num_ellpsoids", num_routers},
		{ "power", 1}
	};
	const int num_antialiased_textures = 23;
	Texture2D antialiased_textures[num_antialiased_textures] = {
		frequency_texture,
		//buffer[0].lic_texture[0], buffer[0].lic_texture[0],
		noise[0]};
	bool num_samples_changed = false, num_routers_changed = false;
	while (!glfwWindowShouldClose(w.getWindow())) {
		if (w.getResized()) {
			resolution.x = w.width;
			resolution.y = w.height;
			createFramebuffers(resolution, buffer, eyes, use_vr, true);
			w.setResized(false);
		}
		//clear default framebuffer
		if (use_vr) {
			vr.SaveControllerIDs();
			vr.updateHMDPoseMatrix();
		}
		for (int i = 0; i < num_fb; i++) {
			if (i == 0) {
				curr_eye = vr::Hmd_Eye::Eye_Left;
			}
			else {
				curr_eye = vr::Hmd_Eye::Eye_Right;
			}
			ProjectionMat = camera.getProjection();
			ViewMat = camera.getView();
			if (use_vr) {
				ProjectionMat = vr.getProjectionMatrix(curr_eye);
				ViewMat = vr.getViewMatrix(curr_eye) * camera_offset * ViewMat;

			}
			glBindFramebuffer(GL_FRAMEBUFFER, eyes[i].frame_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//render skybox
			skybox_shader.Use();
			skybox_shader.SetUniform("projection", ProjectionMat);
			skybox_shader.SetUniform("view", glm::mat4(glm::mat3(ViewMat)));

			glDepthMask(GL_FALSE);
			render(skybox, &skybox_shader);
			glDepthMask(GL_TRUE);

			//render ground plane
			ground_shader.SetUniform("projection", ProjectionMat);
			ground_shader.SetUniform("camera", ViewMat);
			ground_shader.SetUniform("model", ground_transform);
			ground_shader.SetUniform("heatmap", 0);
			render(quad, &ground_shader);

			//render building model
			glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].frame_buffer);

			if (i == 0 && w.signal && !polling_pbo) {
				glReadBuffer(GL_COLOR_ATTACHMENT2);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[odd_or_even_frame]);
				glReadPixels(0, 0, resolution.x, resolution.y, GL_BGRA, GL_FLOAT, 0);
				block = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
				//odd_or_even_frame = (odd_or_even_frame + 1) % 2;
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				polling_pbo = true;
			}
			else if (i == 0 && polling_pbo && glClientWaitSync(block, GL_SYNC_FLUSH_COMMANDS_BIT, 0)) {
				glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[int(odd_or_even_frame)]);
				fragPosArray = (glm::vec4*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

				//fragPosArray = (glm::vec4*)src;

				if (fragPosArray) {
					glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
				}
				polling_pbo = false;
				w.signal = false;
				send_data = true;
			}

			glBindFramebuffer(GL_READ_FRAMEBUFFER, eyes[i].frame_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			model_shader.Use();
			model_shader.SetUniform("model", avw_transform);
			model_shader.SetUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(avw_transform))));
			model_shader.SetUniform("camera", ViewMat);
			model_shader.SetUniform("projection", ProjectionMat);
			Lights modelLights = setPointLights(totalLights, constant, linear, quadratic);
			model_shader.SetUniform("viewPos", camera.getPosition());
			model_shader.SetUniform("distance_mask", deferred_shading_floats["distance_mask"]);

			//glDepthMask(GL_FALSE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			render(AVW, &model_shader);
			glDisable(GL_CULL_FACE);
			//glDepthMask(GL_TRUE);
			//glClear(GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].lic_framebuffer[0]);
			glClear(GL_COLOR_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].lic_framebuffer[1]);
			glClear(GL_COLOR_BUFFER_BIT);
			int num_forces = num_routers;
			int num_iters = 4;
			if (lic_shading_bools["multirouter"])
				num_forces = 1;
			for (int i = 0; i < num_forces; i++) {
				
				force_shader.Use();
				glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].force_framebuffer);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				force_shader.SetUniform("ellipsoid_transform", wifi.ellipsoid_transform);
				force_shader.SetUniform("radius_stretch", wifi.radius_stretch);
				force_shader.SetUniform("ellipsoid_index_offset", i);

				force_shader.SetUniform("num_ellipsoids", num_routers/num_forces);
				force_shader.SetUniform("extent", 1.0f);

				render(quad, &force_shader);
				
				for (int j = 0; j < num_iters; j++) {
					lic_shader.Use();

					lic_shader.SetUniforms(lic_shading_floats);
					lic_shader.SetUniforms(lic_shading_ints);
					lic_shader.SetUniforms(lic_shading_bools);
					lic_shader.SetUniform("camera", ViewMat);
					lic_shader.SetUniform("projection", ProjectionMat);
					lic_shader.SetUniform("color", glm::vec3(wifi_colors.at(i)));
					lic_shader.SetUniform("ellipsoid_index_offset", i);
					lic_shader.SetUniform("router_num", i);
					lic_shader.SetUniform("step_num", j);

					glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].lic_framebuffer[j%2]);
					glClear(GL_DEPTH_BUFFER_BIT);
					render(quad, &lic_shader);
					//if (j == 0) {
					//	noise[i].giveName("temp_name");
					//}
					//buffer[0].lic_texture.giveName("noise_tex[" + std::to_string(i) + "]");
				}
				//noise[i].giveName("noise_tex[" + std::to_string(i) + "]");
				//buffer[0].lic_texture.giveName("lic_tex");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].lic_accum_framebuffer);

			deferred_shader.Use();
			deferred_shader.SetEllipsoid(ellipsoid);
			deferred_shader.SetUniforms(deferred_shading_floats);
			deferred_shader.SetUniforms(deferred_shading_ints);
			deferred_shader.SetUniforms(deferred_shading_bools);
			deferred_shader.SetUniform("camera", ViewMat);
			deferred_shader.SetUniform("projection", ProjectionMat);
			deferred_shader.SetUniform("num_lic", num_iters % 2);
			deferred_shader.SetUniform("alpha_boost", lic_shading_floats["alpha_boost"]);
			deferred_shader.SetUniform("power", lic_shading_ints["power"]);
			wifi_transform = glm::translate(glm::mat4(1), wifi_translate);
			wifi.ellipsoid_transform = wifi_transform * glm::scale(glm::mat4(1), wifi_scale);
			deferred_shader.SetUniform("ellipsoid_transform", wifi.ellipsoid_transform);
			wifi.radius_stretch = wifi_scale;
			

			deferred_shader.SetUniform("radius_stretch", wifi.radius_stretch);
			deferred_shader.SetUniform("viewPos", camera.getPosition());
			
			glm::vec3 selectedPos;
			if (send_data) {
				glm::vec2 index = glm::ivec2(w.currX, (resolution.y - w.currY));
				selectedPos = fragPosArray[int(index.y * resolution.x + index.x)];
				selectedPos = glm::vec3(selectedPos.b, selectedPos.g, selectedPos.r);
				deferred_shader.SetUniform("selectedPos", selectedPos);
				renderPopup = wifi.getNumActiveRouters(routers) > 0;
				wifi.setRenderText(popup_text, selectedPos, routers);
				std::cout << glm::to_string(selectedPos) << std::endl;
				send_data = false;
				for (int i = 0; i < popup_text.size(); i++) {
					popupText.RenderText(glm::uvec2(0, font_height * popup_text.size() - (i + 1) * font_height),
						glm::uvec2(2560, popup_text.size() * font_height),
						popup_text.at(i), i == 0);
				}
				
				Texture2D text(popupText.tex, popupText.height, popupText.width);
				text.giveName("quad_texture");
				text.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
				quad.getMeshes().at(0)->setTexture(text);
				//std::cout << glm::to_string(fragPosArray[int(index.y * resolution.x + index.x)]) << std::endl;
			}
			deferred_shader.SetLights(modelLights, camera.getPosition(), deferred_shading_ints["num_point_lights"]);

			


			glBindFramebuffer(GL_FRAMEBUFFER, eyes[i].frame_buffer);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDepthMask(GL_FALSE);
 			render(quad, &deferred_shader);
			glDepthMask(GL_TRUE);


			if (num_samples_changed || num_routers_changed) {
				updateNoise(lic, num_samples, num_routers, noise, antialiased_textures, lic_texture_res, an);
				for (int i = 0; i < num_routers; i++) {
					quad.getMeshes().at(0)->setTexture(noise[i]);
				}
				num_samples_changed = false;
				num_routers_changed = false;
			}
			
			if (renderPopup) {
				glm::mat4 quad_transform(1);
				glm::vec3 pos = glm::lerp(selectedPos, camera.getPosition(), .1f);
				quad_transform = glm::translate(quad_transform, pos);
				quad_shader.SetUniform("camera", ViewMat);
				quad_shader.SetUniform("projection", ProjectionMat);
				quad_shader.SetUniform("quad_center", pos);
				quad_shader.SetUniform("billboardSize", glm::vec2(popupText.width/(float)resolution.x, popupText.height/(float)resolution.y) * billboard_scale);
				quad_shader.SetUniform("num_routers", wifi.getNumActiveRouters(routers));
				quad_shader.SetUniform("model", quad_transform);
				render(quad, &quad_shader);
			}

			if (updated_routers) {
				wifi.updateRouterStructure(routers, wifinames, freqs, router_shaders, 2, nearest_router_on);
				updated_routers = false;
				if (nearest_router_on) {
					std::vector<glm::vec4> new_wifi_colors(wifi.getNumActiveRouters(routers));
					std::vector<glm::vec4> new_wifi_colors_rearranged(wifi.getNumActiveRouters(routers));
					std::vector<int> vec_indices(wifi.getNumActiveRouters(routers));
					std::vector<int> unused_indices(wifi.getNumActiveRouters(routers));
					for (int i = 0; i < unused_indices.size(); i++) { unused_indices[i] = i; }
					maxSeperatedColors(wifi.getNumActiveRouters(routers), new_wifi_colors, jittered);
					std::vector<std::string> available_macs = wifi.getAvailablesMacs();
					//Remember collors between updates
					if (saved_colors.size() == num_routers) {
						int count = 0;
						int used_router_num = 0;
						std::vector<std::string> router_names(saved_colors.size());
						for (auto router : routers) {
							if (router) {
								router_names[used_router_num] = available_macs[count];
								auto map_location = saved_colors.find(router_names[used_router_num]);
								if (map_location != saved_colors.end()) {
									vec_indices[used_router_num] = std::distance(saved_colors.begin(), map_location);
									auto location = std::find(unused_indices.begin(), unused_indices.end(), vec_indices[used_router_num]);
									unused_indices.erase(location);
								}
								else {
									vec_indices[used_router_num] = -1;
								}
								used_router_num++;
							}
							count++;
						}
						
						std::vector<std::string>old_names;
						for (auto name2color : saved_colors)
							old_names.push_back(name2color.first);
						for (int i = 0; i < vec_indices.size(); i++) {
							if (vec_indices[i] == -1) { vec_indices[i] = unused_indices.back(); unused_indices.pop_back(); }
							new_wifi_colors_rearranged[i] = saved_colors[old_names[vec_indices[i]]];
						}
						
					}
					else {
						new_wifi_colors_rearranged = new_wifi_colors;
					}
					saved_colors.clear();
					int i = 0, count = 0;
					for (auto router : routers) {
						if (router) {
							saved_colors[available_macs[count]] = new_wifi_colors_rearranged[i++];
						}
						count++;
					}
					wifi_colors.resize(wifi.getNumActiveRouters(routers) + wifi.getNumWifiNames());
					for (int i = 0; i < new_wifi_colors.size(); i++) {
						wifi_colors[wifi.getNumWifiNames() + i] = new_wifi_colors_rearranged[i];
					}
					wifi_tex.updateTexture(&wifi_colors, wifi.getNumActiveRouters(routers) + wifi.getNumWifiNames(), 1);
					auto router_strings = wifi.getRouterStrings();
					for (int i = 0; i < router_strings.size(); i++) {
						tr.RenderText(glm::uvec2(0, font_height * router_strings.size() - (i + 1) * font_height), glm::uvec2(2560, router_strings.size() * font_height), router_strings[router_strings.size() - i - 1], i==0);
					}

					Texture2D text(tr.tex, tr.height, tr.width);
					text.giveName("text_tex");
					quad.getMeshes().at(0)->setTexture(text);
				}
			}
			

			//render wifi instances
			instance_shader.Use();
			wifi_transforms.clear();
			if (deferred_shading_bools["shade_instances"])
				wifi_transforms = wifi.getTransforms(wifinames, routers, wifi_scale);

			std::vector<float> wifi_color_indices = wifi.getColorIndices();
			if (deferred_shading_bools["shade_instances"])
				Sphere.getMeshes().at(0)->SetInstanceTransforms(wifi_transforms, wifi_color_indices);
			instance_shader.SetUniform("projection", ProjectionMat);
			instance_shader.SetUniform("view", ViewMat);
			//wifi_transform = glm::scale(glm::mat4(1), wifi_scale);

			instance_shader.SetUniform("transform", wifi_transform);
			instance_shader.SetUniform("model_transform", glyph_transform);
			instance_shader.SetUniform("normalMat", glm::mat3(1));
			instance_shader.SetLights(lights);
			instance_shader.SetUniform("viewPos", camera.getPosition());

			glBindFramebuffer(GL_FRAMEBUFFER, eyes[i].frame_buffer);
			if (wifi_transforms.size() > 0 && deferred_shading_bools["shade_instances"])
				render(Sphere, &instance_shader);
			if (use_vr) {
				ground_shader.Use();
				ground_shader.SetUniform("camera", vr.getViewMatrix(curr_eye));
				glm::mat4 controller_rotation = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1), glm::radians(60.0f), glm::vec3(1, 0, 0));
				ground_shader.SetUniform("model", vr.getControllerPose(vr.LeftDeviceId) * controller_rotation * glm::scale(glm::mat4(1), .01f * glm::vec3(-1, -1, 1)));
				render(LeftHand, &ground_shader);
				glm::mat4 right_hand_transform = vr.getControllerPose(vr.RightDeviceId)  * controller_rotation * glm::scale(glm::mat4(1), 1.0f * glm::vec3(-1, -1, 1));
				ground_shader.SetUniform("model", right_hand_transform * glm::scale(glm::mat4(1), .01f * glm::vec3(1,1,1)));
				render(RightHand, &ground_shader);
				if (i == 0) {
					if (vr.ray_picker_enable) {
						//glm::vec3 forward = vr.getControllerPose(vr.RightDeviceId) *controller_rotation* glm::vec4(-1, 0, 0,0);
						glm::vec3 forward, pos2;
						glm::quat  orientation;
						glm::vec3 skew, scale;
						glm::vec4 perspective;
						//std::cout << glm::to_string(forward) << std::endl;
						//std::cout << glm::to_string(camera.getDirection()) << std::endl;;
						glm::vec3 position;

						glm::decompose(vr.getControllerPose(vr.RightDeviceId), scale, orientation, position, skew, perspective);
						auto inv_view = glm::inverse(camera_offset * camera.getView());
						position = glm::vec3(inv_view * glm::vec4(position, 1));
				
						forward = glm::conjugate(orientation) * glm::vec3(inv_view * glm::vec4(0, 0,-1, 0));
						//position = glm::vec3(position.b, position.r, -position.g);
						rayPicker(forward, position, glm::vec3(0), vr.teleport_position);
						//vr.teleport_position.z = camera.getPosition().z;
						//camera.setPosition(vr.teleport_position);
						
						//vr.teleport_position = glm::vec3(1, 1, 1) * position;
						deferred_shader.SetUniform("selectedPos", glm::vec3(vr.teleport_position.r, vr.teleport_position.g, vr.teleport_position.b));
						vr.ray_picker_enable = false;
					}
				}
				right_hand_transform = right_hand_transform * 
					glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(1, 0, 0)) * 
					glm::scale(glm::mat4(1), glm::vec3(-1, -100, -1));
				ground_shader.SetUniform("model", right_hand_transform * glm::scale(glm::mat4(1), .01f * glm::vec3(1, 1, 1)));

				render(Cylinder, &ground_shader);
				vr.composite(curr_eye, eyes[i].screen_tex);
			}
		}
		glFlush(); 
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render2quad.Use();
		render(quad, &render2quad);
		
		//Render ImGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Rendering Terms");
			ImGui::SliderInt("Number of Routers", &num_routers, 1, 20);
			if (ImGui::Button("Nearest Routers")) {
				num_routers_changed = true;
				std::fill(freqs.begin(), freqs.end(), true);
				wifi.setNearestNRouters(num_routers, camera.getPosition(), wifinames, routers, freqs);
				deferred_shading_floats["delta_theta"] = 180.f / wifi.getActiveFreqs(freqs).size();
				updated_routers = true;
				nearest_router_on = true;
			}
			ImGui::SliderInt("Total Lights", &totalLights, 1, 200);
			ImGui::SliderInt("Lights Shown", &deferred_shading_ints["num_point_lights"], 1, totalLights);
			ImGui::SliderFloat("constant", &constant, 0, 1);
			ImGui::SliderFloat("linear", &linear, 0, 1);
			ImGui::SliderFloat("quadratic", &quadratic, 0, 1);
			//ImGui::SliderFloat("extent", &deferred_shading_floats["extent"], 0, 3);
			ImGui::SliderFloat("BillBoard Scale", &billboard_scale, 0, 1);
			ImGui::Checkbox("Shade Instances", &deferred_shading_bools["shade_instances"]);
			ImGui::Checkbox("Line Integral Convolution", &deferred_shading_bools["lic_on"]);
			if (deferred_shading_bools["lic_on"]) {
				ImGui::SliderFloat("Alpha_Boost", &lic_shading_floats["alpha_boost"], 1, 30);
				ImGui::SliderInt("Power", &lic_shading_ints["power"], 1, 8);
				ImGui::Checkbox("Screenspcace LIC", &lic_shading_bools["screen_space_lic"]);
				if (deferred_shading_bools["screen_space_lic"]) {
					ImGui::SliderFloat("Vector Threshold", &lic_shading_floats["tunable"], 0, 3);
					ImGui::Checkbox("Hug Walls", &lic_shading_bools["cull_discontinuities"]);
				}
				ImGui::Checkbox("Multirouter", &lic_shading_bools["multirouter"]);
				if(lic_shading_bools["multirouter"]) {
					ImGui::Checkbox("Use Color Weaving", &deferred_shading_bools["color_weaving"]);
					if (deferred_shading_bools["color_weaving"]) {
						ImGui::Checkbox("Use Blending", &deferred_shading_bools["blending"]);
					}
				}
				ImGui::Checkbox("Procedural Noise", &lic_shading_bools["procedural_noise"]);
				if (lic_shading_bools["procedural_noise"]) {
					ImGui::SliderFloat("Density", &lic_shading_floats["density"], 0, 1);
					if (!lic_shading_bools["screen_space_lic"])
						ImGui::SliderFloat("Cling Factor", &deferred_shading_floats["cling"], 0, 1);
				}else {
					num_samples_changed = ImGui::SliderInt("num_samples", &num_samples, 12800, 102400 * 16);
				}
				ImGui::SliderFloat("Fragment Position Scale", &lic_shading_floats["frag_pos_scale"], 0, 200);
				ImGui::SliderFloat("Rate", &lic_shading_floats["learning_rate"], 0, .9);
				ImGui::Checkbox("Use LIC Mask", &lic_shading_bools["use_mask"]);
				
			}
			else {
				ImGui::Checkbox("frequency_bands", &deferred_shading_bools["frequency_bands"]);
				ImGui::Checkbox("Display Names", &deferred_shading_bools["display_names"]);
				if (deferred_shading_bools["display_names"]) {
					ImGui::SliderInt("Number of Dashes", &deferred_shading_ints["num_contours"], 1, 20);
				}
				ImGui::SliderFloat("Contour Frequency", &deferred_shading_floats["frequency"], 0, 1);
				ImGui::SliderFloat("Linear Term", &deferred_shading_floats["linear_term"], 0, 1);
				ImGui::SliderFloat("thickness", &deferred_shading_floats["thickness"], 0, .1);
			}
			ImGui::Checkbox("Invert Color Representation", &deferred_shading_bools["invert_colors"]);
			ImGui::Checkbox("texton_background", &deferred_shading_bools["texton_background"]);

			if (ImGui::Checkbox("Antialiasing", &deferred_shading_bools["anti_aliasing"])) {
				for (int i = 0; i < num_routers + 1; i++) {
					Texture2D texture = antialiased_textures[i];
					texture.Bind();
					if (deferred_shading_bools["anti_aliasing"]) {
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, an);
						texture.setTexMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
					}
					else {
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);
						texture.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
					}
				}
			}
			ImGui::Checkbox("Jittered Colors", &jittered);
			ImGui::SliderFloat("u stretch", &deferred_shading_floats["u_stretch"], 0, 10);
			ImGui::SliderFloat("v stretch", &deferred_shading_floats["v_stretch"], 0, 10);
			ImGui::SliderFloat("distance mask", &deferred_shading_floats["distance_mask"], 0, 3);
			//ImGui::SliderFloat("Z Boost", &z_boost, 1, 10);
			if (ImGui::TreeNode("Wifi Names")) {
				for (int i = 0; i < wifinames.size(); i++) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(wifi_colors.at(i).r, wifi_colors.at(i).g, wifi_colors.at(i).b, wifi_colors.at(i).a));
					if (ImGui::Selectable(wifi.getWifiName(i).c_str(), wifinames.at(i)))
					{
						if (!ImGui::GetIO().KeyCtrl) {    // Clear selection when CTRL is not held
							std::fill(wifinames.begin(), wifinames.end(), false);
						}
						wifinames.at(i) = !wifinames.at(i);
						updated_routers = true;
						nearest_router_on = false;
						if (!wifinames.at(i))
							wifi.fillRouters(wifi.getWifiName(i).c_str(), routers, false);
						else
							wifi.fillRouters(wifi.getWifiName(i).c_str(), routers, true);
						wifi.setAvailableFreqs(wifi.getSelectedNames(wifinames));
						std::fill(freqs.begin(), freqs.end(), true);
						wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));

					}
					ImGui::PopStyleColor();
				}
				ImGui::TreePop();
			}if (ImGui::TreeNode("Frequencies")) {
				for (int i = 0; i < wifi.getAvailableFreqs().size(); i++) {
					if (ImGui::Selectable(std::to_string(wifi.getAvailableFreqs().at(i)).c_str(), freqs.at(i))) {
						if (!ImGui::GetIO().KeyCtrl)    // Clear selection when CTRL is not held
							std::fill(freqs.begin(), freqs.end(), false);
						freqs.at(i) = !freqs.at(i);
						if (freqs.at(i)) {
							std::fill(routers.begin(), routers.end(), false);
						}
						updated_routers = true;
						nearest_router_on = false;
						wifi.setAvailableMacs(wifi.getSelectedNames(wifinames), wifi.getSelectedFreqs(freqs));
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Routers")) {

				for (int i = 0; i < wifi.getAvailablesMacs().size(); i++) {
					if (ImGui::Selectable(wifi.getAvailablesMacs().at(i).c_str(), routers.at(i)))
					{
						if (!ImGui::GetIO().KeyCtrl)    // Clear selection when CTRL is not held
							std::fill(routers.begin(), routers.end(), false);
						routers.at(i) = !routers.at(i);
						updated_routers = true;
						nearest_router_on = false;
					}
				}
				ImGui::TreePop();
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		if (nearest_router_on) {
			ImGui::Begin("Nearest Routers");
				for (int i = 0; i < wifi.getRouterStrings().size(); i++) {
					glm::vec4 color = wifi_colors[i + wifi.getNumWifiNames()];
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, 1));
					ImGui::Text(wifi.getRouterStrings().at(i).c_str());
					ImGui::PopStyleColor();
				}
			ImGui::End();
		}
		if (renderPopup) {
			ImGui::Begin("Ellipsoid Distances");
			for (int i = 0; i < popup_text.size(); i++) {
				glm::vec4 color = wifi_colors[i + wifi.getNumWifiNames()];
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, 1));
				ImGui::Text(popup_text.at(i).c_str());
				ImGui::PopStyleColor();
			}
			ImGui::End();
		}
		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (use_vr) {
			vr.handle_vr_input();
			glm::vec3 pos = camera.getPosition();
			pos.z += vr.adjusted_height;
			vr.adjusted_height = 0;
			camera.setPosition(pos);
		}
		w.ProcessFrame();
		if(use_vr)
			vr.handoff();
		glFinish();

	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	if(use_vr)
		vr.terminate();
	std::ofstream light_position_file("lightPositions.txt", std::ios::out);
	for (int i = 0; i < w.lightPositions.size(); i++) {
		light_position_file << glm::to_string(w.lightPositions.at(i)) << std::endl;
	}
	return 1;
}