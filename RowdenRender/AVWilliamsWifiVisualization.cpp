
#pragma once
#include "RowRender.h"
#include "WifiData.h"
#include "AVWWifiData.h"
#include "VR_Wrapper.h"

#include <random>
#include <fstream>
#include <iostream>
#include <chrono>
#include <time.h>
#include <numeric>

#include <glm/gtc/matrix_access.hpp>

using namespace vr;

GLuint fhp_tex, bhp_tex;
bool nearest_router_on = false;
bool jittered = true;
bool use_vr = false ;
struct gBuffer {
	GLuint frame_buffer, normal_tex, color_tex, frag_pos_tex, freq_mask_tex, depth_render_buf;
	Texture2D color_texture, frag_pos_texture, normal_texture, freq_mask_texture;
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
		out[i] = glm::vec4(hsv2rgb(glm::vec3(v, .7f, 1.f)), 1);
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

void createFramebuffer(glm::vec2 resolution, gBuffer* buffer) {
	glGenFramebuffers(1, &buffer->frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->frame_buffer);

	glGenTextures(1, &buffer->normal_tex);
	glBindTexture(GL_TEXTURE_2D, buffer->normal_tex);
	buffer->normal_texture = Texture2D();
	buffer->normal_texture.SetTextureID(buffer->normal_tex);
	buffer->normal_texture.giveName("normal_tex");
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->normal_tex, 0);	
	
