#pragma once
#include "RowRender.h"
#include "VR_Wrapper.h"
#include "WifiData.h"
#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include "Lights.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Model.h"
//#include <tinyxml2.h>
#include <random>
#include <fstream>
#include <iostream>
#include <chrono>
#include <time.h>
#include <direct.h>
#include <functional>
#include <glm/gtc/matrix_access.hpp>
#include "GaussianLoader.h"

#define MAX(a,b) a<b? b:a
#define MIN(a,b) a<b? a:b

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define DEBUG true
#define BENCHMARK false
#define MY_PI 3.1415926535897932384626433

using namespace vr;

int counter = 10;
float increment = 0.05;
float scale = 1.0;
bool update = true;

bool signed_distance = true;

//int size = 800;
bool animated = true;
const char* animation_file = "test_1.txt";
float speed = 6.0f;


glm::vec2 resolution = glm::vec2(2560, 1440);
glm::vec3 rand_dim = glm::vec3(50, 50, 50);

GLuint volume_textureId, depth_mask_id, normal_textureId, max_volumeId;

glm::vec3 iso_grid_size = glm::vec3(16, 16, 4);
std::vector<glm::vec2> isoRangeVolume = std::vector <glm::vec2>();

struct FramebufferDesc
{
	GLuint m_nDepthBufferId;
	GLuint m_nRenderTextureId;
	GLuint m_nRenderFramebufferId;
	GLuint m_nResolveTextureId;
	GLuint m_nResolveFramebufferId;
};
FramebufferDesc leftEyeDesc;
FramebufferDesc rightEyeDesc;



void updateIsoRangeVolume(std::vector<unsigned char> volume, glm::uvec3 dimensions, bool *enabled_vols, float increment) {
	glm::uvec3 index = glm::uvec3(0);
	isoRangeVolume.resize(iso_grid_size.x * iso_grid_size.y * iso_grid_size.z);
	glm::uvec2 grid_step = glm::uvec2(ceil(dimensions.x / iso_grid_size.x), ceil(dimensions.y / iso_grid_size.y));
	for (int i = 0; i < iso_grid_size.x; i++) {
		for (int j = 0; j < iso_grid_size.y; j++) {
			isoRangeVolume[i + iso_grid_size.x * j] = glm::vec2(255, 0);
			for (int k = 0; (k < grid_step.x) && (grid_step.x * i + k < dimensions.x); k++) {
				for (int l = 0; (l < grid_step.y) && (grid_step.y * j + l < dimensions.y); l++) {
					for (int m = 0; m < dimensions.z; m++) {
						if (!enabled_vols[m]) {
							continue;
						}
						if (isoRangeVolume[i + iso_grid_size.x * j].x > volume.at(i * grid_step.x + k + (j * grid_step.y + l) * dimensions.x + m * dimensions.x * dimensions.y)) {
							isoRangeVolume[i + iso_grid_size.x * j].x = volume.at(i * grid_step.x + k + (j * grid_step.y + l) * dimensions.x + m * dimensions.x * dimensions.y);
						}if (isoRangeVolume[i + iso_grid_size.x * j].y < volume.at(i * grid_step.x + k + (j * grid_step.y + l) * dimensions.x + m * dimensions.x * dimensions.y)) {
							isoRangeVolume[i + iso_grid_size.x * j].y = volume.at(i * grid_step.x + k + (j * grid_step.y + l) * dimensions.x + m * dimensions.x * dimensions.y);
						}
					}
				}
			}
		}
	}
	
	for (int i = 0; i < iso_grid_size.z; i++) {
		for (int j = 0; j < iso_grid_size.x * iso_grid_size.y; j++) {
			isoRangeVolume[j + i * iso_grid_size.x * iso_grid_size.y].x = (isoRangeVolume[j].x) / 255.0f - increment *(i + 1)/ iso_grid_size.z;
			isoRangeVolume[j + i * iso_grid_size.x * iso_grid_size.y].y = (isoRangeVolume[j].y) / 255.0f - increment * (i/iso_grid_size.z);
		}
	}
}

//any old render function
void render(Model mesh, ShaderProgram *sp) {
	
	for (Mesh* m : mesh.getMeshes()) {
		m->Render(sp);
	}
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

#include <windows.h>
/*
std::string getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}
*/
void MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131154) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

void LoadCampusModel(Model *completeCampus) {
	completeCampus->addModel(new Model("Content/Models/Buildings/campus.fbx"));
	completeCampus->setModel();
}

int get3DIndex(int i, int j, int k, int x_dim, int y_dim, int num_cells) {                                                                         
	if (i >= x_dim || j >= y_dim || k >= num_cells) {
		return -1;
	}
	return i + j * x_dim + k * x_dim * y_dim;
}

float getIntensity(std::vector<float> intensities, int max_index, int index) {
	if (index > max_index) {
		return 0;
	}
	else if (index < 0) {
		return -1;
	}
	return intensities.at(index);
}

