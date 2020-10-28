#pragma once
#include "RowRender.h"
#include "WifiData.h"
#include "AVWWifiData.h"

#include <random>
#include <fstream>
#include <iostream>
#include <chrono>
#include <time.h>
#include <numeric>

#include <glm/gtc/matrix_access.hpp>

struct GBuffer {
	Texture2D normal_tex, albedo_tex, fragPos_tex, depth_tex, ssao_tex;
	GLuint framebuffer, matID, depth, ssao_buffer;

};

void createGbuffer(glm::vec2 resolution, GBuffer&gBuffer) {
	GLuint normal, albedo, fragPos, depth, ssao_val;

	glGenFramebuffers(1, &gBuffer.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.framebuffer);

	gBuffer.normal_tex = Texture2D();
	gBuffer.normal_tex.giveName("normal_tex");
	gBuffer.albedo_tex = Texture2D();
	gBuffer.albedo_tex.giveName("albedo_tex");
	gBuffer.fragPos_tex = Texture2D();
	gBuffer.fragPos_tex.giveName("fragPos_tex");
	gBuffer.depth_tex = Texture2D();
	gBuffer.depth_tex.giveName("depth");
	gBuffer.ssao_tex = Texture2D();
	gBuffer.ssao_tex.giveName("ssao_tex");

	// "Bind" the newly created texture : all future texture functions will modify this texture
	
	glGenTextures(1, &normal);
	glBindTexture(GL_TEXTURE_2D, normal);
	gBuffer.normal_tex.SetTextureID(normal);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normal, 0);

	glGenTextures(1, &albedo);
	glBindTexture(GL_TEXTURE_2D, albedo);
	gBuffer.albedo_tex.SetTextureID(albedo); 

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set "renderedTexture" as our colour attachement #0

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, albedo, 0);
	
	//setUp position texture
	glGenTextures(1, &fragPos);
	glBindTexture(GL_TEXTURE_2D, fragPos);
	gBuffer.fragPos_tex.SetTextureID(fragPos); 

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set "renderedTexture" as our colour attachement #0

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fragPos, 0);
	

	glGenTextures(1, &depth);
	glBindTexture(GL_TEXTURE_2D, depth);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, resolution.x, resolution.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
	gBuffer.depth_tex.SetTextureID(depth);
	// Set the list of draw buffers.
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, DrawBuffers); // "2" is the size of DrawBuffers

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glGenFramebuffers(1, &gBuffer.ssao_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.ssao_buffer);
	//setUp ssao texture
	glGenTextures(1, &ssao_val);
	glBindTexture(GL_TEXTURE_2D, ssao_val);
	gBuffer.ssao_tex.SetTextureID(ssao_val);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, resolution.x, resolution.y, 0, GL_RED, GL_FLOAT, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set "renderedTexture" as our colour attachement #0

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_val, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ssao framebuffer broke" << std::endl;
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}

int DefferedRenderingDemo() {
	glm::vec2 resolution = glm::vec2(2560, 1440);
	//initialize glfw
	glfwInit();
	glfwSetErrorCallback(error_callback);
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

	//set Clear Color
	glClearColor(0,0,0,0);

	//Initialize Shaders
	ShaderProgram prepass_shader({ ShaderProgram::Shaders::SCREEN_VERT, ShaderProgram::Shaders::PREPASS_SHADER });
	ShaderProgram deffered_rendering_shader({ ShaderProgram::Shaders::DEFFERED_RENDER_VERT, ShaderProgram::Shaders::DEFFERED_RENDER_FRAG });
	ShaderProgram ssao_shader({ ShaderProgram::Shaders::SSAO_FRAG, ShaderProgram::Shaders::SSAO_VERT });

	//Create GBuffer
	GBuffer gBuffer;
	createGbuffer(resolution, gBuffer);

	//Setup Cube
	Model Cube("./Content/Models/cube/cube.obj");
	Cube.setModel();
	Cube.getMeshes().at(0)->addTexture(gBuffer.albedo_tex);
	Cube.getMeshes().at(0)->addTexture(gBuffer.normal_tex);
	Cube.getMeshes().at(0)->addTexture(gBuffer.fragPos_tex);
	Cube.getMeshes().at(0)->addTexture(gBuffer.depth_tex);
	Cube.getMeshes().at(0)->addTexture(gBuffer.ssao_tex);

	//Setup Screen Quad
	Model Quad("./Content/Models/quad/quad_centered.obj");
	Quad.setModel();
	Quad.getMeshes().at(0)->addTexture(gBuffer.albedo_tex);
	Quad.getMeshes().at(0)->addTexture(gBuffer.normal_tex);
	Quad.getMeshes().at(0)->addTexture(gBuffer.fragPos_tex);
	Quad.getMeshes().at(0)->addTexture(gBuffer.depth_tex);
	Quad.getMeshes().at(0)->addTexture(gBuffer.ssao_tex);

	//Setup camera
	Camera camera = Camera(glm::vec3(0, -1, 0), glm::vec3(0, 0, 0), 60.0f, w.width / (float)w.height);
	w.SetCamera(&camera);
	w.setSpeed(1);

	//setup lights
	Lights lights = Lights();
	lights.addPointLight(glm::vec3(2, 2, 2), 1, .9, .2, glm::vec3(1), glm::vec3(1), glm::vec3(1));
	prepass_shader.SetUniform("num_point_lights", 1);

	prepass_shader.SetUniform("ambient_coeff", .2f);
	prepass_shader.SetUniform("spec_coeff", .2f);
	prepass_shader.SetUniform("diffuse_coeff", .6f);
	prepass_shader.SetUniform("shininess", 32);

	//setUp SSAO Kernel
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	//std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		float scale = (float)i / 64.0;
		scale = glm::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		//ssaoKernel.push_back(sample);
		ssao_shader.SetUniform(("samples[" + std::to_string(i) + "]").c_str(), sample);
	}
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}
	Texture2D ssaoNoise_tex;
	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	ssaoNoise_tex.SetTextureID(noiseTexture);
	ssaoNoise_tex.giveName("texNoise");

	Quad.getMeshes().at(0)->addTexture(ssaoNoise_tex);

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(w.getWindow())) {

		//render cube
		deffered_rendering_shader.Use();
		deffered_rendering_shader.SetUniform("model", glm::mat4(1));
		deffered_rendering_shader.SetUniform("normalMatrix", glm::mat3(1));
		deffered_rendering_shader.SetUniform("camera", camera.getView());
		deffered_rendering_shader.SetUniform("projection", camera.getProjection());
		
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render(Cube, &deffered_rendering_shader);
		glDisable(GL_CULL_FACE);


		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.ssao_buffer);
		glClear(GL_COLOR_BUFFER_BIT);
		

		ssao_shader.Use();
		ssao_shader.SetUniform("kernelSize", 64);
		ssao_shader.SetUniform("radius", .5f);
		ssao_shader.SetUniform("bias", .025f);
		ssao_shader.SetUniform("projection", camera.getProjection());
		ssao_shader.SetUniform("resolution", resolution);

		render(Quad, &ssao_shader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		prepass_shader.Use();
		prepass_shader.SetLights(lights);
		prepass_shader.SetUniform("viewPos", camera.getPosition());
		//glDisable(GL_DEPTH_TEST);
		render(Quad, &prepass_shader);

		w.ProcessFrame();

		glFinish();
	}

	glfwTerminate();
	return 1;
}


