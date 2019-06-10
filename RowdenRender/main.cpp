#pragma once
#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include "ShaderProgram.h"
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


	ShaderProgram sp = ShaderProgram({ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX});

	

	mesh.SetData();
	

	while (!glfwWindowShouldClose(w.window)) //main render loop
	{
		float timeValue = glfwGetTime();
		float greenVal = (sin(timeValue) / 2.0) + .5;

		sp.SetUniform4f("ourColor", glm::vec4(0.0f, greenVal, 0.0f, 1.0f));
		render(mesh);
		w.ProcessFrame();
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 