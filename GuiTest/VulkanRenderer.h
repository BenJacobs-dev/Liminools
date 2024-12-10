#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef VULKANIMPORTS
#define VULKANIMPORTS
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <imconfig.h>
#include <imgui_tables.cpp>
#include <imgui_internal.h>
#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>
#include <imgui_demo.cpp>
#include <backends/imgui_impl_glfw.cpp>
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
#include <stdio.h>
#include <filesystem>
#undef snprintf
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Uniforms.h"

#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

static std::vector<char> readFile(const std::string& filename);

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct UniformBufferObject {
    alignas(16) glm::vec4 color;
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class VulkanRenderer;

class imguiWindow {
public:
    void initGui(VulkanRenderer* vk, SaveData* saveDataIn, bool* inputEnabledIn, void (*enableInputIn)(), std::unordered_map<std::string, AnimationData>* animationsIn);

    void drawWindow();

    void saveWorldData();

    void saveWorldDataJson();

    void dataToJson(nlohmann::json*j);

    void updateOldSaves_v0_1_3(bool isStartUp);

    void loadWorldDataJson(bool isStartUp);

    void jsonToData(nlohmann::json*j);

    void updateOldSaves_v0_1_2();

    void deleteWorld();

private:

    VulkanRenderer* vkRenderer;

    const char* texturePath = "textures\\";
    const char* skyBoxPath = "skyboxes\\";
    const char* worldSavesPath = "worlds\\";
    const char* fileExtension = ".json";
    const char* version = "v0.2.0-";
    char fileName[128];
    //char skyBoxName[128] = "skyboxes\\Black.png";
    char textureName[MAX_IMAGES][128] = { "textures\\Black.png" };

	SaveData* saveData;

    bool* inputEnabled;

    void (*enableInput)();

    std::unordered_map<std::string, AnimationData>* animations;

    bool alreadyOpen[MAX_OBJECTS+1][MAX_OBJECTS + 1][MAX_OBJECTS + 1][MAX_OBJECTS + 1][2] = {false}; // [x][y][z][w][u], 0 = structure, 1 = object, 2 = combine modifier, 3 = domain modifier, 4 = normal object (0) or combine object (1)

    void createPlayPopup1F(std::string animationKey, float* val, const char* title);

    void createPlayPopup1I(std::string animationKey, int* val, const char* title);

    void createPlayPopup3F(std::string animationKey, float* val, const char* title);

    void createPlayPopup4F(std::string animationKey, float* val, const char* title);

    void createAnimationDataF(std::string animationKey, int typeValIn, int typeAnimationIn, float curFIn, float stepFIn, float minFIn, float maxFIn, float timePerStepIn, bool increasingIn);

    void createAnimationDataI(std::string animationKey, int typeValIn, int typeAnimationIn, int curIIn, int stepIIn, int minIIn, int maxIIn, float timePerStepIn, bool increasingIn);

    size_t mapAttributeForAnimation(std::string attribute);

    std::string mapAnimationForAttribute(size_t offset);

    size_t createKey(void* memberPtr);

    void drawIndex(int index, int depth, int* keys);

    void descriptionIndex(int index);

    void drawSubstructure(int type, int index, int depth, std::string prefix, int* keys);

    void drawObject(int index, int depth, std::string prefix, int* keys);
    
    void formatObject(int i);

    void descriptionObject(int i);

    void drawCombineModifier(int index, int depth, int* keys);

    void formatCombineModifier(int i);

    void descriptionCombineModifier(int i);

    void drawDomainModifier(int index, int depth, int* keys);

    void formatDomainModifier(int i);

    void descriptionDomainModifier(int i);

    void getSavedWorlds(std::vector<std::string>& savedWorlds);

    void getSkyBoxes(std::vector<std::string>& skyBoxes);

    void getTextures(std::vector<std::string>& textures);
    
    void createHelpPage();

    bool fileExists(char* fileNameIn, std::string extension);

    bool textureExists(std::string textureN);

    bool validFileName();

    void camDataCopy_v0_1_2_to_v0_1_3(CameraData_v0_1_2* camDataOld, CameraData_v0_1_3* camData);

    void worldDataCopy_v0_1_2_to_v0_1_3(WorldObjectsData_v0_1_2* worldDataOld, WorldObjectsData_v0_1_3* worldData);

    void camDataCopy_v0_1_3_to_v0_2_0(CameraData_v0_1_3* camDataOld, CameraData* camData);

    void worldDataCopy_v0_1_3_to_v0_2_0(WorldObjectsData_v0_1_3* worldDataOld, WorldObjectsData* worldData);
};


const std::string INIT_SKYBOX = "skyboxes\\Black.png";
const std::string INIT_TEXTURE = "textures\\Black.png";
const std::string TEST_TEXTURE = "textures\\JustAGuy.png";

class VulkanRenderer {
public:

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::string MODEL_PATH = "viking_room.obj";



    bool framebufferResized = false;

    void run(GLFWwindow* window);

    void initRenderer(GLFWwindow* window);

    void initUniforms(SaveData* saveDataIn);

    void initGui(bool* inputEnabled, void (*enableInput)(), std::unordered_map<std::string, AnimationData>* animationsIn);

    void drawFrame();

    void cleanupRenderer();

    bool uiWantsMouse();

    bool uiWantsKeyboard();

    int getWindowWidth();

    int getWindowHeight();

    void swapTexture(const std::string& texturePath, size_t index);

    void updateOldSaves();

    void wait();

    GLFWwindow* getWindow();

private:

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    GLFWwindow* window;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices queueIndices;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSurfaceKHR surface;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkDescriptorSetLayout descriptorSetLayout;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t currentFrame = 0;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};

            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };
    

    const std::vector<Vertex> vertices = {
    {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
    {{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
    };
    

    //std::vector<Vertex> vertices;
    //std::vector<uint32_t> indices;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    struct UniformDefinitionObject {
        std::vector<VkBuffer> buffers = {};
        std::vector<VkDeviceMemory> buffersMemory = {};
        std::vector<void*> buffersMapped = {};
        uint32_t binding = -1;
        VkShaderStageFlags stageFlags = 0;
        uint64_t size = 0;
        void* data = nullptr;
        bool updated = false;
    };

    //std::vector<UniformDefinitionObject> uniformData;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    uint32_t mipLevels;
    VkImage textureImage[MAX_IMAGES];
    VkDeviceMemory textureImageMemory[MAX_IMAGES];
    VkImageView imageView[MAX_IMAGES];
    VkSampler textureSampler;

    std::vector<VkBuffer> cameraUniformBuffers;
    std::vector<VkDeviceMemory> cameraUniformBuffersMemory;

    std::vector<VkBuffer> worldObjectsUniformBuffers;
    std::vector<VkDeviceMemory> worldObjectsUniformBuffersMemory;


    SaveData* saveData;

    imguiWindow imgui;

    // Create Window
    void initWindow(GLFWwindow* window);


    // Init Vulkan 
    void initVulkan();


    // Init Imgui
    void initImgui();


    // Create Single Time Commands
    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);


    // Create Vulkan Instance
    void createInstance();

    //void verifyExtensionAvailable(std::vector<const char*> requiredExtensions, std::vector<VkExtensionProperties> availableExtensions);

    std::vector<const char*> getRequiredExtensions();


    // Enable Validation/debugging
    bool checkValidationLayerSupport();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void setupDebugMessenger();


    // Create Surface
    void createSurface();


    // Pick Physics Device (GPU)
    void pickPhysicalDevice();

    bool isDeviceSuitable(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);


    // Create Logical Device
    void createLogicalDevice();


    
    void createSwapChain();

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void recreateSwapChain();

    void cleanupSwapChain();


    // Create Image Views
    void createImageViews();


    // Create The Graphics Pipeline - I might be able to remove most of this since i am only working/mainly in the fragment shader
    void createGraphicsPipeline();

    VkShaderModule createShaderModule(const std::vector<char>& code);


    // Create Render Pass
    void createRenderPass();


    // Create Frame Buffers
    void createFramebuffers();


    // Create Command Buffers
    void createCommandPool();

    void createCommandBuffers();


    // This is the main command list for each draw call, this is probably what will be updated to add new uniforms and stuff
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);


    // Create Synchronization Objects
    void createSyncObjects();


    // Create Images
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);


    // Create Textures
    void createTextureImage(const std::string& texturePath, size_t index);

    void createTextureImageView(size_t index);

    void createTextureSampler();

    void cleanupTexture(size_t index);


    // Create Depth Buffer
    void createDepthResources();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat format);


    // Create Model
    //void loadModel();


    // Create MipMaps
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);


    // Setup Multisampling
    VkSampleCountFlagBits getMaxUsableSampleCount();

    void createColorResources();


    // Create Vertex Buffer - Not Really Needed for a Path Tracer
    void createVertexBuffer();

    void createIndexBuffer();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


    // Create Uniforms
    void createDescriptorSetLayout();

    void createUniformBuffers();

    void updateUniformBuffer(uint32_t currentImage);

    void createDescriptorPool();

    void createDescriptorSets();

    void updateDescriptorSets();


    // The main render loop
    void mainLoop();

    // Draw Frame
    void drawFramePrivate();


    // Clean Up
    void cleanup();
};

#endif