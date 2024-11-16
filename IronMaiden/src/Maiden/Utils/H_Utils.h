#ifndef UTILS_INCLUDE_GUARD
#ifdef INCLUDE_UTILS
#include "H_EngineUtils.h"
#endif
#endif

#ifdef INCLUDE_LOGGER
#include "../Core/H_Logger.hpp"
#endif

#ifdef INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#endif

#ifdef INCLUDE_GLFW
#define VKB_VALIDATION_LAYERS
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#endif

#ifdef INCLUDE_GLM

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

#ifdef INCLUDE_YAML
#ifndef INCLUDE_YAML_UTILS
#ifndef MADAM_DYNAMIC_LINK
#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#endif
#include <yaml-cpp/yaml.h>
#else
#include "H_YamlUtils.h"
#endif
#endif