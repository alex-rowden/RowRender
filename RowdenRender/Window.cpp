#include "Window.h"

void Window::SetVersion(int version_major, int version_minor) {
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version_minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Use core-profile (access a smaller subset of open-gl features
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