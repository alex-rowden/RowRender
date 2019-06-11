#pragma once
#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
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

	quad.addTexCoord(glm::vec2(1, 1));
	quad.addTexCoord(glm::vec2(1, 0));
	quad.addTexCoord(glm::vec2(0, 0));
	quad.addTexCoord(glm::vec2(0, 1));

	Mesh mesh = Mesh(&quad);


	ShaderProgram sp = ShaderProgram({ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX});

	std::vector<glm::vec4> colors;
	colors.emplace_back(1, 0, 0, 1);
	colors.emplace_back(0, 1, 0, 1);
	colors.emplace_back(0, 0, 1, 1);
	colors.emplace_back(0, 0, 0, 1);
	mesh.SetColors(colors);

	mesh.SetData();
	
	Texture2D texture = Texture2D("\Content\\Textures\\brick_wall.jpg");
	glm::mat4 transformation = glm::mat4(1.0f);
	transformation = glm::rotate(transformation, glm::radians(90.0f), glm::vec3(0, 0, 1));
	transformation = glm::scale(transformation, glm::vec3(.5, .5, .5));

	while (!glfwWindowShouldClose(w.window)) //main render loop
	{
		sp.SetUniform4fv("transform", transformation);
		float timeValue = glfwGetTime();
		float greenVal = (sin(timeValue) / 2.0) + .5;
		//texture.Bind();
		render(mesh);
		w.ProcessFrame();
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 