struct Gaussian {
	float x_coord, y_coord, amplitude, sigma;
};
#pragma once
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

class Model;
class ShaderProgram;

//any old render function
void render(Model mesh, ShaderProgram* sp);

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