	glGenTextures(1, &buffer->color_tex);
	glBindTexture(GL_TEXTURE_2D, buffer->color_tex);
	buffer->color_texture = Texture2D();
	buffer->color_texture.SetTextureID(buffer->color_tex);
	buffer->color_texture.giveName("albedo_tex");
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer->color_tex, 0);	
	
	glGenTextures(1, &buffer->frag_pos_tex);
	glBindTexture(GL_TEXTURE_2D, buffer->frag_pos_tex);
	buffer->frag_pos_texture = Texture2D();
	buffer->frag_pos_texture.SetTextureID(buffer->frag_pos_tex);
	buffer->frag_pos_texture.giveName("fragPos_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffer->frag_pos_tex, 0);
	
	glGenTextures(1, &buffer->freq_mask_tex);
	glBindTexture(GL_TEXTURE_2D, buffer->freq_mask_tex);
	buffer->freq_mask_texture = Texture2D();
	buffer->freq_mask_texture.SetTextureID(buffer->freq_mask_tex);
	buffer->freq_mask_texture.giveName("freq_mask_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, resolution.x, resolution.y, 0, GL_RED_INTEGER, GL_INT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, buffer->freq_mask_tex, 0);

	GLenum DrawBuffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, DrawBuffers); // "2" is the size of DrawBuffers

	glGenRenderbuffers(1, &buffer->depth_render_buf);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_render_buf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_render_buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createEyeFramebuffer(glm::vec2 resolution, eyeBuffer* buffer) {
	glGenFramebuffers(1, &buffer->frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->frame_buffer);

	glGenTextures(1, &buffer->screen_tex);
	glBindTexture(GL_TEXTURE_2D, buffer->screen_tex);
	buffer->screenTexture = Texture2D();
	buffer->screenTexture.SetTextureID(buffer->screen_tex);
	buffer->screenTexture.giveName("ScreenTexture");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->screen_tex, 0);

	glGenRenderbuffers(1, &buffer->depth_render_buf);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_render_buf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_render_buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createFramebuffers(glm::vec2 resolution, gBuffer *buffers, eyeBuffer *eyes, bool use_vr) {
	createFramebuffer(resolution, &buffers[0]);
	createEyeFramebuffer(resolution, &eyes[0]);
	if (use_vr)
		createEyeFramebuffer(resolution, &eyes[1]);
	eyes[0].screenTexture.giveName("screenTexture");
}


void rayPicker(glm::vec3 ray, glm::vec3 ray_coord, glm::vec3 camera_pos ,glm::vec3& intersection) {
	std::cout << glm::to_string(ray) << std::endl;
	float plane_height = 0;
	float z_coord = ray_coord.z + camera_pos.z;
	if(z_coord > .92)
		plane_height += 1.01;
	if (z_coord > 1.92)
		plane_height += 1.01;
	if (z_coord > 2.92)
		plane_height += 1.01;

	float t = (z_coord - plane_height) / ray.z;
	std::cout << t << std::endl;
	intersection = camera_pos + t * ray;
	//intersection = camera_pos + ray * .1f;
	return;
}


int AVWilliamsWifiVisualization() {

	
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
	int font_size = 64;
	tr.SetCharacterSize(font_size);
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
	ShaderProgram model_shader = ShaderProgram({ ShaderProgram::Shaders::FRAG_ELLIPSOID, ShaderProgram::Shaders::VERT_ELLIPSOID });
	ShaderProgram skybox_shader = ShaderProgram({ ShaderProgram::Shaders::SKY_FRAG, ShaderProgram::Shaders::SKY_VERT });
	ShaderProgram instance_shader = ShaderProgram({ ShaderProgram::Shaders::INSTANCE_FRAG_COLOR, ShaderProgram::Shaders::INSTANCE_VERT_COLOR });
	ShaderProgram ground_shader = ShaderProgram({ ShaderProgram::Shaders::NO_LIGHT_FRAG, ShaderProgram::Shaders::NO_LIGHT_VERT });
	ShaderProgram volume_shader = ShaderProgram({ShaderProgram::Shaders::VOLUME_FRAG_3D, ShaderProgram::Shaders::VOLUME_VERT_3D});
	ShaderProgram deferred_shader = ShaderProgram({ ShaderProgram::Shaders::DEFFERED_RENDER_ELLIPSOID_FRAG, ShaderProgram::Shaders::DEFFERED_RENDER_ELLIPSOID_VERT });
	ShaderProgram render2quad = ShaderProgram({ ShaderProgram::Shaders::SCREEN_FRAG, ShaderProgram::Shaders::SCREEN_VERT });
	
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
	Texture2D white = Texture2D(glm::vec4(1,1,1,.2));
	AVW.getMeshes()[0]->setTexture(white, 0);
	//AVW.getMeshes()[0]->addTexture(text);
	glm::mat4 avw_transform(1);
	avw_transform = glm::rotate(avw_transform, glm::radians(90.f), glm::vec3(1, 0, 0));
	glm::vec3 avw_scale(2, 1, 2);
	avw_transform = glm::scale(avw_transform, avw_scale);

	//setup instance rendering
	Model Sphere("./Content/Models/sphere.obj");
	Sphere.setModel();
	glm::mat4 wifi_transform(1);
	wifi_transform = glm::scale(wifi_transform, glm::vec3(1,1,1));
	glm::mat4 glyph_transform(1);
	glyph_transform = glm::scale(glyph_transform, glm::vec3(1));
	bool updated_routers = true;

	//setup hand model
	Model LeftHand("./Content/Models/cube/cube.obj");
	LeftHand.setModel();
	Model RightHand("./Content/Models/cube/cube.obj");
	RightHand.setModel();

	Model Cylinder("./Content/Models/cylinder.obj");
	Cylinder.setModel();

	//Setup ground rendering
	Model quad("./Content/Models/quad/quad_centered.obj");
	quad.setModel();
	glm::mat4 ground_transform(1);
	ground_transform = glm::scale(ground_transform, glm::vec3(15, 19, 2));
	ground_transform = glm::translate(ground_transform, glm::vec3(0, 0, -.101));
	quad.getMeshes().at(0)->setTexture(Texture2D(glm::vec4(50, 50, 50, 255) * (1 / 255.0f)), 0);
	//quad.getMeshes().at(0)->addTexture(text);
	

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
	Camera camera = Camera(glm::vec3(1.951824, 11.616767, 2.541506),glm::vec3(1.951824 - .932394, 11.616767 - .281266, 2.541506 - .227) , 60.0f, w.width / (float)w.height);
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

	

	//send light params to model shader
	deferred_shader.SetUniform1f("ambient_coeff", .2);
	deferred_shader.SetUniform1f("spec_coeff", 0);
	deferred_shader.SetUniform1f("diffuse_coeff", .7);
	deferred_shader.SetUniform1i("shininess", 32);

	//send light params to instance shader
	instance_shader.SetUniform1f("ambient_coeff", .3);
	instance_shader.SetUniform1f("spec_coeff", .4);
	instance_shader.SetUniform1f("diffuse_coeff", .3);
	instance_shader.SetUniform1i("shininess", 16);
	bool shade_instances = false;
	bool contour_on = false;
	bool display_names = false;
	float constant = 1, linear = .9, quadratic = .98;
	int num_routers = 6;

	//load avw wifi data
	AVWWifiData wifi(model_shader);
	Ellipsoid ellipsoid;
	wifi.loadEllipsoid("one_router.elipsoid", ellipsoid);
	float radius = 1;
	float frequency = 1.246;
	float extent = 1;
	float z_boost = 1;
	float theta = 0;
	float u_stretch = 1.157;
	float v_stretch = .567;
	int num_dashes = 6;

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

	Texture2D frequency_texture = Texture2D("./Content/Textures/pills.png");
	frequency_texture.giveName("frequency_tex");
	frequency_texture.setTexMinMagFilter(GL_NEAREST, GL_NEAREST);
	frequency_texture.setTexParameterWrap(GL_REPEAT);
	AVW.getMeshes().at(0)->addTexture(frequency_texture);


	std::vector<glm::vec4> heatmap = std::vector<glm::vec4>(2);
	heatmap[0] = glm::vec4(1, 1, 1, 1);
	heatmap[1] = glm::vec4(0, 0, 0, 1);
	Texture2D heatmap_tex = Texture2D(&heatmap, 2, 1);
	heatmap_tex.giveName("ellipsoid_tex");
	heatmap_tex.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
	heatmap_tex.setTexParameterWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
	AVW.getMeshes().at(0)->addTexture(heatmap_tex);
	AVW.getMeshes().at(0)->addTexture(wifi_tex);


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
	createFramebuffers(resolution, buffer, eyes, use_vr);
	quad.getMeshes().at(0)->addTexture(buffer[0].color_texture);
	quad.getMeshes().at(0)->addTexture(buffer[0].frag_pos_texture);
	quad.getMeshes().at(0)->addTexture(buffer[0].normal_texture);
	quad.getMeshes().at(0)->addTexture(eyes[0].screenTexture);

	int num_fb = 1;
	if (use_vr)
		num_fb = 2;
	//main render loop
	
	vr::Hmd_Eye curr_eye = vr::Hmd_Eye::Eye_Left;
	glm::mat4 ProjectionMat, ViewMat;

	std::map<std::string, glm::vec4> saved_colors;
	int old_num_colors = -1;

	float linear_term = 1, thickness = .1, distance_mask = 0;
	bool bin_orientations = false;

	while (!glfwWindowShouldClose(w.getWindow())) {
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
			skybox_shader.SetUniform4fv("projection", ProjectionMat);
			skybox_shader.SetUniform4fv("view", glm::mat4(glm::mat3(ViewMat)));

			glDepthMask(GL_FALSE);
			render(skybox, &skybox_shader);
			glDepthMask(GL_TRUE);

			//render ground plane
			ground_shader.SetUniform4fv("projection", ProjectionMat);
			ground_shader.SetUniform4fv("camera", ViewMat);
			ground_shader.SetUniform4fv("model", ground_transform);
			ground_shader.SetUniform1i("heatmap", 0);
			render(quad, &ground_shader);

			//render building model
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer[i].frame_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			model_shader.Use();
			model_shader.SetUniform4fv("model", avw_transform);
			model_shader.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse(avw_transform))));
			model_shader.SetUniform4fv("camera", ViewMat );
			Lights modelLights = setPointLights(totalLights, constant, linear, quadratic);
			model_shader.SetUniform4fv("projection", ProjectionMat);
			
			model_shader.SetUniform1f("transparency", transparency);
			model_shader.SetEllipsoid(ellipsoid);
			wifi_transform = glm::translate(glm::mat4(1), wifi_translate);
			wifi.ellipsoid_transform = wifi_transform * glm::scale(glm::mat4(1), wifi_scale);
			model_shader.SetUniform4fv("ellipsoid_transform", wifi.ellipsoid_transform);
			wifi.radius_stretch = wifi_scale * glm::vec3(1, 1, z_boost);

			model_shader.SetUniform3f("radius_stretch", wifi.radius_stretch);
			model_shader.SetUniform1f("radius", radius);
			model_shader.SetUniform1f("extent", extent);
			model_shader.SetUniform1f("frequency", 1 / frequency);
			model_shader.SetUniform1f("linear_term", linear_term);
			model_shader.SetUniform1f("thickness", thickness);
			model_shader.SetUniform1b("contour_on", contour_on);
			model_shader.SetUniform1b("display_names", display_names);
			model_shader.SetUniform1f("theta", theta);
			model_shader.SetUniform1f("u_stretch", u_stretch);
			model_shader.SetUniform1f("v_stretch", v_stretch);
			model_shader.SetUniform1f("distance_mask", distance_mask);
			model_shader.SetUniform1i("num_contours", num_dashes);
			model_shader.SetUniform3f("viewPos", camera.getPosition());
			model_shader.SetUniform1i("num_freqs", wifi.getActiveFreqs(freqs).size());
			model_shader.SetUniform1f("delta_theta", 180.f/wifi.getActiveFreqs(freqs).size());
			model_shader.SetUniform1b("bin_orientations", bin_orientations);
			//glDepthMask(GL_FALSE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			render(AVW, &model_shader);
			glDisable(GL_CULL_FACE);
			//glDepthMask(GL_TRUE);
			//glClear(GL_DEPTH_BUFFER_BIT);

			
			deferred_shader.Use();
			deferred_shader.SetUniform1i("num_point_lights", numLights);
			deferred_shader.SetUniform3f("viewPos", camera.getPosition());
			deferred_shader.SetLights(modelLights, camera.getPosition(), numLights);

			glBindFramebuffer(GL_FRAMEBUFFER, eyes[i].frame_buffer);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDepthMask(GL_FALSE);
			render(quad, &deferred_shader);
			glDepthMask(GL_TRUE);

			if (updated_routers) {
				wifi.updateRouterStructure(routers, wifinames, freqs, model_shader, nearest_router_on);
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
					AVW.getMeshes().at(0)->setTexture(text);
				}
			}
			//render wifi instances
			instance_shader.Use();
			wifi_transforms.clear();
			if (shade_instances)
				wifi_transforms = wifi.getTransforms(wifinames, routers, wifi_scale);

			std::vector<float> wifi_color_indices = wifi.getColorIndices();
			if (shade_instances)
				Sphere.getMeshes().at(0)->SetInstanceTransforms(wifi_transforms, wifi_color_indices);
			instance_shader.SetUniform4fv("projection", ProjectionMat);
			instance_shader.SetUniform4fv("view", ViewMat);
			//wifi_transform = glm::scale(glm::mat4(1), wifi_scale);

			instance_shader.SetUniform4fv("transform", wifi_transform);
			instance_shader.SetUniform4fv("model_transform", glyph_transform);
			instance_shader.SetUniform3fv("normalMat", glm::mat3(1));
			instance_shader.SetLights(lights);
			instance_shader.SetUniform3f("viewPos", camera.getPosition());
			glBindFramebuffer(GL_FRAMEBUFFER, eyes[i].frame_buffer);
			if (wifi_transforms.size() > 0 && shade_instances)
				render(Sphere, &instance_shader);
			if (use_vr) {
				ground_shader.Use();
				ground_shader.SetUniform4fv("camera", vr.getViewMatrix(curr_eye) * camera_offset);
				ground_shader.SetUniform4fv("model", vr.getControllerPose(vr.LeftDeviceId) * glm::scale(glm::mat4(1), .1f * glm::vec3(1)));
				render(LeftHand, &ground_shader);
				glm::mat4 right_hand_transform = vr.getControllerPose(vr.RightDeviceId) * glm::scale(glm::mat4(1), .1f * glm::vec3(1));
				ground_shader.SetUniform4fv("model", right_hand_transform);
				render(RightHand, &ground_shader);
				if (i == 0) {
					if (vr.ray_picker_enable) {
						//glm::vec4 forward = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1, 0, 0)) * glm::vec4(0, 0, 1, 0);
						rayPicker(glm::normalize(vr.getControllerPose(vr.RightDeviceId) * glm::vec4(0,0,1,0)), glm::vec3(vr.getControllerPose(vr.RightDeviceId) * glm::vec4(0, 0, 0, 1)), camera.getPosition(), vr.teleport_position);
						std::cout << "Teleport to: " << glm::to_string(vr.teleport_position) << std::endl;
						vr.teleport_position.z = camera.getPosition().z;
						camera.setPosition(vr.teleport_position);
						vr.ray_picker_enable = false;
					}
				}
				right_hand_transform = right_hand_transform * 
					glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(1, 0, 0)) *
					glm::scale(glm::mat4(1), glm::vec3(.1, 10, .1));
				ground_shader.SetUniform4fv("model", right_hand_transform);

				render(Cylinder, &ground_shader);
				vr.composite(curr_eye, buffer[i].color_tex);
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
			ImGui::SliderFloat("transparency", &transparency, 0, 1);
			ImGui::SliderInt("N", &num_routers, 1, 20);
			if (ImGui::Button("Nearest N Routers")) {
				wifi.setNearestNRouters(num_routers, camera.getPosition(), wifinames, routers, freqs);
				updated_routers = true;
				nearest_router_on = true;
			}if (ImGui::Button("Reset")) {
				std::fill(freqs.begin(), freqs.end(), true);
				std::fill(routers.begin(), routers.end(), true);
				std::fill(wifinames.begin(), wifinames.end(), true);
			}
			ImGui::SliderInt("Total Lights", &totalLights, 1, 200);
			ImGui::SliderInt("Lights Shown", &numLights, 1, totalLights);
			ImGui::SliderFloat("constant", &constant, 0, 1);
			ImGui::SliderFloat("linear", &linear, 0, 1);
			ImGui::SliderFloat("quadratic", &quadratic, 0, 1);
			ImGui::SliderFloat3("Bounding Box Scale", glm::value_ptr(bounding_cube_scale), 5, 35);
			ImGui::SliderFloat3("Bounding Box Offset", glm::value_ptr(bounding_cube_translate), -10, 10);
			ImGui::Checkbox("Shade Instances", &shade_instances);
			ImGui::Checkbox("Bin Orientations", &bin_orientations);
			ImGui::Checkbox("Display Names", &display_names);
			ImGui::Checkbox("Jittered Colors", &jittered);
			ImGui::SliderFloat("Extent", &extent, 0, 5);
			ImGui::SliderFloat("Dash Frequency", &radius, 1, 5);
			ImGui::SliderFloat("Contour Frequency", &frequency, .1, 10);
			ImGui::SliderFloat("Theta", &theta, 0, 360);
			ImGui::SliderFloat("u stretch", &u_stretch, 0, 10);
			ImGui::SliderFloat("v stretch", &v_stretch, 0, 1);
			ImGui::SliderFloat("Linear Term", &linear_term, 0, 1);
			ImGui::SliderFloat("thickness", &thickness, 0, .1);
			ImGui::SliderFloat("distance mask", &distance_mask, 0, 3);
			ImGui::SliderInt("Number of Dashes", &num_dashes, 1, 20);
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