#ifndef UTILS_INCLUDE_GUARD
#ifdef INCLUDE_UTILS
#include "H_EngineUtils.h"
#endif
#endif

#ifndef LOGGER_INCLUDE_GUARD
#ifdef INCLUDE_LOGGER
#include "../Core/H_Logger.hpp"
#endif
#endif

#ifndef VULKAN_INCLUDE_GUARD
#ifdef INCLUDE_VULKAN

#endif
#endif

#ifndef GLFW_INCLUDE_GUARD
#ifdef INCLUDE_GLFW

#ifdef INCLUDE_VULKAN

#define VKB_VALIDATION_LAYERS
#define GLFW_INCLUDE_VULKAN

#endif
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#endif
#endif

#ifndef GLM_INCLUDE_GUARD
#ifdef INCLUDE_GLM

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#endif
#endif

#ifndef YAML_INCLUDE_GUARD
#ifdef INCLUDE_YAML
#ifndef MADAM_DYNAMIC_LINK
#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#endif
#include <yaml-cpp/yaml.h>
#endif
#endif


// Include Guards Definition

#ifdef INCLUDE_LOGGER
#define LOGGER_INCLUDE_GUARD
#endif

#ifdef INCLUDE_VULKAN
#define VULKAN_INCLUDE_GUARD
#endif

#ifdef INCLUDE_GLFW
#define GLFW_INCLUDE_GUARD
#endif

#ifdef INCLUDE_GLM
#define GLM_INCLUDE_GUARD
#endif

#ifdef INCLUDE_YAML
#define YAML_INCLUDE_GUARD
#endif
