#pragma once
#include "RowRender.h"

#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Model.h"

#include <fstream>
int counter = 10;
//any old render function
void render(Model mesh, ShaderProgram *sp) {
	if (counter > 100)
		counter -= 100;
	glClearColor(counter/100.0f, counter / 100.0f, counter / 100.0f, 1.0);
	
	mesh.Render(sp);
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
	Window w = Window("Better Window");
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEPTH_TEST);
	
	glDebugMessageCallback(MessageCallback, 0);

	w.SetFramebuferSizeCallback();

	//Shape cube = Shape(Shape::PREMADE::CUBE);
	

	//Mesh mesh = Mesh(&cube);


	ShaderProgram sp = ShaderProgram({ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX});
	ShaderProgram light_sp = ShaderProgram({ShaderProgram::Shaders::LIGHT_FRAG, ShaderProgram::Shaders::LIGHT_VERT});
	

	//mesh.SetData();
	//
	//Texture2D texture = Texture2D("Content\\Textures\\brick_wall.jpg");
	//texture.setTexParameterWrap(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	Model model = Model("Content\\Models\\cube\\cube.obj");
	Model light = Model("Content\\Models\\cube\\cube.obj");
	glm::mat4 transformation = glm::mat4(1.0f);
	
	Camera camera = Camera(glm::vec3(0, 1, 1), glm::vec3(0, 0, 0), 45.0f, 800/600.0f);
	w.SetCamera(&camera);
	glm::mat4 projection;
	//projection = glm::perspective(glm::radians(45.0f), 800/600.0f, 0.1f, 1000.0f);
	glm::mat4 light_transform = glm::translate(glm::mat4(1.0f), glm::vec3(3, 3, 3));
	while (!glfwWindowShouldClose(w.getWindow())) //main render loop
	{
		transformation = glm::mat4(1.0f);
		//transformation = glm::translate(transformation, glm::vec3(0, 0, -3));
		//transformation = glm::rotate(transformation, glm::radians(10 * (float)glfwGetTime()), glm::vec3(.5f, 1.0f,0));
		transformation = glm::scale(transformation, glm::vec3(.05, .05, .05));
		sp.SetUniform4fv("model", transformation);
		sp.SetUniform4fv("camera", camera.getView());
		sp.SetUniform3f("lightColor", glm::vec3(1, 0, 1));
		light_sp.SetUniform4fv("model", transformation * light_transform);
		light_sp.SetUniform4fv("camera", camera.getView());
		sp.SetUniform3f("lightPos", glm::vec3(3, 3, 3));
		//texture.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render(model, &sp);
		render(light, &light_sp);
		w.ProcessFrame(&camera);
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 