#include "Window.h"

void Window::SetVersion(int version_major, int version_minor) {
	
	glfwWindowHint(GLFW_VERSION_MAJOR, (int)version_major);
	glfwWindowHint(GLFW_VERSION_MINOR, ((int)(version_minor)));
}

void Window::SetVersion(float version) {
	SetVersion((int)version, (int)(version * 10) % 10);
}

bool Window::makeWindow(int height, int width, std::string title) {
	window = glfwCreateWindow(height, width, title.c_str(), NULL, NULL);
	if (window == NULL) {
		return false;
	}
	return true;
}