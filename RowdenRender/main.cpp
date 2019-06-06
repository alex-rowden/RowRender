#pragma once
#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include <fstream>
int counter = 0;
//any old render function
void render() {
	counter += 1;
	if (counter > 100)
		counter -= 100;
	glClearColor(counter/100.0f, counter / 100.0f, counter / 100.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
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

	Mesh mesh = Mesh(triangle);

	char *vertexShaderString;
	std::ifstream vertex_shader_file;
	vertex_shader_file.open("vertex_shader.glsl", std::ifstream::in);
	if (vertex_shader_file.is_open()) {
		return -1;
	}
	std::streampos size = vertex_shader_file.tellg();
	vertexShaderString = new char[size];
	vertex_shader_file.read(vertexShaderString, size);

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (GLchar * const *)vertexShaderString, NULL);
	glCompileShader(vertexShader);

	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	while (!glfwWindowShouldClose(w.window)) //main render loop
	{
		render();
		w.ProcessFrame();
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 