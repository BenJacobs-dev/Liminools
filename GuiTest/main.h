#pragma once

#ifndef VULKANIMPORTS
#define VULKANIMPORTS
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
#include "VulkanRenderer.h"


// Create the window outside the application class so i can set up input
void initWindow();

void configureInput();

void enableInput();

void disableInput();

void initCamData();

void updateCamData();

// Set Up Keyboard Input
void moveForwards(float speed);

void moveLeft(float speed);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void createControls();

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

int main();

