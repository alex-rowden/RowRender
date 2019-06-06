#pragma once
#include "RowRender.h"
class Window
{
public:

	GLFWwindow* window;

	void SetVersion(int version_major, int version_minor);
	void SetVersion(float version);
	bool makeWindow(int height, int width, std::string title);
	void ProcessFrame(bool useStandard = true);
	void ProcessFrame(void (*processInputFunc)(GLFWwindow*), bool useStandard = false);
	void SetViewportSize(int width, int height);
	void SetFramebuferSizeCallback();
};

