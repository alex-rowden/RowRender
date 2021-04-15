struct Gaussian {
	float x_coord, y_coord, amplitude, sigma;
};

#pragma once

#define PI 3.1415926535897932384626433
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <cmath>
#include <stdio.h>
#include <glad/glad.h> //must come before GLFW call (annoyingly)
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm\gtx\compatibility.hpp>
#include <glm/gtx/string_cast.hpp>
#include "Camera.h"
#include "Window.h"
#include "Shape.h"
#include "Texture2D.h"
#include "Mesh.h"
#include "Lights.h"
#include "ShaderProgram.h"
#include "Model.h"
#include <openvr.h>
#include "TextRenderer.h"


class Model;
class ShaderProgram;


struct Ellipsoid {
	glm::vec4 mu;
	glm::vec4 r;
	glm::mat4 axis;

};

const glm::vec3 GOLD = glm::vec3(255, 144, 0) / 255.0f;
const glm::vec3 PURPLE = glm::vec3(84, 106, 255) / 255.0f;
//any old render function
void render(Model mesh, ShaderProgram* sp);
void render(Model mesh, ShaderProgram* sp, int i);

//input is vec3 in form (H,S,V)
//H - [0, 360) representing Hue ("attribute of a visual sensation according to which an area appears to be similar to one of the perceived colors or combinations")
//S - [0,1] representing Saturation ("colorfulness of a stimulus relative to its own brightness")
//V - [0,1] representing Value ( the largest component of the color)
glm::vec3 hsv2rgb(glm::vec3);

void error_callback(int error, const char* description);

void setupDearIMGUI(GLFWwindow* window);

inline float rand_float() {return static_cast<float>(rand()) / static_cast <float> (RAND_MAX);}

/*
std::string getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}
*/
template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

#define RT_CHECK_ERROR_NO_CONTEXT( func ) \
  do { \
    RTresult code = func; \
    if (code != RT_SUCCESS) \
      std::cerr << "ERROR: Function " << #func << std::endl; \
  } while (0)

