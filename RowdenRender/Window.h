#pragma once
#include "RowRender.h"
#include "Camera.h"
#include <iostream>
#include <fstream>
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
	void setSpeed(float speed) { this->speed = speed; };
	GLFWwindow* getWindow();
	float lastX = 400, lastY = 300;
	Camera* camera;
	bool firstMouse = true;
	glm::vec3 scale = glm::vec3(1);
	glm::vec3 translate = glm::vec3(0);
	float speed = .5f;
	bool pressed = false;
	int j, width, height = 0;
	int i = 2;
	bool signal = false;
	double lastTime = 0;
	float horizontalAngle = 3.14f;
	float verticalAngle = 0.0f;
	std::fstream out;
	bool printing = false;
private:
	

};

