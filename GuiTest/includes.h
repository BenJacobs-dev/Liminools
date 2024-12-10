#pragma once

#ifndef INCLUDES_H
#define INCLUDES_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif
#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#endif


#ifndef IMGUI_IMPLEMENTATION
#define IMGUI_IMPLEMENTATION
#include <imconfig.h>
//#include <imgui_tables.cpp>
#include <imgui_internal.h>
#include <imgui.h>
//#include <imgui_draw.cpp>
//#include <imgui_widgets.cpp>
//#include <imgui_demo.cpp>
#include <backends/imgui_impl_glfw.h>
#endif

#ifndef IMGUI_BUT_BETTER
#define IMGUI_BUT_BETTER
#include <imgui_impl_vulkan_but_better.h>
#endif

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "Uniforms.h"
#include "imguiWindow.h"
#include "VulkanRenderer.h"
#include "main.h"

#endif