glm::vec3 calcNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
{
	glm::vec3 edge0 = v1 - v0;
	glm::vec3 edge1 = v2 - v0;

	return glm::normalize(cross(edge0, edge1));
}
glm::vec3 vertexInterp(float isolevel, glm::vec3 p0, glm::vec3 p1, float f0, float f1)
{
	if (abs(f1 - f0) < std::numeric_limits<float>::epsilon()) {
		return p0;
	}
	float t = (isolevel - f0) / (f1 - f0);
	return glm::lerp(p0, p1, t);
}
/*
GLenum glFormatFromBufferFormat(bufferPixelFormat pixel_format, RTformat buffer_format)
{
	if (buffer_format == RT_FORMAT_UNSIGNED_BYTE4)
	{
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_BGRA;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGRA;
		default:
			throw std::exception("Unknown buffer pixel format");
		}
	}
	else if (buffer_format == RT_FORMAT_FLOAT4)
	{
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGRA;
		default:
			throw std::exception("Unknown buffer pixel format");
		}
	}
	else if (buffer_format == RT_FORMAT_FLOAT3)
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_RGB;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGB;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGR;
		default:
			throw std::exception("Unknown buffer pixel format");
		}
	else if (buffer_format == RT_FORMAT_FLOAT)
		return GL_LUMINANCE;
	else
		throw std::exception("Unknown buffer format");
}


void printMemUsage() {
	size_t free_byte;

	size_t total_byte;
	
	optix::cudaError_t cuda_status = optix::cudaMemGetInfo(&free_byte, &total_byte);

	if (optix::cudaSuccess != cuda_status) {

		printf("Error: cudaMemGetInfo fails, %s \n", optix::cudaGetErrorString(cuda_status));

		exit(1);

	}



	double free_db = (double)free_byte;

	double total_db = (double)total_byte;

	double used_db = total_db - free_db;

	printf("GPU memory usage: used = %f, free = %f MB, total = %f MB\n",

		used_db / 1024.0 / 1024.0, free_db / 1024.0 / 1024.0, total_db / 1024.0 / 1024.0);
}

void cudaPrint() {
	unsigned int numberOfDevices = 0;
	RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetDeviceCount(&numberOfDevices));
	std::cout << "Number of Devices = " << numberOfDevices << std::endl << std::endl;

	for (unsigned int i = 0; i < numberOfDevices; ++i)
	{
		char name[256];
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_NAME, sizeof(name), name));
		std::cout << "Device " << i << ": " << name << std::endl;

		int computeCapability[2] = { 0, 0 };
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY, sizeof(computeCapability), &computeCapability));
		std::cout << "  Compute Support: " << computeCapability[0] << "." << computeCapability[1] << std::endl;

		RTsize totalMemory = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TOTAL_MEMORY, sizeof(totalMemory), &totalMemory));
		std::cout << "  Total Memory: " << (unsigned long long) totalMemory << std::endl;

		int clockRate = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CLOCK_RATE, sizeof(clockRate), &clockRate));
		std::cout << "  Clock Rate: " << clockRate << " Hz" << std::endl;

		int maxThreadsPerBlock = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, sizeof(maxThreadsPerBlock), &maxThreadsPerBlock));
		std::cout << "  Max. Threads per Block: " << maxThreadsPerBlock << std::endl;

		int smCount = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, sizeof(smCount), &smCount));
		std::cout << "  Streaming Multiprocessor Count: " << smCount << std::endl;

		int executionTimeoutEnabled = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_EXECUTION_TIMEOUT_ENABLED, sizeof(executionTimeoutEnabled), &executionTimeoutEnabled));
		std::cout << "  Execution Timeout Enabled: " << executionTimeoutEnabled << std::endl;

		int maxHardwareTextureCount = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_HARDWARE_TEXTURE_COUNT, sizeof(maxHardwareTextureCount), &maxHardwareTextureCount));
		std::cout << "  Max. Hardware Texture Count: " << maxHardwareTextureCount << std::endl;

		int tccDriver = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TCC_DRIVER, sizeof(tccDriver), &tccDriver));
		std::cout << "  TCC Driver enabled: " << tccDriver << std::endl;

		int cudaDeviceOrdinal = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL, sizeof(cudaDeviceOrdinal), &cudaDeviceOrdinal));
		std::cout << "  CUDA Device Ordinal: " << cudaDeviceOrdinal << std::endl << std::endl;
	}
}
*/
void setupDearIMGUI(GLFWwindow *window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	return;
}

unsigned char getMax(long column, std::vector<unsigned char>& intensities, int skip, int numSlices) {
	unsigned char max = intensities.at(column);
	for (int i = 1; i < numSlices; i++) {
		if (max < intensities.at(column + i * skip)) {
			max = intensities.at(column + i * skip);
		}
	}
	return max;
}

void create_max_volume(std::vector<unsigned char> &intensities, WifiData &wifi, std::vector<unsigned char>&max_volume) {
	//glm::vec3 dimensions = glm::vec3(wifi.numLonCells, wifi.numLatCells, wifi.numSlices);
	for (int i = 0; i < wifi.numLonCells * wifi.numLatCells; i++) {
		max_volume.emplace_back(getMax(i, intensities, wifi.numLatCells * wifi.numLonCells, wifi.numSlices));
	}
	return;
}

bool overlap(glm::vec2 a, glm::vec2 b) {
	return a.y >= b.x && b.y >= a.x;
}



bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc& framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nRenderTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nRenderTextureId, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	/*
	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);
	*/
	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}


