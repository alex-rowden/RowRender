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
#include <openvr.h>
#define M_PIf 3.1415926535897932384626433
#define RT_CHECK_ERROR_NO_CONTEXT( func ) \
  do { \
    RTresult code = func; \
    if (code != RT_SUCCESS) \
      std::cerr << "ERROR: Function " << #func << std::endl; \
  } while (0)

struct Gaussian {
    float x_coord, y_coord, amplitude, sigma;
};