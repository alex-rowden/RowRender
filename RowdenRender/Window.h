#pragma once
#include "RowRender.h"
class Window
{
public:

	GLFWwindow* window;

	void SetVersion(int version_major, int version_minor);
	void SetVersion(float version);
	bool makeWindow(int height, int width, std::string title);

};