int main() {
	std::string wrk_dir = std::string("C:\\Users\\ARR87\\Documents\\GitHub\\RowRender\\RowdenRender");
	clock_t start = clock();
	std::vector<short> normal_x, normal_y;
	std::vector<float> use_intensities, max_volume;
	WifiData wifi;

	std::string filename = "gaussian_freqs";
	//std::string filename = "2Ghz_channels_float";
	//std::string filename = "sphere_freqs";
	wifi.loadBinary((filename + ".raw").c_str(), use_intensities, normal_x, normal_y);

	//create_max_volume(use_intensities, wifi, max_volume);
	max_volume = use_intensities;

	if (BENCHMARK) {
		std::cout << "Loading Data: " << (start - clock()) / CLOCKS_PER_SEC << " seconds" << std::endl;
		start = clock();
	}

	std::mt19937::result_type seed = time(0);
	auto generator = std::bind(std::uniform_real_distribution<float>(-1, 1),
		std::mt19937(seed));  // mt19937 is a standard mersenne_twister_engin
	glfwInit();
	glfwSetErrorCallback(error_callback);
	Window w = Window("Better Window", resolution.x, resolution.y);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	setupDearIMGUI(w.window);
	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glDebugMessageCallback(MessageCallback, 0);

	w.SetFramebuferSizeCallback();
	glfwSwapInterval(0);
	if (BENCHMARK) {
		std::cout << "GLFW INIT " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
		start = clock();
	}

	VR_Wrapper vr = VR_Wrapper();
	vr.initialize();
	vr.initCompositor();

	glm::uvec2 RenderSize = vr.getRenderTargetSize();

	ShaderProgram sp = ShaderProgram({ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX});
	ShaderProgram campus_map_sp = ShaderProgram({ShaderProgram::Shaders::NO_LIGHT_FRAG, ShaderProgram::Shaders::NO_LIGHT_VERT});
	//ShaderProgram screen_shader = ShaderProgram({ ShaderProgram::Shaders::SCREEN_FRAG, ShaderProgram::Shaders::SCREEN_VERT });
	ShaderProgram skybox_shader = ShaderProgram({ ShaderProgram::Shaders::SKY_FRAG, ShaderProgram::Shaders::SKY_VERT });
	ShaderProgram instance_shader = ShaderProgram({ ShaderProgram::Shaders::INSTANCE_FRAG, ShaderProgram::Shaders::INSTANCE_VERT });
	ShaderProgram volume_shader;
	if (!signed_distance)
		volume_shader = ShaderProgram({ ShaderProgram::Shaders::VOLUME_FRAG, ShaderProgram::Shaders::VOLUME_VERT });
	else
		volume_shader = ShaderProgram({ ShaderProgram::Shaders::SIGNED_DISTANCE_FRAG, ShaderProgram::Shaders::VOLUME_VERT });
	ShaderProgram front_back_shader = ShaderProgram({ ShaderProgram::Shaders::FRONT_BACK_FRAG, ShaderProgram::Shaders::FRONT_BACK_VERT });
	//mesh.SetData();
	//
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	std::vector<std::string> skybox_files;

	skybox_files.emplace_back("C:/Users/ARR87/Documents/GitHub/RowRender/RowdenRender/Content/Textures/Skyboxes/miramar_lf.png");
	skybox_files.emplace_back("C:/Users/ARR87/Documents/GitHub/RowRender/RowdenRender/Content/Textures/Skyboxes/miramar_rt.png");
	skybox_files.emplace_back("C:/Users/ARR87/Documents/GitHub/RowRender/RowdenRender/Content/Textures/Skyboxes/miramar_bk.png");
	skybox_files.emplace_back("C:/Users/ARR87/Documents/GitHub/RowRender/RowdenRender/Content/Textures/Skyboxes/miramar_ft.png");
	skybox_files.emplace_back("C:/Users/ARR87/Documents/GitHub/RowRender/RowdenRender/Content/Textures/Skyboxes/miramar_up.png");
	skybox_files.emplace_back("C:/Users/ARR87/Documents/GitHub/RowRender/RowdenRender/Content/Textures/Skyboxes/miramar_dn.png");


	Texture2D texture = Texture2D("Content\\Textures\\CampusMap.png");
	Texture2D noise = Texture2D("Content\\Textures\\random_noise.jpeg");
	Texture2D skybox_tex = Texture2D(skybox_files);
	skybox_tex.name = "skybox";
	//texture.setTexParameterWrap(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	Model model;
	LoadCampusModel(&model);
	Texture2D white = Texture2D(Texture2D::COLORS::WHITE);
	model.getMeshes()[0]->setTexture(white, 0);
	model.getMeshes()[0]->setTexture(white, 1);
	//model.getMeshes()[0]->SetUniformColor(glm::vec4(1, 1, 1, 1));
	Model skybox = Model("Content\\Models\\cube\\cube.obj");
	skybox.setModel();
	skybox.getMeshes().at(0)->setTexture(skybox_tex, 0);
	Model campusMap = Model("Content\\Models\\quad\\quad.obj");
	campusMap.setModel();
	campusMap.getMeshes().at(0)->setTexture(texture, 0);
	Model RayTraced = Model("Content\\Models\\quad\\quad_centered.obj");
	RayTraced.setModel();
	Model Blur = Model("Content\\Models\\quad\\quad_centered.obj");
	Blur.setModel();
	Model Tree = Model("Content\\Models\\tree_scaled.FBX");
	Tree.setModel();
	Model volume_cube = Model("Content\\Models\\cube\\cube.obj");
	volume_cube.setModel();
	Model untex_cube = Model("Content\\Models\\cube\\cube.obj");
	untex_cube.setModel();
	std::vector<Texture2D> volume_sets = std::vector<Texture2D>();
	volume_sets.resize(wifi.numSlices);
	std::vector<unsigned short> normal;
	normal.resize(normal_x.size() * 2);
	noise.giveName("noise");
	RayTraced.getMeshes().at(0)->addTexture(noise);
	for (size_t i = 0; i < normal_x.size(); i++) {
		normal.at(i * 2) = normal_x.at(i);
		normal.at(i * 2 + 1) = normal_y.at(i);
	}
	for (int i = 0; i < wifi.numSlices; i++) {
		Texture2D volume_data = Texture2D(&use_intensities[i * wifi.numLonCells * wifi.numLatCells], wifi.numLonCells, wifi.numLatCells);
		Texture2D normal_data = Texture2D(&normal[2 * i * wifi.numLonCells * wifi.numLatCells], wifi.numLonCells, wifi.numLatCells);
		volume_data.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
		volume_data.setTexParameterWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		volume_data.setBorderColor(glm::vec4(0, 0, 0, 0));
		normal_data.setTexMinMagFilter(GL_NEAREST, GL_NEAREST);
		volume_data.giveName("volume" + std::to_string(i));
		normal_data.giveName("normal" + std::to_string(i));
		RayTraced.getMeshes().at(0)->addTexture(volume_data);
		RayTraced.getMeshes().at(0)->addTexture(normal_data);
	}

	Texture2D depth_texture = Texture2D();
	depth_texture.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
	depth_texture.giveName("depth_tex");
	GLuint temp_tex;
	temp_tex = depth_texture.getID();
	RayTraced.getMeshes().at(0)->addTexture(depth_texture);

	glm::mat4 transformation = glm::scale(glm::mat4(1), scale * glm::vec3(-1, 1, -1));// glm::scale(glm::mat4(1), glm::vec3(-0.256f, 0.3f, -0.388998f));
	glm::vec3 volume_scale = glm::vec3(100.f, 100.f, 50.f);
	glm::vec3 box_min = glm::vec3(-50, -50, 0);
	volume_shader.SetUniform3f("volume_size", volume_scale);

	volume_shader.SetUniform1f("zNear", .1f);
	volume_shader.SetUniform1f("zFar", 1000.f);
	float ambientStrength = .5f;
	glm::vec3 lightDir = normalize(glm::vec3(0, 0.5, 0.5));
	float specularStrength = .2;
	float diffuseStrength = .7;
	float shininess = 64;
	volume_shader.SetUniform1f("shininess", shininess);
	volume_shader.SetUniform1f("ambientStrength", ambientStrength);
	volume_shader.SetUniform1f("specularStrength", specularStrength);
	volume_shader.SetUniform1f("diffuseStrength", diffuseStrength);



	//volume_shader.SetUniform3f("lightDir", lightDir);
	glm::vec2 lightDirP = glm::vec2(acos(lightDir.z), atan2(lightDir.y, lightDir.x));
	volume_shader.SetUniform2f("lightDirP", lightDirP);
	glm::vec2 sincosLightTheta = glm::vec2(sin(lightDirP.x), cos(lightDirP.x));
	volume_shader.SetUniform2f("sincosLightTheta", sincosLightTheta);

	struct TreeEntry {
		double lat, lon;
		int objID;
	};

	std::vector<TreeEntry*> trees;
	//load in tree positions
	std::ifstream tree_file = std::ifstream("Content/Data/plants.csv");
	if (!tree_file.is_open()) {
		std::cerr << "ERROR OPENING PLANTS.CSV\n" << std::endl;
	}
	std::string line;

	getline(tree_file, line);
	std::string header = line;

	while (!tree_file.eof())
	{
		TreeEntry* curr_tree = new TreeEntry();
		std::string IDstr;
		getline(tree_file, IDstr, ',');
		curr_tree->objID = atoi(IDstr.c_str());

		std::string skip;
		getline(tree_file, skip, ','); //S_ID
		getline(tree_file, skip, ','); //TAGID
		getline(tree_file, skip, ','); //CNAME1
		getline(tree_file, skip, ','); //CNAME2
		getline(tree_file, skip, ','); //CNAME3
		getline(tree_file, skip, ','); //GENUS
		getline(tree_file, skip, ','); //SPECIES
		getline(tree_file, skip, ','); //CULTIVAR

		std::string diameter_str;
		getline(tree_file, diameter_str, ',');
		//curr_tree->diameter = atof(diameter_str.c_str());

		std::string height_str;
		getline(tree_file, height_str, ',');
		//curr_tree->height = atof(height_str.c_str());

		getline(tree_file, skip, ','); //CRADAVG
		getline(tree_file, skip, ','); //TRUNKHEIGHT

		std::string lat_str, lon_str;
		getline(tree_file, lat_str, ',');
		getline(tree_file, lon_str);
		curr_tree->lat = atof(lat_str.c_str());
		curr_tree->lon = atof(lon_str.c_str());

		curr_tree->lon += 76.936594579149414130370132625103f;
		curr_tree->lat -= 38.990750300955419049842021195218f;
		trees.emplace_back(curr_tree);
		//std::cout << trees.size() << std::endl;
	}

	std::vector<glm::mat4> treeTransforms;

	//Create Tree transforms
	for (TreeEntry* treeEntry : trees)
	{
		if (treeTransforms.size() < 0)
			break;
		glm::mat4 transform = //glm::translate(glm::mat4(1), glm::vec3(20, 20, 1));
			glm::translate(glm::mat4(1), glm::vec3(treeEntry->lon * 3000000, treeEntry->lat * 3000000, 1.0f));
		treeTransforms.emplace_back(transform);
	}

	Tree.getMeshes().at(0)->SetInstanceTransforms(treeTransforms);
	if (BENCHMARK) {
		std::cout << "Model Loading " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
		start = clock();
	}
	/*
	WifiData wifi;

	std::vector<short> normal_x, normal_y, normal_z;
	std::vector<unsigned char> use_intensities;
	wifi.loadBinary("sphere_scaled512.raw", use_intensities, normal_x, normal_y, normal_z);
	*/

	
	time_t timer = time(NULL);
	struct tm local_time;
	localtime_s(&local_time, &timer);
	std::string foldername = "Output/Render_";
	char str[80];
	strftime(str, sizeof(str), "%b_%d_%y_%H_%M_%S", &local_time);
	foldername.append(str);

	if (!CreateDirectory(foldername.c_str(), NULL)) {
		std::cout << "directory creation failed" << std::endl;
	}

#if(false)
;

	std::fstream out = std::fstream("wifi_data.raw", std::ios::binary | std::ios::out);

	for (int i = 0; i < use_intensities.size(); i++) {
		UINT8 test = (UINT8)(use_intensities.at(i) * 255);
		out << test;
	}
	out.close();
#endif	



	glm::mat4 volume_transform = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 1, 0));
	//RTresult ret = rtBufferCreate(context->get(), RT_BUFFER_INPUT, &ray_buffer);

	Shape myShape;

	//setup camera
	Camera camera = Camera(glm::vec3(0, 50, 0), glm::vec3(0, 0, 0), 60.0f, w.width / (float)w.height);
	//Camera camera = Camera(glm::vec3(49.2877, 18.2977, 3.57346), glm::vec3(50, 49.999, 0), 60.0f, w.width / (float)w.height);
	//Camera camera = Camera(glm::vec3(36.9, 13.1627, 1.514), glm::vec3(40.3, 46.682, 3.57), 60.0f, w.width/(float)w.height);
	//Camera camera = Camera(glm::vec3(34,37.5, .5), glm::vec3(35, 37.5, 0.5), 90.0f, w.width / w.height);
	w.SetCamera(&camera);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//sutil::displayBufferPPM("out.ppm", context["output_buffer"]->getBuffer());

	//Mesh volume = Mesh(&myShape);

	//Model vol = Model();
	//vol.addMesh(&volume);
	//vol.setModel();
	//Texture2D wifi_intensities = Texture2D(&pixels, wifi.numLonCells, wifi.numLatCells);
	//campusMap.getMeshes().at(0)->setTexture(wifi_intensities, 0);
	Texture2D hdr_texture = Texture2D();
	hdr_texture.setDims(w.height, w.width, 4);
	//Camera camera = Camera(glm::vec3(0, 10, 10), glm::vec3(0, 0, 0), 45.0f, 800/600.0f);

	glm::mat4 projection;
	//w.scale = glm::vec3(1, .03, 1);
	w.scale = glm::vec3(0, 0, 0);
	w.translate = glm::vec3(0, 0, 0);
	//w.translate = glm::vec3(0, -.1f, 0);

	w.setSpeed(.5 * 10);

	Lights lights = Lights();
	float toNorm = 1 / 255.0;
	glm::vec3 color = glm::vec3(252, 202, 158) / 255.0f;
	glm::vec3 purple = glm::vec3(106, 93, 141) / 255.0f;
	glm::vec3 gold = glm::vec3(241, 198, 101) / 255.0f;
	//glm::vec3 
	//lights.addPointLight(50.0f * glm::vec3(1, 1, 2), .1, 0.01, 0, color, color, glm::vec3(1, 1, 1));
	//lights.addPointLight(50.0f * glm::vec3(1, .1, .5), 1, 0.0, 0, purple, purple, glm::vec3(1, 1, 1));
	lights.addDirLight(glm::vec3(0, 1, 0), gold);
	//lights.addPointLight(glm::vec3(0, 50, 0), 1, 0.0, 0, gold, gold, glm::vec3(1, 1, 1));
	sp.SetUniform1f("ambient_coeff", .5);
	sp.SetUniform1f("spec_coeff", .1);
	sp.SetUniform1f("diffuse_coeff", .4);
	sp.SetUniform1i("shininess", 32);

	glm::mat4 campusTransform;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> look_ats;
	std::vector<float> distances;
	char char_buffer[100];
	char* next_token;
	if (animated) {
		positions.emplace_back(camera.getPosition());
		look_ats.emplace_back(camera.getPosition() + camera.getDirection());
		std::fstream animation = std::fstream(animation_file, std::ios::in);
		bool odd = false;
		while (animation.getline(char_buffer, 100)) {
			char* parts = strtok_s(char_buffer, ", ", &next_token);
			glm::vec3 vec;
			int axis = 0;
			while (parts != NULL) {
				vec[axis] = atof(parts);
				parts = strtok_s(NULL, ", ", &next_token);
				axis++;
			}
			if (odd) {
				look_ats.emplace_back(vec);
			}
			else {
				positions.emplace_back(vec);
			}
			odd = !odd;
		}
		positions.emplace_back(camera.getPosition());
		look_ats.emplace_back(camera.getPosition() + camera.getDirection());
		for (int i = 0; i < positions.size() - 1; i++) {
			distances.emplace_back(glm::distance(positions.at(i), positions.at(i + 1)));
		}
		animated = false;
	}


	int fps = 0;
	uint64_t fps_counter = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	unsigned long int num_frames = 0;
	if (BENCHMARK) {
		std::cout << "Setup OptiX " << (double)(clock() - start) / CLOCKS_PER_SEC << " seconds" << std::endl;
		start = clock();
	}
	//Rendering Parameters
	float center = .959;//.56; //.2075
	float width = .001;//.015
	float base_opac = 0.379;
	float bubble_top = 1.0f;
	float bubble_bottom = 0;
	float bubble_max_opac = .032f;
	float bubble_min_opac = .0001f;
	float spec_term = .05;
	float sil_term = .95;
	bool color_aug = false;
	float tune = .45f;
	float fov = 60;
	int tex_num = 0;
	float max_iso_val = 0;
	bool iso_change = false;
	float increment = 1.136f;
	float old_increment = 0;
	float volumeStepSize = .1;//.11 / 3.0;
	float step_mod = 0;
	float shade_opac = .9;
	float box_z_min = 0.001;
	float fcp = 0.1;
	float effectiveStepSize = volumeStepSize;
	bool variableStepOn = false;
	float minStepSize = .0025;
	glm::vec3 color1 = glm::vec3(255, 255, 178) / 225.0f;
	glm::vec3 color2 = glm::vec3(254, 204, 92) / 225.0f;
	glm::vec3 color3 = glm::vec3(253, 141, 60) / 255.0f;
	glm::vec3 color4 = glm::vec3(240, 59, 32) / 255.0f;
	glm::vec3 color5 = glm::vec3(180, 0, 38) / 255.0f;
	glm::vec3 color6 = glm::vec3(253 / 255.0f, 117 / 255.0f, 0 / 255.0f);
	glm::vec3 intersection_color = glm::vec3(0);
	bool enable_color[6] = { true, false, false, false, false, false };
	//bool lighting_enabled = false;

	//glGenTextures(1, &temp_tex);
	glBindTexture(GL_TEXTURE_2D, temp_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, RenderSize.x, RenderSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	//RayTraced.getMeshes().at(0)->addTexture(depth_texture);
	Texture2D front_hit = Texture2D();
	Texture2D back_hit = Texture2D();
	GLuint fb = 0;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	GLuint front_hit_point_tex, back_hit_point_tex;
	front_hit_point_tex = front_hit.getID();
	back_hit_point_tex = back_hit.getID();
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, front_hit_point_tex);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, RenderSize.x, RenderSize.y, 0, GL_RGB, GL_FLOAT, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, front_hit_point_tex, 0);


	glBindTexture(GL_TEXTURE_2D, back_hit_point_tex);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, RenderSize.x, RenderSize.y, 0, GL_RGB, GL_FLOAT, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set "renderedTexture" as our colour attachement #0

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, back_hit_point_tex, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, DrawBuffers); // "2" is the size of DrawBuffers

	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, RenderSize.x, RenderSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return 0;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CreateFrameBuffer(RenderSize.x, RenderSize.y, leftEyeDesc);
	CreateFrameBuffer(RenderSize.x, RenderSize.y, rightEyeDesc);

	//front_hit.SetTextureID(front_hit_point_tex);
	front_hit.giveName("fhp");
	RayTraced.getMeshes().at(0)->addTexture(front_hit);


	//back_hit.SetTextureID(back_hit_point_tex);
	back_hit.giveName("bhp");
	RayTraced.getMeshes().at(0)->addTexture(back_hit);

	//volume_shader.SetUniform1i("numTex", wifi.numSlices);
	volume_shader.SetUniform1i("numTex", MIN(wifi.numSlices, 5));
	float volume_z = 3.5;
	int framesSinceMoved = 0;

	//const int num_gaussians = 2;
	//Gaussian gaussians[num_gaussians];
	//gaussians[0] = { 0, 0, 1, 1 };
	//gaussians[1] = { 10, 0, 1, 1.5f };

	std::vector<Gaussian> gaussians = std::vector < Gaussian>();
	GaussianLoader::loadGaussians("first_channel.gaus", gaussians);
	
	int num_gaussians = gaussians.size();

	//for (int i = 2; i < num_gaussians; i++) {
	//	gaussians[i] = {((rand()/(float)RAND_MAX) - .5f) * 50.0f, ((rand() / (float)RAND_MAX) - .5f) * 50.0f , 1, ((rand()/(float)RAND_MAX)/2.0f + .5f) * 2.0f};
	//}
	if (signed_distance) {
		volume_shader.SetUniform1i("num_gaussians", num_gaussians);
		volume_shader.SetGaussians(gaussians);
	}
	while (!glfwWindowShouldClose(w.getWindow())) //main render loop
	{

		glfwPollEvents();
		bool cameraMoved = camera.getMoved();
		camera.setMoved(false);
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Rendering Terms");
		
		ImGui::SliderFloat("IsoVal Center", &center, 0.0f, 1.0f);
		ImGui::SliderFloat("IsoVal width", &width, 0.0f, fmin(center/2.0, 1-center/2.0));

		ImGui::SliderFloat("Base Opacity", &base_opac, 0.0f, 1.f);
		ImGui::SliderFloat("Sillhoutte Term", &sil_term, 0.0f, 1.0f);
		ImGui::SliderFloat("bubble top", &bubble_top, 0.0f, 1.0f);
		ImGui::SliderFloat("bubble bottom", &bubble_bottom, 0.0f, bubble_top);
		ImGui::SliderFloat("bubble max opac", &bubble_max_opac, 0.0f, 1.0f);
		ImGui::SliderFloat("bubble min opac", &bubble_min_opac, 0.0f, bubble_max_opac);
		ImGui::SliderFloat("Debug", &tune, 0.0f, 1.0f);
		ImGui::SliderFloat("Front Clip Plane", &fcp, 0.1f, 10.0f);
		ImGui::SliderFloat("Step Size", &volumeStepSize, 0.001f, .2f);
		ImGui::SliderFloat("Step mod", &step_mod, 0.0f, 20.0f);
		
		ImGui::SliderFloat("Increment", &increment, 0.0f, 10.0f);
		ImGui::SliderFloat("Cube Z", &volume_z, 1.0f, 50.0f);
		ImGui::SliderFloat("Cube Z min", &box_z_min, -10.0f, 1.0f);
		ImGui::Checkbox("Shade intersection", &color_aug);
		ImGui::Checkbox("Variable Step Size", &variableStepOn);
		ImGui::SliderFloat("Specular Term", &spec_term, 0.0f, 10.0f);
		ImGui::SliderFloat("FOV", &fov, 0.0f, 90.0f);
		ImGui::Checkbox("Enable Channel 1", &enable_color[0]);
		if(enable_color[0])
			ImGui::ColorEdit3("Channel 1", &color1.x);
		ImGui::Checkbox("Enable Channel 4", &enable_color[1]);
		if (enable_color[1])
			ImGui::ColorEdit3("Channel 4", &color2.x);
		ImGui::Checkbox("Enable Channel 6", &enable_color[2]);
		if (enable_color[2])
			ImGui::ColorEdit3("Channel 6", &color3.x);
		ImGui::Checkbox("Enable Channel 9", &enable_color[3]);
		if (enable_color[3])
			ImGui::ColorEdit3("Channel 9", &color4.x);
		ImGui::Checkbox("Enable Channel 11", &enable_color[4]);
		if (enable_color[4])
			ImGui::ColorEdit3("Channel 11", &color5.x);
		ImGui::ColorEdit3("Intersection Color", &intersection_color.x);
		ImGui::SliderFloat("Intersection opacity", &shade_opac, 0.0f, 1.0f);
		//ImGui::Checkbox("Enable Lighting", &enable_color[5]);
		//ImGui::ColorEdit3("Volume Base Color", &color1.x);
		//ImGui::SliderInt("TextureNum", &tex_num, 0, wifi.numSlices);
		
		ImGui::End();
		
		if (cameraMoved) {
			effectiveStepSize = volumeStepSize;
			framesSinceMoved = 0;
		}
		else if (variableStepOn && framesSinceMoved > 4 && effectiveStepSize > minStepSize) {
			effectiveStepSize = effectiveStepSize / 2.0f;
			if (effectiveStepSize < minStepSize)
				effectiveStepSize = minStepSize;
			framesSinceMoved = 0;
		}

		volume_shader.SetUniform2f("IsoValRange", glm::vec2(center - width/2.0f, center + width/2.0f));
		volume_shader.SetUniform1f("StepSize", effectiveStepSize);
		volume_shader.SetUniform1f("increment", increment);
		volume_shader.SetUniform1f("base_opac", base_opac);
		volume_shader.SetUniform3f("color1", color1);
		volume_shader.SetUniform3f("color2", color2);
		volume_shader.SetUniform3f("color3", color3);
		volume_shader.SetUniform3f("color4", color4);
		volume_shader.SetUniform3f("color5", color5);
		volume_shader.SetUniform3f("shade_color", intersection_color);
		volume_shader.SetUniform1f("fcp", fcp);

		volume_shader.SetUniform1f("bubble_min", bubble_bottom);
		volume_shader.SetUniform1f("bubble_max", bubble_top);
		volume_shader.SetUniform1f("min_opac", bubble_min_opac);
		volume_shader.SetUniform1f("max_opac", bubble_max_opac);
		volume_shader.SetUniform1f("shade_opac", shade_opac);
		if (color_aug)
			volume_shader.SetUniform1f("enable_intersection", 1);
		else
			volume_shader.SetUniform1f("enable_intersection", 0);

		volume_shader.SetUniform1f("spec_term", spec_term);
		volume_shader.SetUniform1f("bubble_term", sil_term);
		volume_shader.SetUniform1f("step_mod", step_mod);
		volume_shader.SetUniform1f("tune", tune);
		
		box_min.z = box_z_min;
		volume_shader.SetUniform3f("box_min", box_min);
		volume_shader.SetUniform3f("box_max", box_min + glm::vec3(100, 100.f, volume_z));
		
		if (center + width / 2.0f != max_iso_val) {
			iso_change = true;
		}
		/*
		if (iso_change || increment != old_increment) {
			old_increment = increment;
			updateIsoRangeVolume(use_intensities, glm::uvec3(wifi.numLonCells, wifi.numLatCells, wifi.numSlices), enable_color, increment);
			max_iso_val = center + width / 2.0f;
			iso_change = false;
		}
		*/
		
		int enabledColors = 0;
		for (int i = 0; i < 6; i++) {
			if (enable_color[i]) {
				enabledColors |= 1 << i;
			}
		}
		volume_shader.SetUniform1i("enabledVolumes", enabledColors);
		vr.handle_vr_input();

		
		
		camera.fov = fov;
		clock_t per_frame = clock();
		glEnable(GL_DEPTH_TEST);
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - fps_counter < 1000) {
			fps++;
		}
		else {
			std::cout << fps << std::endl;
			fps = 0;
			fps_counter = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		if (animated) {
			float distance = speed * num_frames * 1 / 60.0f;
			int i = 0;
			while (distance > distances.at(i)) {
				distance -= distances.at(i);
				i++;
				if (i == distances.size()) {
					animated = false;
					distance = 0;
					i = 0;
					//uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

					return 0;
				}
			}

			float step = distance / distances.at(i);
			camera.setPosition(glm::lerp(positions.at(i), positions.at(i + 1), step));
			camera.setDirection(glm::lerp(look_ats.at(i) - positions.at(i), look_ats.at(i + 1) - positions.at(i + 1), step));
		}

		if (w.signal) {
			animated = true;
			start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			w.signal = false;
			update = true;

		}
		if (BENCHMARK) {
			std::cout << "Calculate FPS and Update Animation: " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}

		glm::mat4 transformation = glm::translate(glm::mat4(1), glm::vec3(75, 40.7, .6) - glm::vec3(50, 50, 0));
		transformation = glm::scale(transformation,   glm::vec3(0.00996, 0.012782, 0.0155));// glm::scale(glm::mat4(1), glm::vec3(-0.256f, 0.3f, -0.388998f));
		//transformation = glm::rotate(transformation, glm::radians(180.0f), glm::vec3(0, 1, 0));

		campusTransform = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0) - glm::vec3(50, 50, 0));
		campusTransform = glm::scale(campusTransform, scale * glm::vec3(100, 100, 100));


		//skybox_shader.SetUniform4fv("projection", camera.getProjection());
		//skybox_shader.SetUniform4fv("view", glm::mat4(glm::mat3(camera.getView())));

		glDepthMask(GL_FALSE);
		//render(skybox, &skybox_shader);
		glDepthMask(GL_TRUE);
		if (BENCHMARK) {
			std::cout << "Render Skybox " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		sp.SetUniform4fv("model", transformation);
		sp.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse( transformation))));
		sp.SetUniform4fv("camera", camera.getView());
		sp.SetUniform4fv("projection", camera.getProjection());
		sp.SetLights(lights);
		sp.SetUniform3f("view", camera.getPosition());
		//render(model, &sp);
		if (BENCHMARK) {
			std::cout << "Render Campus Model " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		
		//campus_map_sp.SetUniform4fv("camera", camera.getView());
		//campus_map_sp.SetUniform4fv("projection", camera.getProjection());
		
		if (BENCHMARK) {
			std::cout << "Render Campus Map " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		instance_shader.Use();
		
		instance_shader.SetUniform4fv("transform", glm::scale(glm::translate(glm::mat4(1), glm::vec3(72.099, 63.9, 0) + w.translate - glm::vec3(50, 50, 0)), glm::vec3(.00095, .00159, .0009) + w.scale));
		//render(Tree, &instance_shader);
		if (BENCHMARK) {
			std::cout << "Render Trees " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}

		front_back_shader.SetUniform4fv("model", glm::translate(glm::scale(glm::mat4(1), glm::vec3(50, 50, volume_z)), glm::vec3(1, 1, box_z_min + .5)));
		
		front_back_shader.SetUniform3f("box_min", box_min);
		front_back_shader.SetUniform3f("volume_scale", volume_scale);
		
		
		volume_shader.SetUniform3f("viewPos", glm::vec3(0,0,0));
		glm::vec3 HalfwayVec = glm::normalize(glm::vec3(0,0,0) + lightDir);
		glm::vec2 HalfwayVecP = glm::vec2(acos(HalfwayVec.z), atan2(HalfwayVec.y, HalfwayVec.x));
		volume_shader.SetUniform2f("HalfwayVecP", HalfwayVecP);
		glm::vec2 sincosHalfwayTheta = glm::vec2(sin(HalfwayVecP.x), cos(HalfwayVecP.x));
		volume_shader.SetUniform2f("sincosHalfwayTheta", sincosHalfwayTheta);
		vr.SaveControllerIDs();
		//vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0); //update Poses
		vr.updateHMDPoseMatrix();
		for (int i = 0; i < 2; i++) {
			Hmd_Eye curr_eye;
			FramebufferDesc curr_fb;
			if (i == 0) {
				curr_eye = Eye_Left;
				curr_fb = leftEyeDesc;
			}
			else {
				curr_eye = Eye_Right;
				curr_fb = rightEyeDesc;
			}

			glm::mat4 ProjectionMat = vr.getProjectionMatrix(curr_eye);
			glm::mat4 unRotView = vr.getViewMatrix(curr_eye);
			glm::mat4 ViewMat = glm::rotate(unRotView, glm::radians(-90.0f), glm::vec3(1, 0, 0));

			glBindFramebuffer(GL_FRAMEBUFFER, curr_fb.m_nRenderFramebufferId);
			glViewport(0, 0, RenderSize.x, RenderSize.y);
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

			skybox_shader.SetUniform4fv("projection", ProjectionMat);
			skybox_shader.SetUniform4fv("view", glm::mat4(glm::mat3(ViewMat)));
			sp.SetUniform4fv("projection", ProjectionMat);
			sp.SetUniform4fv("camera", ViewMat);
			campus_map_sp.SetUniform4fv("projection", ProjectionMat);
			campus_map_sp.SetUniform4fv("camera", ViewMat);
			instance_shader.SetUniform4fv("projection", ProjectionMat);
			instance_shader.SetUniform4fv("camera", ViewMat);
			//instance_shader.SetUniform4fv("transform", glm::scale(glm::translate(glm::mat4(1), glm::vec3(72.099, 63.9, 0) + w.translate - glm::vec3(50, 50, 0)), glm::vec3(.00095, .00159, .0009) + w.scale));

			glDepthMask(GL_FALSE);
			skybox_shader.Use();
			render(skybox, &skybox_shader);
			glDepthMask(GL_TRUE);
			campus_map_sp.Use();
			campus_map_sp.SetUniform4fv("camera", unRotView);
			campus_map_sp.SetUniform4fv("model", glm::rotate(vr.getControllerPose(vr.RightDeviceId) * glm::scale(glm::mat4(1), .1f * glm::vec3(1)), glm::radians(-0.0f), glm::vec3(1, 0, 0)));
			render(untex_cube, &campus_map_sp);
			campus_map_sp.SetUniform4fv("model", glm::rotate(vr.getControllerPose(vr.LeftDeviceId) * glm::scale(glm::mat4(1), .1f * glm::vec3(1)), glm::radians(-0.0f), glm::vec3(1, 0, 0)));
			render(untex_cube, &campus_map_sp);
			campus_map_sp.SetUniform4fv("camera", ViewMat);
			sp.Use();
			render(model, &sp);
			campus_map_sp.SetUniform4fv("model", campusTransform);
			campus_map_sp.Use();
			render(campusMap, &campus_map_sp);
			instance_shader.Use();
			render(Tree, &instance_shader);
			

			glBindTexture(GL_TEXTURE_2D, temp_tex);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, RenderSize.x, RenderSize.y);
			//glCopyImageSubData(temp_tex, GL_TEXTURE_2D, 0, 0, 0, 0,
			//					depth_mask_id, GL_TEXTURE_2D, 0, 0, 0, 0, 
			//					resolution.x, resolution.y, 1);
			glBindTexture(GL_TEXTURE_2D, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, fb);

			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			front_back_shader.SetUniform4fv("camera", ViewMat);
			front_back_shader.SetUniform4fv("projection", ProjectionMat);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			front_back_shader.SetUniform1i("front", 1);
			front_back_shader.Use();
			render(volume_cube, &front_back_shader);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);
			front_back_shader.SetUniform1i("front", 0);
			render(volume_cube, &front_back_shader);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_CULL_FACE);
			glBindFramebuffer(GL_FRAMEBUFFER, curr_fb.m_nRenderFramebufferId);
			volume_shader.Use();
			render(RayTraced, &volume_shader);
			
			vr.composite(curr_eye, curr_fb.m_nRenderTextureId);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		vr.handoff();
		//glFlush();
		glFinish();
		//GLfloat* depths = new GLfloat[w.width * w.height];

		//glReadPixels(0, 0, w.width, w.height, GL_DEPTH_COMPONENT, GL_FLOAT, depths);

		//glBindTexture(GL_TEXTURE_2D, depth_mask_id);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w.width, w.height, 0, GL_RED, GL_FLOAT, (void*)depths);
		//glBindTexture(GL_TEXTURE_2D, 0);

		std::string filename;
		/*
		if (false) {
			BYTE* depth_pix = new BYTE[w.width * w.height];

			for (int i = 0; i < w.width * w.height; i++) {
				depth_pix[i] = (unsigned char)(depths[i] * 255.0f);
			}
		
			filename = std::string(foldername + "/");
			filename.append(std::to_string(num_frames));
			filename.append(".bmp");

			stbi_flip_vertically_on_write(true);
			int save_result = stbi_write_bmp
			(
				filename.c_str(),
				resolution.x, resolution.y,
				1, depth_pix
			);
			if (save_result == 0) {
				std::cout << "shit" << std::endl;
			}
		}
		*/
		
		
		
		if (BENCHMARK) {
			std::cout << "Update Depth Buffer: " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}

		//context["lightPos"]->setFloat(make_float3(camera.getPosition()));
		
		if (max_iso_val != center + width / 2.0f) {
			//updateBoundingBox(center + width / 2.0f, max_volume);
			max_iso_val = center + width / 2.0f;
		}
		
		
		

		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		if (BENCHMARK) {
			std::cout << "Render Volume to Screen " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		filename = std::string(foldername + "/");
		filename.append(std::to_string(num_frames++));
		filename.append(".bmp");
		void* data;
		if (animated) {
			// Make the BYTE array, factor of 3 because it's RBG.
			BYTE* pixels = new BYTE[3 * w.width * w.height];

			glReadPixels(0, 0, w.width, w.height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			stbi_flip_vertically_on_write(true);
			int save_result = stbi_write_bmp
			(
				filename.c_str(),
				resolution.x, resolution.y,
				3, pixels
			);
			if (save_result == 0) {
				std::cout << "shit" << std::endl;
			}
			delete[] pixels;
		}
		ImGui::Render();
		
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (BENCHMARK) {
			std::cout << "Render GUI " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		
		w.ProcessFrame(&camera);
		if (BENCHMARK) {
			std::cout << "Full frame " << (double)((clock() - per_frame)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		//glClearColor(1,1,1, 1);
		glClearColor(135/255.0f, 206/255.0f, 235/255.0f, 1.0f);
		//glClearColor(.22, .69, .87, 1);
		//glClearColor(.196078, .6, .8, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFinish();
		framesSinceMoved++;
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate(); //Shut it down!
	vr.terminate();
	//VR_Shutdown();
	//vr_pointer = NULL;

	return 1;
} 