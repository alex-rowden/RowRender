#pragma once
#include "RowRender.h"
#include "Camera.h"
class Window
{
public:
	GLFWwindow* window;
	Window(const char* name);
	void SetCamera(Camera* camera);
	void SetVersion(int version_major, int version_minor);
	void SetVersion(float version);
	bool makeWindow(int height, int width, std::string title);
	void ProcessFrame(bool useStandard = true);
	void ProcessFrame(void (*processInputFunc)(GLFWwindow*), bool useStandard = false);
	void SetViewportSize(int width, int height);
	void SetFramebuferSizeCallback();
	void standardInputProcessor(GLFWwindow* window);
	GLFWwindow* getWindow();
	float lastX = 400, lastY = 300;
	Camera* camera;
private:
	float lastTime = 0;

};

