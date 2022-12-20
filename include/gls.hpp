#ifndef GLS_HPP
#define GLS_HPP

// STL
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// OpenGl
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad/glad.hpp"
using GLID = GLint;

// GLM
#define GLM_FORCE_INLINE
#define GLM_FORCE_XYZW_ONLY

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// fmt
#include <fmt/core.h>

#endif // GLS_HPP
