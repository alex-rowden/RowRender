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
	
	w.SetFramebuferSizeCallback();

	Shape triangle = Shape();
	triangle.addVertex(glm::vec3(-.5, -.5, 0));
	triangle.addVertex(glm::vec3(.5, -.5, 0));
	triangle.addVertex(glm::vec3(0, .5, 0));

	Mesh mesh = Mesh(&triangle);

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

	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &success);

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