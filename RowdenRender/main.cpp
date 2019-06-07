#pragma once
#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include <fstream>
int counter = 0;
//any old render function
void render(Mesh mesh) {
	counter += 1;
	if (counter > 100)
		counter -= 100;
	glClearColor(counter/100.0f, counter / 100.0f, counter / 100.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	mesh.Render();
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

#include <windows.h>

std::string getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}

void MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}



int main() {
	


	std::cout << getexepath() << std::endl;
	glfwInit();
	glfwSetErrorCallback(error_callback);
	Window w;
	w.SetVersion(3, 3);
	

	bool window_made = w.makeWindow(800, 600, "helloClass");

	if (!window_made) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	w.SetFramebuferSizeCallback();

	Shape quad = Shape();
	quad.addVertex(glm::vec3(.5, .5, 0));
	quad.addVertex(glm::vec3(.5, -.5, 0));
	quad.addVertex(glm::vec3(-.5, -.5, 0));
	quad.addVertex(glm::vec3(-.5, .5, 0));

	quad.addIndex(glm::ivec3(0, 1, 3));
	quad.addIndex(glm::ivec3(1, 2, 3));

	Mesh mesh = Mesh(&quad);

	std::string vertexShaderString, fragmentShaderString;
	std::ifstream vertex_shader_file, fragment_shader_file;

	std::string path = __FILE__; //gets source code path, include file name
	path = path.substr(0, 1 + path.find_last_of('\\')); //removes file name

	vertex_shader_file.open(path + "vertex_shader.glsl");
	if (vertex_shader_file.is_open()) {
		char line[256];
		while (vertex_shader_file.good()) {
			vertex_shader_file.getline(line, 256);
			vertexShaderString.append(line);
			vertexShaderString.append("\n");
		}
	}
	vertex_shader_file.close();

	fragment_shader_file.open(path + "fragment_shader.glsl");
	if (fragment_shader_file.is_open()) {
		char line[256];
		while (fragment_shader_file.good()) {
			fragment_shader_file.getline(line, 256);
			fragmentShaderString.append(line);
			fragmentShaderString.append("\n");
		}
	}
	fragment_shader_file.close();

	unsigned int vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* shader_source = vertexShaderString.c_str();
	glShaderSource(vertexShader, 1, &shader_source, NULL);
	glCompileShader(vertexShader);

	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	shader_source = fragmentShaderString.c_str();
	glShaderSource(fragmentShader, 1, &shader_source, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	mesh.SetData();

	glUseProgram(shaderProgram);
	

	while (!glfwWindowShouldClose(w.window)) //main render loop
	{
		render(mesh);
		w.ProcessFrame();
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 