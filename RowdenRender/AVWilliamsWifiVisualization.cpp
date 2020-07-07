#pragma once
#include "RowRender.h"
#include "WifiData.h"


#include <random>
#include <fstream>
#include <iostream>
#include <chrono>
#include <time.h>

#include <glm/gtc/matrix_access.hpp>


int AVWilliamsWifiVisualization() {
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
	glfwSwapInterval(0);

	//Load shaders
	ShaderProgram model_shader = ShaderProgram({ ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX });
	ShaderProgram skybox_shader = ShaderProgram({ ShaderProgram::Shaders::SKY_FRAG, ShaderProgram::Shaders::SKY_VERT });

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
	Model AVW("./Content/Models/AVW_model.obj");
	AVW.setModel();
	Texture2D white = Texture2D(Texture2D::COLORS::WHITE);
	AVW.getMeshes()[0]->setTexture(white, 0);
	glm::mat4 avw_transform(1);

	//Setup Camera
	Camera camera = Camera(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), 60.0f, w.width / (float)w.height);
	w.SetCamera(&camera);
	w.setSpeed(1);

	//Setup Light
	Lights lights = Lights();
	lights.addDirLight(glm::vec3(1, 0, 0), glm::vec3(1, 1, 1));

	model_shader.SetUniform1f("ambient_coeff", .5);
	model_shader.SetUniform1f("spec_coeff", .1);
	model_shader.SetUniform1f("diffuse_coeff", .4);
	model_shader.SetUniform1i("shininess", 32);

	glEnable(GL_DEPTH_TEST);
	//main render loop
	while (!glfwWindowShouldClose(w.getWindow())) {
		//clear default framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//event handling
		glfwPollEvents();

		//render skybox
		skybox_shader.SetUniform4fv("projection", camera.getProjection());
		skybox_shader.SetUniform4fv("view", glm::mat4(glm::mat3(camera.getView())));

		glDepthMask(GL_FALSE);
		render(skybox, &skybox_shader);
		glDepthMask(GL_TRUE);

		//render building model

		model_shader.SetUniform4fv("model", avw_transform);
		model_shader.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse(avw_transform))));
		model_shader.SetUniform4fv("camera", camera.getView());
		model_shader.SetUniform4fv("projection", camera.getProjection());
		model_shader.SetLights(lights);
		model_shader.SetUniform3f("viewPos", camera.getPosition());

		render(AVW, &model_shader);

		w.ProcessFrame();

		glFinish();
	}
	glfwTerminate();
	return 1;
}