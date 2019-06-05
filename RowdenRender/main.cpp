#include <iostream>
#include <glad/glad.h> //must come before GLFW call (annoyingly)
#include <GLFW/glfw3.h>

int counter = 0;
//any old render function
void render() {
	counter += 1;
	if (counter > 100)
		counter -= 100;
	glClearColor(counter/100.0f, counter / 100.0f, counter / 100.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

//Call in order to resize the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

int main() {
	std::cout << "Hello World" << std::endl;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Set to Version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Use core-profile (access a smaller subset of open-gl features

	GLFWwindow* window = glfwCreateWindow(800, 600, "HelloWindow", NULL, NULL); //Create the window

	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); //focus on the new window

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, 800, 600); //Set viewport to full window size
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //Set framebuffer callback

	while (!glfwWindowShouldClose(window)) //main render loop
	{
		processInput(window); //get keypresses etc.

		render();

		glfwSwapBuffers(window); //dual buffer swap
		glfwPollEvents();//get polled events
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 