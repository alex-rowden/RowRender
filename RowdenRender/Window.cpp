#include "Window.h"

//Call in order to resize the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void standardInputProcessor(GLFWwindow* window) { //Go to processInputFunction, no extra steps needed
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void Window::SetVersion(int version_major, int version_minor) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version_minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Use core-profile (access a smaller subset of open-gl features
}

void Window::SetFramebuferSizeCallback() {
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //Set framebuffer callback
}

void Window::SetVersion(float version) {
	SetVersion((int)version, (int)(version * 10) % 10);
}

bool Window::makeWindow(int height, int width, std::string title) {
	window = glfwCreateWindow(height, width, title.c_str(), NULL, NULL);
	if (window == NULL) {
		
		return false;
	}
	glfwMakeContextCurrent(window); //focus on the new window
	return true;
}

void Window::SetViewportSize(int width, int height) {
	glViewport(0, 0, width, height); //Set viewport to full window size
}

//Process each input frame, by default uses standard input processor
void Window::ProcessFrame(bool useStandard) {
	standardInputProcessor(window); //get keypresses etc.
	glfwSwapBuffers(window); //dual buffer swap
	glfwPollEvents();//get polled events
}

//allow custom input processing function, standard process by default not used
void Window::ProcessFrame(void (*processInputFunc)(GLFWwindow *), bool useStandard) { 
	processInputFunc(window);
	ProcessFrame();
}

