#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <cmath>
#include <glad/glad.h> //must come before GLFW call (annoyingly)
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm\gtx\compatibility.hpp>

#define RT_CHECK_ERROR_NO_CONTEXT( func ) \
  do { \
    RTresult code = func; \
    if (code != RT_SUCCESS) \
      std::cerr << "ERROR: Function " << #func << std::endl; \
  } while (0)

