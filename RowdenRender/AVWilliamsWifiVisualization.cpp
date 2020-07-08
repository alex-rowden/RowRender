#pragma once
#include "RowRender.h"
#include "WifiData.h"
#include "AVWWifiData.h"

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

	//Enable Transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Load shaders
	ShaderProgram model_shader = ShaderProgram({ ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX });
	ShaderProgram skybox_shader = ShaderProgram({ ShaderProgram::Shaders::SKY_FRAG, ShaderProgram::Shaders::SKY_VERT });
	ShaderProgram instance_shader = ShaderProgram({ ShaderProgram::Shaders::INSTANCE_FRAG, ShaderProgram::Shaders::INSTANCE_VERT });

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
	Texture2D white = Texture2D(glm::vec4(1,1,1,.2));
	AVW.getMeshes()[0]->setTexture(white, 0);
	glm::mat4 avw_transform(1);
	avw_transform = glm::rotate(avw_transform, glm::radians(90.f), glm::vec3(1, 0, 0));
	glm::vec3 avw_scale(2, 1, 2);
	avw_transform = glm::scale(avw_transform, avw_scale);

	//setup instance rendering
	Model Cube("./Content/Models/cube/cube.obj");
	Cube.setModel();
	glm::mat4 wifi_transform(1);
	wifi_transform = glm::scale(wifi_transform, glm::vec3(1,1,1));
	glm::mat4 glyph_transform(1);
	glyph_transform = glm::scale(glyph_transform, glm::vec3(.1, .1, .1));

	//Setup Camera
	Camera camera = Camera(glm::vec3(-15, 0, 8), glm::vec3(0, 0, 0), 60.0f, w.width / (float)w.height);
	w.SetCamera(&camera);
	w.setSpeed(1);

	//Setup Light
	Lights lights = Lights();
	lights.addDirLight(glm::vec3(-1, .2, .2), glm::vec3(1, 1, 1));

	model_shader.SetUniform1f("ambient_coeff", .3);
	model_shader.SetUniform1f("spec_coeff", .3);
	model_shader.SetUniform1f("diffuse_coeff", .4);
	model_shader.SetUniform1i("shininess", 32);

	//load avw wifi data
	AVWWifiData wifi;
	wifi.loadWifi("./Content/Data/AVW1.txt", "1");
	wifi.loadWifi("./Content/Data/AVW2.txt", "2");
	wifi.loadWifi("./Content/Data/AVW3.txt", "3");
	wifi.loadWifi("./Content/Data/AVW4.txt", "4");
	wifi.setWifiNames();

	//Setup ImGUI variables
	glm::vec3 wifi_scale = glm::vec3(30, 34.792, 1);
	glm::vec3 wifi_translate = glm::vec3(-15., -16.042, -.833);
	static std::vector<bool> wifinames(wifi.getNumWifiNames());
	wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));
	
	static std::vector<bool> routers(wifi.getAvailablesMacs().size());
	std::fill(wifinames.begin(), wifinames.end(), true);
	std::fill(routers.begin(), routers.end(), true);

	glEnable(GL_DEPTH_TEST);
	//main render loop
	while (!glfwWindowShouldClose(w.getWindow())) {
		//clear default framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//render skybox
		skybox_shader.Use();
		skybox_shader.SetUniform4fv("projection", camera.getProjection());
		skybox_shader.SetUniform4fv("view", glm::mat4(glm::mat3(camera.getView())));

		glDepthMask(GL_FALSE);
		render(skybox, &skybox_shader);
		glDepthMask(GL_TRUE);

		//render building model
		model_shader.Use();
		model_shader.SetUniform4fv("model", avw_transform);
		model_shader.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse(avw_transform))));
		model_shader.SetUniform4fv("camera", camera.getView());
		model_shader.SetUniform4fv("projection", camera.getProjection());
		model_shader.SetLights(lights);
		model_shader.SetUniform3f("viewPos", camera.getPosition());

		render(AVW, &model_shader);
		glClear(GL_DEPTH_BUFFER_BIT);
		//render wifi instances
		instance_shader.Use();
		Cube.getMeshes().at(0)->SetInstanceTransforms(wifi.getTransforms(wifinames, routers, wifi_scale));
		instance_shader.SetUniform4fv("projection", camera.getProjection());
		instance_shader.SetUniform4fv("view", camera.getView());
		//wifi_transform = glm::scale(glm::mat4(1), wifi_scale);
		wifi_transform = glm::translate(glm::mat4(1), wifi_translate);
		instance_shader.SetUniform4fv("transform", wifi_transform);
		instance_shader.SetUniform4fv("model_transform", glyph_transform);
		render(Cube, &instance_shader);

		//Render ImGUI

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Rendering Terms");
		ImGui::SliderFloat3("wifi scale", glm::value_ptr(wifi_scale), 20, 40);
		ImGui::SliderFloat3("wifi translate", glm::value_ptr(wifi_translate), 0, -20);
		if (ImGui::TreeNode("Wifi Names")) {
			for (int i = 0; i < wifinames.size(); i++) {
				if (ImGui::Selectable(wifi.getWifiName(i).c_str(), wifinames.at(i)))
				{
					if (!ImGui::GetIO().KeyCtrl) {    // Clear selection when CTRL is not held
						std::fill(wifinames.begin(), wifinames.end(), false);
					}
					wifinames.at(i) = !wifinames.at(i);
				}
			}
			ImGui::TreePop();
		}if (ImGui::TreeNode("Routers")) {
			wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));
			for (int i = 0; i < wifi.getAvailablesMacs().size(); i++) {
				if (ImGui::Selectable(wifi.getAvailablesMacs().at(i).c_str(), routers.at(i)))
				{
					if (!ImGui::GetIO().KeyCtrl)    // Clear selection when CTRL is not held
						std::fill(routers.begin(), routers.end(), false);
					routers.at(i) = !routers.at(i);
				}
			}
			ImGui::TreePop();
		}
		ImGui::End();
		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		w.ProcessFrame();

		glFinish();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 1;
}