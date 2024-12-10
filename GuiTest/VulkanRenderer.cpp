#include "VulkanRenderer.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::cout << filename << std::endl;
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

//ImGui Functions
void imguiWindow::initGui(VulkanRenderer* vk, SaveData* saveDataIn, bool* inputEnabledIn, void (*enableInputIn)(), std::unordered_map<std::string, AnimationData>* animationsIn) {
    vkRenderer = vk;
    saveData = saveDataIn;
    inputEnabled = inputEnabledIn;
    enableInput = enableInputIn;
    animations = animationsIn;
}

std::vector<std::string> worldObjectTypes = {
    "Nothing",
    "Plane",
    "Sphere",
    "Box",
    "Box2",
    "Corner",
    "Wiggle Sphere",
    "Cylinder",
    "Capsule",
    "Torus",
    "Circle",
    "Disc",
    "Hexagon Circumcircle",
    "Hexagon Incircle",
    "Cone",
    "Wiggle Sphere Move",
    "Wiggle Plane Move"
};

std::vector<std::string> worldObjectCombineModifierTypes = {
    "Nothing",
    "Union",
    "Intersection",
    "Difference",
    "Union Chamfer",
    "Intersection Chamfer",
    "Difference Chamfer",
    "Union Round",
    "Intersection Round",
    "Difference Round",
    "Union Columns",
    "Intersection Columns",
    "Difference Columns",
    "Union Stairs",
    "Intersection Stairs",
    "Difference Stairs",
    "Union Soft",
    "Pipe",
    "Engrave",
    "Grove",
    "Tounge",
    "Bounding Box",
    "Invert Intersection",
};

std::vector<std::string> worldObjectDomainModifierTypes = {
    "Nothing",
    "Translate 3D",
    "Scale 3D",
    "Rotate 3D",
    "Reflect Plane",
    "Repeat 3D",
    "Repeat Mirror 3D",
    "Extend 3D",
    "Multiply 3D",
    "Ring 3D",
    "Octant 3D",
    "Twist 3D",
    "Bend 3D",
    "Wiggle World Move 3D",
    "Wiggle Cam Move 3D",
    "Rotate World Move 3D",
    "Rotate Cam Move 3D",
    "Look At Camera",
};

std::vector<std::string> worldObjectIndexTypes = {
	"Nothing",
	"Object",
    "Combine Modifier",
    "Domain Modifier",
};

void imguiWindow::createAnimationDataF(std::string animationKey, int typeValIn, int typeAnimationIn, float curFIn, float stepFIn, float minFIn, float maxFIn, float timePerStepIn, bool increasingIn) {
    AnimationData newAnimation;
    newAnimation.offset = mapAttributeForAnimation(animationKey);
	newAnimation.typeVal = typeValIn;
	newAnimation.typeAnimation = typeAnimationIn;
    newAnimation.curF = curFIn;
	newAnimation.stepF = stepFIn;
	newAnimation.minF = minFIn;
	newAnimation.maxF = maxFIn;
    newAnimation.timePerStep = timePerStepIn;
    newAnimation.timeSinceLastStep = 0.0f;
    newAnimation.stepI = 0;
    newAnimation.minI = 0;
    newAnimation.maxI = 0;
    newAnimation.curI = 0;
    newAnimation.increasing = increasingIn;
	animations->insert(std::make_pair(animationKey, newAnimation));
    std::cout << "There are now " << animations->size() << " animations" << std::endl;
}

void imguiWindow::createAnimationDataI(std::string animationKey, int typeValIn, int typeAnimationIn, int curIIn, int stepIIn, int minIIn, int maxIIn, float timePerStepIn, bool increasingIn) {
	AnimationData newAnimation;
    newAnimation.offset = mapAttributeForAnimation(animationKey);
	newAnimation.typeVal = typeValIn;
	newAnimation.typeAnimation = typeAnimationIn;
    newAnimation.curI = curIIn;
	newAnimation.stepI = stepIIn;
	newAnimation.minI = minIIn;
	newAnimation.maxI = maxIIn;
    newAnimation.timePerStep = timePerStepIn;
    newAnimation.timeSinceLastStep = 0.0f;
    newAnimation.stepF = 0.0f;
    newAnimation.minF = 0.0f;
    newAnimation.maxF = 0.0f;
    newAnimation.curF = 0.0f;
    newAnimation.increasing = increasingIn;
	animations->insert(std::make_pair(animationKey, newAnimation));
    std::cout << "There are now " << animations->size() << " animations" << std::endl;
}

/*
    saveData->camData.camera_pos[0] = "camData camera_pos x"
    saveData->camData.camera_pos[1] = "camData camera_pos y"
    saveData->camData.camera_pos[2] = "camData camera_pos z"
    saveData->camData.camera_rot[0] = "camData camera_rot x"
    saveData->camData.camera_rot[1] = "camData camera_rot y"
    saveData->camData.camera_rot[2] = "camData camera_rot z"
    saveData->camData.light_pos[0] = "camData light_pos x"
    saveData->camData.light_pos[1] = "camData light_pos y"
    saveData->camData.light_pos[2] = "camData light_pos z"
    saveData->camData.data1.x = "camData data1 x"
    saveData->camData.data1.y = "camData data1 y"
    saveData->camData.data1.z = "camData data1 z"
    saveData->camData.data1.w = "camData data1 w"
    saveData->camData.data2.x = "camData data2 x"
    saveData->camData.data2.y = "camData data2 y"
    saveData->camData.data2.z = "camData data2 z"
    saveData->camData.data2.w = "camData data2 w"
    saveData->camData.data3.x = "camData data3 x"
    saveData->camData.data3.y = "camData data3 y"
    saveData->camData.data3.z = "camData data3 z"
    saveData->camData.data3.w = "camData data3 w"
    saveData->camData.data4.x = "camData data4 x"
    saveData->camData.data4.y = "camData data4 y"
    saveData->camData.data4.z = "camData data4 z"
    saveData->camData.data4.w = "camData data4 w"
    saveData->camData.resolution.x = "camData resolution x"
    saveData->camData.resolution.y = "camData resolution y"
    saveData->camData.int1 = "camData int1"
    saveData->camData.int2 = "camData int2"
    saveData->camData.int3 = "camData int3"
    saveData->camData.int4 = "camData int4"
    saveData->camData.int5 = "camData int5"
    saveData->camData.int6 = "camData int6"
    saveData->camData.int7 = "camData int7"
    saveData->camData.int8 = "camData int8"
    saveData->camData.time = "camData time"
    saveData->camData.num_steps = "camData num_steps"
    saveData->camData.min_step = "camData min_step"
    saveData->camData.max_dist = "camData max_dist"
    saveData->camData.ray_depth = "camData ray_depth"


    for (int i = 0; i < max_objects; i++) {
        saveData->worldData.objects[i].center.x = "worldData objects i center x"
        saveData->worldData.objects[i].center.y = "worldData objects i center y"
        saveData->worldData.objects[i].center.z = "worldData objects i center z"
        saveData->worldData.objects[i].size.x = "worldData objects i size x"
        saveData->worldData.objects[i].size.y = "worldData objects i size y"
        saveData->worldData.objects[i].size.z = "worldData objects i size z"
        saveData->worldData.objects[i].color.x = "worldData objects i color x"
        saveData->worldData.objects[i].color.y = "worldData objects i color y"
        saveData->worldData.objects[i].color.z = "worldData objects i color z"
        saveData->worldData.objects[i].data1.x = "worldData objects i data1 x"
        saveData->worldData.objects[i].data1.y = "worldData objects i data1 y"
        saveData->worldData.objects[i].data1.z = "worldData objects i data1 z"
        saveData->worldData.objects[i].data1.w = "worldData objects i data1 w"
        saveData->worldData.objects[i].data2.x = "worldData objects i data2 x"
        saveData->worldData.objects[i].data2.y = "worldData objects i data2 y"
        saveData->worldData.objects[i].data2.z = "worldData objects i data2 z"
        saveData->worldData.objects[i].data2.w = "worldData objects i data2 w"
        saveData->worldData.objects[i].data3.x = "worldData objects i data3 x"
        saveData->worldData.objects[i].data3.y = "worldData objects i data3 y"
        saveData->worldData.objects[i].data3.z = "worldData objects i data3 z"
        saveData->worldData.objects[i].data3.w = "worldData objects i data3 w"
        saveData->worldData.objects[i].data4.x = "worldData objects i data4 x"
        saveData->worldData.objects[i].data4.y = "worldData objects i data4 y"
        saveData->worldData.objects[i].data4.z = "worldData objects i data4 z"
        saveData->worldData.objects[i].data4.w = "worldData objects i data4 w"
        saveData->worldData.objects[i].textureIndex = "worldData objects i textureIndex"
        saveData->worldData.objects[i].int2 = "worldData objects i int2"
        saveData->worldData.objects[i].int3 = "worldData objects i int3"
        saveData->worldData.objects[i].int4 = "worldData objects i int4"
        saveData->worldData.objects[i].int5 = "worldData objects i int5"
        saveData->worldData.objects[i].int6 = "worldData objects i int6"
        saveData->worldData.objects[i].int7 = "worldData objects i int7"
        saveData->worldData.objects[i].int8 = "worldData objects i int8"
        saveData->worldData.objects[i].type = "worldData objects i type"
        saveData->worldData.objects[i].is_negated = "worldData objects i is_negated"
        saveData->worldData.objects[i].shadow_blur = "worldData objects i shadow_blur"
        saveData->worldData.objects[i].shadow_intensity = "worldData objects i shadow_intensity"
        saveData->worldData.objects[i].reflectivity = "worldData objects i reflectivity"
        saveData->worldData.objects[i].transparency = "worldData objects i transparency"
        saveData->worldData.objects[i].diffuse_intensity = "worldData objects i diffuse_intensity"
        saveData->worldData.objects[i].refractive_index = "worldData objects i refractive_index"
    }

    for (int i = 0; i < max_objects; i++) {
        saveData->worldData.combineModifiers[i].data1.x = "worldData combineModifiers i data1 x"
        saveData->worldData.combineModifiers[i].data1.y = "worldData combineModifiers i data1 y"
        saveData->worldData.combineModifiers[i].data1.z = "worldData combineModifiers i data1 z"
        saveData->worldData.combineModifiers[i].data1.w = "worldData combineModifiers i data1 w"
        saveData->worldData.combineModifiers[i].data2.x = "worldData combineModifiers i data2 x"
        saveData->worldData.combineModifiers[i].data2.y = "worldData combineModifiers i data2 y"
        saveData->worldData.combineModifiers[i].data2.z = "worldData combineModifiers i data2 z"
        saveData->worldData.combineModifiers[i].data2.w = "worldData combineModifiers i data2 w"
        saveData->worldData.combineModifiers[i].data3.x = "worldData combineModifiers i data3 x"
        saveData->worldData.combineModifiers[i].data3.y = "worldData combineModifiers i data3 y"
        saveData->worldData.combineModifiers[i].data3.z = "worldData combineModifiers i data3 z"
        saveData->worldData.combineModifiers[i].data3.w = "worldData combineModifiers i data3 w"
        saveData->worldData.combineModifiers[i].data4.x = "worldData combineModifiers i data4 x"
        saveData->worldData.combineModifiers[i].data4.y = "worldData combineModifiers i data4 y"
        saveData->worldData.combineModifiers[i].data4.z = "worldData combineModifiers i data4 z"
        saveData->worldData.combineModifiers[i].data4.w = "worldData combineModifiers i data4 w"
        saveData->worldData.combineModifiers[i].int1 = "worldData combineModifiers i int1"
        saveData->worldData.combineModifiers[i].int2 = "worldData combineModifiers i int2"
        saveData->worldData.combineModifiers[i].int3 = "worldData combineModifiers i int3"
        saveData->worldData.combineModifiers[i].int4 = "worldData combineModifiers i int4"
        saveData->worldData.combineModifiers[i].int5 = "worldData combineModifiers i int5"
        saveData->worldData.combineModifiers[i].int6 = "worldData combineModifiers i int6"
        saveData->worldData.combineModifiers[i].int7 = "worldData combineModifiers i int7"
        saveData->worldData.combineModifiers[i].int8 = "worldData combineModifiers i int8"
        saveData->worldData.combineModifiers[i].index1 = "worldData combineModifiers i index1"
        saveData->worldData.combineModifiers[i].index2 = "worldData combineModifiers i index2"
        saveData->worldData.combineModifiers[i].type = "worldData combineModifiers i type"
        saveData->worldData.combineModifiers[i].index1Type = "worldData combineModifiers i index1Type"
    }

    for (int i = 0; i < max_objects; i++) {
        saveData->worldData.domainModifiers[i].data1.x = "worldData domainModifiers i data1 x"
        saveData->worldData.domainModifiers[i].data1.y = "worldData domainModifiers i data1 y"
        saveData->worldData.domainModifiers[i].data1.z = "worldData domainModifiers i data1 z"
        saveData->worldData.domainModifiers[i].data1.w = "worldData domainModifiers i data1 w"
        saveData->worldData.domainModifiers[i].data2.x = "worldData domainModifiers i data2 x"
        saveData->worldData.domainModifiers[i].data2.y = "worldData domainModifiers i data2 y"
        saveData->worldData.domainModifiers[i].data2.z = "worldData domainModifiers i data2 z"
        saveData->worldData.domainModifiers[i].data2.w = "worldData domainModifiers i data2 w"
        saveData->worldData.domainModifiers[i].data3.x = "worldData domainModifiers i data3 x"
        saveData->worldData.domainModifiers[i].data3.y = "worldData domainModifiers i data3 y"
        saveData->worldData.domainModifiers[i].data3.z = "worldData domainModifiers i data3 z"
        saveData->worldData.domainModifiers[i].data3.w = "worldData domainModifiers i data3 w"
        saveData->worldData.domainModifiers[i].data4.x = "worldData domainModifiers i data4 x"
        saveData->worldData.domainModifiers[i].data4.y = "worldData domainModifiers i data4 y"
        saveData->worldData.domainModifiers[i].data4.z = "worldData domainModifiers i data4 z"
        saveData->worldData.domainModifiers[i].data4.w = "worldData domainModifiers i data4 w"
        saveData->worldData.domainModifiers[i].int1 = "worldData domainModifiers i int1"
        saveData->worldData.domainModifiers[i].int2 = "worldData domainModifiers i int2"
        saveData->worldData.domainModifiers[i].int3 = "worldData domainModifiers i int3"
        saveData->worldData.domainModifiers[i].int4 = "worldData domainModifiers i int4"
        saveData->worldData.domainModifiers[i].int5 = "worldData domainModifiers i int5"
        saveData->worldData.domainModifiers[i].int6 = "worldData domainModifiers i int6"
        saveData->worldData.domainModifiers[i].int7 = "worldData domainModifiers i int7"
        saveData->worldData.domainModifiers[i].int8 = "worldData domainModifiers i int8"
        saveData->worldData.domainModifiers[i].index1 = "worldData domainModifiers i index1"
        saveData->worldData.domainModifiers[i].type = "worldData domainModifiers i type"
        saveData->worldData.domainModifiers[i].index1Type = "worldData domainModifiers i index1Type"
    }

    for (int i = 0; i < max_objects; i++) {
        saveData->worldData.indices[i].data1.x = "worldData indices i data1 x"
        saveData->worldData.indices[i].data1.y = "worldData indices i data1 y"
        saveData->worldData.indices[i].data1.z = "worldData indices i data1 z"
        saveData->worldData.indices[i].data1.w = "worldData indices i data1 w"
        saveData->worldData.indices[i].data2.x = "worldData indices i data2 x"
        saveData->worldData.indices[i].data2.y = "worldData indices i data2 y"
        saveData->worldData.indices[i].data2.z = "worldData indices i data2 z"
        saveData->worldData.indices[i].data2.w = "worldData indices i data2 w"
        saveData->worldData.indices[i].data3.x = "worldData indices i data3 x"
        saveData->worldData.indices[i].data3.y = "worldData indices i data3 y"
        saveData->worldData.indices[i].data3.z = "worldData indices i data3 z"
        saveData->worldData.indices[i].data3.w = "worldData indices i data3 w"
        saveData->worldData.indices[i].data4.x = "worldData indices i data4 x"
        saveData->worldData.indices[i].data4.y = "worldData indices i data4 y"
        saveData->worldData.indices[i].data4.z = "worldData indices i data4 z"
        saveData->worldData.indices[i].data4.w = "worldData indices i data4 w"
        saveData->worldData.indices[i].int1 = "worldData indices i int1"
        saveData->worldData.indices[i].int2 = "worldData indices i int2"
        saveData->worldData.indices[i].int3 = "worldData indices i int3"
        saveData->worldData.indices[i].int4 = "worldData indices i int4"
        saveData->worldData.indices[i].int5 = "worldData indices i int5"
        saveData->worldData.indices[i].int6 = "worldData indices i int6"
        saveData->worldData.indices[i].int7 = "worldData indices i int7"
        saveData->worldData.indices[i].int8 = "worldData indices i int8"
        saveData->worldData.indices[i].index = "worldData indices i index"
        saveData->worldData.indices[i].type = "worldData indices i type"
    }
*/

size_t imguiWindow::mapAttributeForAnimation(std::string attribute) {
    std::string curAtt = attribute;
    std::cout << "Mapping attribute: " << curAtt << std::endl;
    // Check if the attribute starts with "camData "
    if (curAtt.starts_with("camData ")) {
        curAtt = curAtt.substr(8);

        if (curAtt.starts_with("camera_pos ")) {
            curAtt = curAtt.substr(11);
            if (curAtt == "x") return createKey(&saveData->camData.camera_pos[0]);
            else if (curAtt == "y") return createKey(&saveData->camData.camera_pos[1]);
            else if (curAtt == "z") return createKey(&saveData->camData.camera_pos[2]);
        }
        else if (curAtt.starts_with("camera_rot ")) {
            curAtt = curAtt.substr(11);
            if (curAtt == "x") return createKey(&saveData->camData.camera_rot[0]);
            else if (curAtt == "y") return createKey(&saveData->camData.camera_rot[1]);
            else if (curAtt == "z") return createKey(&saveData->camData.camera_rot[2]);
        }
        else if (curAtt.starts_with("light_pos ")) {
            curAtt = curAtt.substr(10);
            if (curAtt == "x") return createKey(&saveData->camData.light_pos[0]);
            else if (curAtt == "y") return createKey(&saveData->camData.light_pos[1]);
            else if (curAtt == "z") return createKey(&saveData->camData.light_pos[2]);
        }
        else if (curAtt.starts_with("data1 ")) {
            curAtt = curAtt.substr(6);
            if (curAtt == "x") return createKey(&saveData->camData.data1.x);
            else if (curAtt == "y") return createKey(&saveData->camData.data1.y);
            else if (curAtt == "z") return createKey(&saveData->camData.data1.z);
            else if (curAtt == "w") return createKey(&saveData->camData.data1.w);
        }
        else if (curAtt.starts_with("data2 ")) {
            curAtt = curAtt.substr(6);
            if (curAtt == "x") return createKey(&saveData->camData.data2.x);
            else if (curAtt == "y") return createKey(&saveData->camData.data2.y);
            else if (curAtt == "z") return createKey(&saveData->camData.data2.z);
            else if (curAtt == "w") return createKey(&saveData->camData.data2.w);
        }
        else if (curAtt.starts_with("data3 ")) {
            curAtt = curAtt.substr(6);
            if (curAtt == "x") return createKey(&saveData->camData.data3.x);
            else if (curAtt == "y") return createKey(&saveData->camData.data3.y);
            else if (curAtt == "z") return createKey(&saveData->camData.data3.z);
            else if (curAtt == "w") return createKey(&saveData->camData.data3.w);
        }
        else if (curAtt.starts_with("data4 ")) {
            curAtt = curAtt.substr(6);
            if (curAtt == "x") return createKey(&saveData->camData.data4.x);
            else if (curAtt == "y") return createKey(&saveData->camData.data4.y);
            else if (curAtt == "z") return createKey(&saveData->camData.data4.z);
            else if (curAtt == "w") return createKey(&saveData->camData.data4.w);
        }
        else if (curAtt.starts_with("resolution ")) {
            curAtt = curAtt.substr(11);
            if (curAtt == "x") return createKey(&saveData->camData.resolution.x);
            else if (curAtt == "y") return createKey(&saveData->camData.resolution.y);
        }
        else if (curAtt == "int1") return createKey(&saveData->camData.int1);
        else if (curAtt == "int2") return createKey(&saveData->camData.int2);
        else if (curAtt == "int3") return createKey(&saveData->camData.int3);
        else if (curAtt == "int4") return createKey(&saveData->camData.int4);
        else if (curAtt == "int5") return createKey(&saveData->camData.int5);
        else if (curAtt == "int6") return createKey(&saveData->camData.int6);
        else if (curAtt == "int7") return createKey(&saveData->camData.int7);
        else if (curAtt == "int8") return createKey(&saveData->camData.int8);
        else if (curAtt == "time") return createKey(&saveData->camData.time);
        else if (curAtt == "num_steps") return createKey(&saveData->camData.num_steps);
        else if (curAtt == "min_step") return createKey(&saveData->camData.min_step);
        else if (curAtt == "max_dist") return createKey(&saveData->camData.max_dist);
        else if (curAtt == "ray_depth") return createKey(&saveData->camData.ray_depth);
    }
    else if (curAtt.starts_with("worldData ")) {
        curAtt = curAtt.substr(10);

        if (curAtt.starts_with("objects ")) {
            curAtt = curAtt.substr(8);
            std::cout << "curAtt: " << curAtt << std::endl;
            std::cout << "index: " << curAtt.substr(0, curAtt.find_first_of(' ')) << std::endl;
            int index = std::stoi(curAtt.substr(0, curAtt.find_first_of(' ')));
            std::cout << "index: " << index << std::endl;
            curAtt = curAtt.substr(curAtt.find_first_of(' ') + 1);

            if (curAtt.starts_with("center ")) {
                curAtt = curAtt.substr(7);
                if (curAtt == "x") return createKey(&saveData->worldData.objects[index].center.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.objects[index].center.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.objects[index].center.z);
            }
            else if (curAtt.starts_with("size ")) {
                curAtt = curAtt.substr(5);
                if (curAtt == "x") return createKey(&saveData->worldData.objects[index].size.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.objects[index].size.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.objects[index].size.z);
            }
            else if (curAtt.starts_with("color ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.objects[index].color.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.objects[index].color.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.objects[index].color.z);
            }
            else if (curAtt.starts_with("data1 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.objects[index].data1.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.objects[index].data1.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.objects[index].data1.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.objects[index].data1.w);
            }
            else if (curAtt.starts_with("data2 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.objects[index].data2.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.objects[index].data2.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.objects[index].data2.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.objects[index].data2.w);
            }
            else if (curAtt.starts_with("data3 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.objects[index].data3.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.objects[index].data3.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.objects[index].data3.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.objects[index].data3.w);
            }
            else if (curAtt.starts_with("data4 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.objects[index].data4.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.objects[index].data4.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.objects[index].data4.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.objects[index].data4.w);
            }
            else if (curAtt == "textureIndex") return createKey(&saveData->worldData.objects[index].textureIndex);
            else if (curAtt == "int2") return createKey(&saveData->worldData.objects[index].int2);
            else if (curAtt == "int3") return createKey(&saveData->worldData.objects[index].int3);
            else if (curAtt == "int4") return createKey(&saveData->worldData.objects[index].int4);
            else if (curAtt == "int5") return createKey(&saveData->worldData.objects[index].int5);
            else if (curAtt == "int6") return createKey(&saveData->worldData.objects[index].int6);
            else if (curAtt == "int7") return createKey(&saveData->worldData.objects[index].int7);
            else if (curAtt == "int8") return createKey(&saveData->worldData.objects[index].int8);
            else if (curAtt == "type") return createKey(&saveData->worldData.objects[index].type);
            else if (curAtt == "is_negated") return createKey(&saveData->worldData.objects[index].is_negated);
            else if (curAtt == "shadow_blur") return createKey(&saveData->worldData.objects[index].shadow_blur);
            else if (curAtt == "shadow_intensity") return createKey(&saveData->worldData.objects[index].shadow_intensity);
            else if (curAtt == "reflectivity") return createKey(&saveData->worldData.objects[index].reflectivity);
            else if (curAtt == "transparency") return createKey(&saveData->worldData.objects[index].transparency);
            else if (curAtt == "diffuse_intensity") return createKey(&saveData->worldData.objects[index].diffuse_intensity);
            else if (curAtt == "refractive_index") return createKey(&saveData->worldData.objects[index].refractive_index);
        }
        else if (curAtt.starts_with("combineModifiers ")) {
            curAtt = curAtt.substr(15);
            std::cout << "curAtt: " << curAtt << std::endl;
            std::cout << "index: " << curAtt.substr(0, curAtt.find_first_of(' ')) << std::endl;
            int index = std::stoi(curAtt.substr(0, curAtt.find_first_of(' ')));
            std::cout << "index: " << index << std::endl;
            curAtt = curAtt.substr(curAtt.find_first_of(' ') + 1);

            if (curAtt.starts_with("data1 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.combineModifiers[index].data1.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.combineModifiers[index].data1.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.combineModifiers[index].data1.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.combineModifiers[index].data1.w);
            }
            else if (curAtt.starts_with("data2 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.combineModifiers[index].data2.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.combineModifiers[index].data2.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.combineModifiers[index].data2.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.combineModifiers[index].data2.w);
            }
            else if (curAtt.starts_with("data3 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.combineModifiers[index].data3.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.combineModifiers[index].data3.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.combineModifiers[index].data3.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.combineModifiers[index].data3.w);
            }
            else if (curAtt.starts_with("data4 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.combineModifiers[index].data4.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.combineModifiers[index].data4.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.combineModifiers[index].data4.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.combineModifiers[index].data4.w);
            }
            else if (curAtt == "int1") return createKey(&saveData->worldData.combineModifiers[index].int1);
            else if (curAtt == "int2") return createKey(&saveData->worldData.combineModifiers[index].int2);
            else if (curAtt == "int3") return createKey(&saveData->worldData.combineModifiers[index].int3);
            else if (curAtt == "int4") return createKey(&saveData->worldData.combineModifiers[index].int4);
            else if (curAtt == "int5") return createKey(&saveData->worldData.combineModifiers[index].int5);
            else if (curAtt == "int6") return createKey(&saveData->worldData.combineModifiers[index].int6);
            else if (curAtt == "int7") return createKey(&saveData->worldData.combineModifiers[index].int7);
            else if (curAtt == "int8") return createKey(&saveData->worldData.combineModifiers[index].int8);
            else if (curAtt == "index1") return createKey(&saveData->worldData.combineModifiers[index].index1);
            else if (curAtt == "index2") return createKey(&saveData->worldData.combineModifiers[index].index2);
            else if (curAtt == "type") return createKey(&saveData->worldData.combineModifiers[index].type);
            else if (curAtt == "index1Type") return createKey(&saveData->worldData.combineModifiers[index].index1Type);
        }
        else if (curAtt.starts_with("domainModifiers ")) {
            curAtt = curAtt.substr(16);
            std::cout << "curAtt: " << curAtt << std::endl;
            std::cout << "index: " << curAtt.substr(0, curAtt.find_first_of(' ')) << std::endl;
            int index = std::stoi(curAtt.substr(0, curAtt.find_first_of(' ')));
            std::cout << "index: " << index << std::endl;
            curAtt = curAtt.substr(curAtt.find_first_of(' ') + 1);

            if (curAtt.starts_with("data1 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.domainModifiers[index].data1.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.domainModifiers[index].data1.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.domainModifiers[index].data1.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.domainModifiers[index].data1.w);
            }
            else if (curAtt.starts_with("data2 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.domainModifiers[index].data2.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.domainModifiers[index].data2.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.domainModifiers[index].data2.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.domainModifiers[index].data2.w);
            }
            else if (curAtt.starts_with("data3 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.domainModifiers[index].data3.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.domainModifiers[index].data3.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.domainModifiers[index].data3.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.domainModifiers[index].data3.w);
            }
            else if (curAtt.starts_with("data4 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.domainModifiers[index].data4.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.domainModifiers[index].data4.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.domainModifiers[index].data4.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.domainModifiers[index].data4.w);
            }
            else if (curAtt == "int1") return createKey(&saveData->worldData.domainModifiers[index].int1);
            else if (curAtt == "int2") return createKey(&saveData->worldData.domainModifiers[index].int2);
            else if (curAtt == "int3") return createKey(&saveData->worldData.domainModifiers[index].int3);
            else if (curAtt == "int4") return createKey(&saveData->worldData.domainModifiers[index].int4);
            else if (curAtt == "int5") return createKey(&saveData->worldData.domainModifiers[index].int5);
            else if (curAtt == "int6") return createKey(&saveData->worldData.domainModifiers[index].int6);
            else if (curAtt == "int7") return createKey(&saveData->worldData.domainModifiers[index].int7);
            else if (curAtt == "int8") return createKey(&saveData->worldData.domainModifiers[index].int8);
            else if (curAtt == "index1") return createKey(&saveData->worldData.domainModifiers[index].index1);
            else if (curAtt == "type") return createKey(&saveData->worldData.domainModifiers[index].type);
            else if (curAtt == "index1Type") return createKey(&saveData->worldData.domainModifiers[index].index1Type);
        }
        else if (curAtt.starts_with("indices ")) {
            curAtt = curAtt.substr(8);
            std::cout << "curAtt: " << curAtt << std::endl;
            std::cout << "index: " << curAtt.substr(0, curAtt.find_first_of(' ')) << std::endl;
            int index = std::stoi(curAtt.substr(0, curAtt.find_first_of(' ')));
            std::cout << "index: " << index << std::endl;
            curAtt = curAtt.substr(curAtt.find_first_of(' ') + 1);

            if (curAtt.starts_with("data1 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.indices[index].data1.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.indices[index].data1.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.indices[index].data1.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.indices[index].data1.w);
            }
            else if (curAtt.starts_with("data2 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.indices[index].data2.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.indices[index].data2.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.indices[index].data2.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.indices[index].data2.w);
            }
            else if (curAtt.starts_with("data3 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.indices[index].data3.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.indices[index].data3.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.indices[index].data3.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.indices[index].data3.w);
            }
            else if (curAtt.starts_with("data4 ")) {
                curAtt = curAtt.substr(6);
                if (curAtt == "x") return createKey(&saveData->worldData.indices[index].data4.x);
                else if (curAtt == "y") return createKey(&saveData->worldData.indices[index].data4.y);
                else if (curAtt == "z") return createKey(&saveData->worldData.indices[index].data4.z);
                else if (curAtt == "w") return createKey(&saveData->worldData.indices[index].data4.w);
            }
            else if (curAtt == "int1") return createKey(&saveData->worldData.indices[index].int1);
			else if (curAtt == "int2") return createKey(&saveData->worldData.indices[index].int2);
			else if (curAtt == "int3") return createKey(&saveData->worldData.indices[index].int3);
			else if (curAtt == "int4") return createKey(&saveData->worldData.indices[index].int4);
			else if (curAtt == "int5") return createKey(&saveData->worldData.indices[index].int5);
			else if (curAtt == "int6") return createKey(&saveData->worldData.indices[index].int6);
			else if (curAtt == "int7") return createKey(&saveData->worldData.indices[index].int7);
			else if (curAtt == "int8") return createKey(&saveData->worldData.indices[index].int8);
			else if (curAtt == "index") return createKey(&saveData->worldData.indices[index].index);
			else if (curAtt == "type") return createKey(&saveData->worldData.indices[index].type);
        }
    }
    std::cout << "Attribut '" << attribute << "' not found" << std::endl;
    return -1;
}

std::string imguiWindow::mapAnimationForAttribute(size_t offset) {
    if (offset == createKey(&saveData->camData.camera_pos[0])) return "camData camera_pos x";
	else if (offset == createKey(&saveData->camData.camera_pos[1])) return "camData camera_pos y";
	else if (offset == createKey(&saveData->camData.camera_pos[2])) return "camData camera_pos z";
	else if (offset == createKey(&saveData->camData.camera_rot[0])) return "camData camera_rot x";
	else if (offset == createKey(&saveData->camData.camera_rot[1])) return "camData camera_rot y";
	else if (offset == createKey(&saveData->camData.camera_rot[2])) return "camData camera_rot z";
	else if (offset == createKey(&saveData->camData.light_pos[0])) return "camData light_pos x";
	else if (offset == createKey(&saveData->camData.light_pos[1])) return "camData light_pos y";
	else if (offset == createKey(&saveData->camData.light_pos[2])) return "camData light_pos z";
	else if (offset == createKey(&saveData->camData.data1.x)) return "camData data1 x";
	else if (offset == createKey(&saveData->camData.data1.y)) return "camData data1 y";
	else if (offset == createKey(&saveData->camData.data1.z)) return "camData data1 z";
	else if (offset == createKey(&saveData->camData.data1.w)) return "camData data1 w";
	else if (offset == createKey(&saveData->camData.data2.x)) return "camData data2 x";
	else if (offset == createKey(&saveData->camData.data2.y)) return "camData data2 y";
	else if (offset == createKey(&saveData->camData.data2.z)) return "camData data2 z";
	else if (offset == createKey(&saveData->camData.data2.w)) return "camData data2 w";
	else if (offset == createKey(&saveData->camData.data3.x)) return "camData data3 x";
	else if (offset == createKey(&saveData->camData.data3.y)) return "camData data3 y";
    else if (offset == createKey(&saveData->camData.data3.z)) return "camData data3 z";
	else if (offset == createKey(&saveData->camData.data3.w)) return "camData data3 w";
	else if (offset == createKey(&saveData->camData.data4.x)) return "camData data4 x";
	else if (offset == createKey(&saveData->camData.data4.y)) return "camData data4 y";
	else if (offset == createKey(&saveData->camData.data4.z)) return "camData data4 z";
	else if (offset == createKey(&saveData->camData.data4.w)) return "camData data4 w";
	else if (offset == createKey(&saveData->camData.resolution.x)) return "camData resolution x";
	else if (offset == createKey(&saveData->camData.resolution.y)) return "camData resolution y";
	else if (offset == createKey(&saveData->camData.int1)) return "camData int1";
	else if (offset == createKey(&saveData->camData.int2)) return "camData int2";
	else if (offset == createKey(&saveData->camData.int3)) return "camData int3";
	else if (offset == createKey(&saveData->camData.int4)) return "camData int4";
	else if (offset == createKey(&saveData->camData.int5)) return "camData int5";
	else if (offset == createKey(&saveData->camData.int6)) return "camData int6";
	else if (offset == createKey(&saveData->camData.int7)) return "camData int7";
	else if (offset == createKey(&saveData->camData.int8)) return "camData int8";
	else if (offset == createKey(&saveData->camData.time)) return "camData time";
	else if (offset == createKey(&saveData->camData.num_steps)) return "camData num_steps";
	else if (offset == createKey(&saveData->camData.min_step)) return "camData min_step";
	else if (offset == createKey(&saveData->camData.max_dist)) return "camData max_dist";
	else if (offset == createKey(&saveData->camData.ray_depth)) return "camData ray_depth";
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (offset == createKey(&saveData->worldData.objects[i].center.x)) return "worldData objects " + std::to_string(i) + " center x";
        else if (offset == createKey(&saveData->worldData.objects[i].center.y)) return "worldData objects " + std::to_string(i) + " center y";
        else if (offset == createKey(&saveData->worldData.objects[i].center.z)) return "worldData objects " + std::to_string(i) + " center z";
        else if (offset == createKey(&saveData->worldData.objects[i].size.x)) return "worldData objects " + std::to_string(i) + " size x";
        else if (offset == createKey(&saveData->worldData.objects[i].size.y)) return "worldData objects " + std::to_string(i) + " size y";
        else if (offset == createKey(&saveData->worldData.objects[i].size.z)) return "worldData objects " + std::to_string(i) + " size z";
        else if (offset == createKey(&saveData->worldData.objects[i].color.x)) return "worldData objects " + std::to_string(i) + " color x";
        else if (offset == createKey(&saveData->worldData.objects[i].color.y)) return "worldData objects " + std::to_string(i) + " color y";
        else if (offset == createKey(&saveData->worldData.objects[i].color.z)) return "worldData objects " + std::to_string(i) + " color z";
        else if (offset == createKey(&saveData->worldData.objects[i].data1.x)) return "worldData objects " + std::to_string(i) + " data1 x";
        else if (offset == createKey(&saveData->worldData.objects[i].data1.y)) return "worldData objects " + std::to_string(i) + " data1 y";
        else if (offset == createKey(&saveData->worldData.objects[i].data1.z)) return "worldData objects " + std::to_string(i) + " data1 z";
        else if (offset == createKey(&saveData->worldData.objects[i].data1.w)) return "worldData objects " + std::to_string(i) + " data1 w";
        else if (offset == createKey(&saveData->worldData.objects[i].data2.x)) return "worldData objects " + std::to_string(i) + " data2 x";
		else if (offset == createKey(&saveData->worldData.objects[i].data2.y)) return "worldData objects " + std::to_string(i) + " data2 y";
		else if (offset == createKey(&saveData->worldData.objects[i].data2.z)) return "worldData objects " + std::to_string(i) + " data2 z";
		else if (offset == createKey(&saveData->worldData.objects[i].data2.w)) return "worldData objects " + std::to_string(i) + " data2 w";
		else if (offset == createKey(&saveData->worldData.objects[i].data3.x)) return "worldData objects " + std::to_string(i) + " data3 x";
		else if (offset == createKey(&saveData->worldData.objects[i].data3.y)) return "worldData objects " + std::to_string(i) + " data3 y";
		else if (offset == createKey(&saveData->worldData.objects[i].data3.z)) return "worldData objects " + std::to_string(i) + " data3 z";
		else if (offset == createKey(&saveData->worldData.objects[i].data3.w)) return "worldData objects " + std::to_string(i) + " data3 w";
		else if (offset == createKey(&saveData->worldData.objects[i].data4.x)) return "worldData objects " + std::to_string(i) + " data4 x";
		else if (offset == createKey(&saveData->worldData.objects[i].data4.y)) return "worldData objects " + std::to_string(i) + " data4 y";
		else if (offset == createKey(&saveData->worldData.objects[i].data4.z)) return "worldData objects " + std::to_string(i) + " data4 z";
		else if (offset == createKey(&saveData->worldData.objects[i].data4.w)) return "worldData objects " + std::to_string(i) + " data4 w";
        else if (offset == createKey(&saveData->worldData.objects[i].textureIndex)) return "worldData objects " + std::to_string(i) + " textureIndex";
		else if (offset == createKey(&saveData->worldData.objects[i].int2)) return "worldData objects " + std::to_string(i) + " int2";
		else if (offset == createKey(&saveData->worldData.objects[i].int3)) return "worldData objects " + std::to_string(i) + " int3";
		else if (offset == createKey(&saveData->worldData.objects[i].int4)) return "worldData objects " + std::to_string(i) + " int4";
		else if (offset == createKey(&saveData->worldData.objects[i].int5)) return "worldData objects " + std::to_string(i) + " int5";
		else if (offset == createKey(&saveData->worldData.objects[i].int6)) return "worldData objects " + std::to_string(i) + " int6";
		else if (offset == createKey(&saveData->worldData.objects[i].int7)) return "worldData objects " + std::to_string(i) + " int7";
		else if (offset == createKey(&saveData->worldData.objects[i].int8)) return "worldData objects " + std::to_string(i) + " int8";
		else if (offset == createKey(&saveData->worldData.objects[i].type)) return "worldData objects " + std::to_string(i) + " type";
		else if (offset == createKey(&saveData->worldData.objects[i].is_negated)) return "worldData objects " + std::to_string(i) + " is_negated";
		else if (offset == createKey(&saveData->worldData.objects[i].shadow_blur)) return "worldData objects " + std::to_string(i) + " shadow_blur";
		else if (offset == createKey(&saveData->worldData.objects[i].shadow_intensity)) return "worldData objects " + std::to_string(i) + " shadow_intensity";
		else if (offset == createKey(&saveData->worldData.objects[i].reflectivity)) return "worldData objects " + std::to_string(i) + " reflectivity";
		else if (offset == createKey(&saveData->worldData.objects[i].transparency)) return "worldData objects " + std::to_string(i) + " transparency";
        else if (offset == createKey(&saveData->worldData.objects[i].diffuse_intensity)) return "worldData objects " + std::to_string(i) + " diffuse_intensity";
        else if (offset == createKey(&saveData->worldData.objects[i].refractive_index)) return "worldData objects " + std::to_string(i) + " refractive_index";
        else if (offset == createKey(&saveData->worldData.combineModifiers[i].data1.x)) return "worldData combineModifiers " + std::to_string(i) + " data1 x";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data1.y)) return "worldData combineModifiers " + std::to_string(i) + " data1 y";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data1.z)) return "worldData combineModifiers " + std::to_string(i) + " data1 z";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data1.w)) return "worldData combineModifiers " + std::to_string(i) + " data1 w";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data2.x)) return "worldData combineModifiers " + std::to_string(i) + " data2 x";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data2.y)) return "worldData combineModifiers " + std::to_string(i) + " data2 y";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data2.z)) return "worldData combineModifiers " + std::to_string(i) + " data2 z";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data2.w)) return "worldData combineModifiers " + std::to_string(i) + " data2 w";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data3.x)) return "worldData combineModifiers " + std::to_string(i) + " data3 x";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data3.y)) return "worldData combineModifiers " + std::to_string(i) + " data3 y";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data3.z)) return "worldData combineModifiers " + std::to_string(i) + " data3 z";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data3.w)) return "worldData combineModifiers " + std::to_string(i) + " data3 w";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data4.x)) return "worldData combineModifiers " + std::to_string(i) + " data4 x";
        else if (offset == createKey(&saveData->worldData.combineModifiers[i].data4.y)) return "worldData combineModifiers " + std::to_string(i) + " data4 y";
        else if (offset == createKey(&saveData->worldData.combineModifiers[i].data4.z)) return "worldData combineModifiers " + std::to_string(i) + " data4 z";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].data4.w)) return "worldData combineModifiers " + std::to_string(i) + " data4 w";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].int1)) return "worldData combineModifiers " + std::to_string(i) + " int1";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].int2)) return "worldData combineModifiers " + std::to_string(i) + " int2";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].int3)) return "worldData combineModifiers " + std::to_string(i) + " int3";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].int4)) return "worldData combineModifiers " + std::to_string(i) + " int4";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].int5)) return "worldData combineModifiers " + std::to_string(i) + " int5";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].int6)) return "worldData combineModifiers " + std::to_string(i) + " int6";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].int7)) return "worldData combineModifiers " + std::to_string(i) + " int7";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].int8)) return "worldData combineModifiers " + std::to_string(i) + " int8";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].index1)) return "worldData combineModifiers " + std::to_string(i) + " index1";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].index2)) return "worldData combineModifiers " + std::to_string(i) + " index2";
		else if (offset == createKey(&saveData->worldData.combineModifiers[i].type)) return "worldData combineModifiers " + std::to_string(i) + " type";
        else if (offset == createKey(&saveData->worldData.combineModifiers[i].index1Type)) return "worldData combineModifiers " + std::to_string(i) + " index1Type";
        else if (offset == createKey(&saveData->worldData.domainModifiers[i].data1.x)) return "worldData domainModifiers " + std::to_string(i) + " data1 x";
        else if (offset == createKey(&saveData->worldData.domainModifiers[i].data1.y)) return "worldData domainModifiers " + std::to_string(i) + " data1 y";
        else if (offset == createKey(&saveData->worldData.domainModifiers[i].data1.z)) return "worldData domainModifiers " + std::to_string(i) + " data1 z";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data1.w)) return "worldData domainModifiers " + std::to_string(i) + " data1 w";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data2.x)) return "worldData domainModifiers " + std::to_string(i) + " data2 x";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data2.y)) return "worldData domainModifiers " + std::to_string(i) + " data2 y";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data2.z)) return "worldData domainModifiers " + std::to_string(i) + " data2 z";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data2.w)) return "worldData domainModifiers " + std::to_string(i) + " data2 w";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data3.x)) return "worldData domainModifiers " + std::to_string(i) + " data3 x";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data3.y)) return "worldData domainModifiers " + std::to_string(i) + " data3 y";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data3.z)) return "worldData domainModifiers " + std::to_string(i) + " data3 z";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data3.w)) return "worldData domainModifiers " + std::to_string(i) + " data3 w";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data4.x)) return "worldData domainModifiers " + std::to_string(i) + " data4 x";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].data4.y)) return "worldData domainModifiers " + std::to_string(i) + " data4 y";
        else if (offset == createKey(&saveData->worldData.domainModifiers[i].data4.z)) return "worldData domainModifiers " + std::to_string(i) + " data4 z";
        else if (offset == createKey(&saveData->worldData.domainModifiers[i].data4.w)) return "worldData domainModifiers " + std::to_string(i) + " data4 w";
        else if (offset == createKey(&saveData->worldData.domainModifiers[i].int1)) return "worldData domainModifiers " + std::to_string(i) + " int1";
        else if (offset == createKey(&saveData->worldData.domainModifiers[i].int2)) return "worldData domainModifiers " + std::to_string(i) + " int2";
        else if (offset == createKey(&saveData->worldData.domainModifiers[i].int3)) return "worldData domainModifiers " + std::to_string(i) + " int3";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].int4)) return "worldData domainModifiers " + std::to_string(i) + " int4";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].int5)) return "worldData domainModifiers " + std::to_string(i) + " int5";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].int6)) return "worldData domainModifiers " + std::to_string(i) + " int6";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].int7)) return "worldData domainModifiers " + std::to_string(i) + " int7";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].int8)) return "worldData domainModifiers " + std::to_string(i) + " int8";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].index1)) return "worldData domainModifiers " + std::to_string(i) + " index1";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].type)) return "worldData domainModifiers " + std::to_string(i) + " type";
		else if (offset == createKey(&saveData->worldData.domainModifiers[i].index1Type)) return "worldData domainModifiers " + std::to_string(i) + " index1Type";
		else if (offset == createKey(&saveData->worldData.indices[i].data1.x)) return "worldData indices " + std::to_string(i) + " data1 x";
		else if (offset == createKey(&saveData->worldData.indices[i].data1.y)) return "worldData indices " + std::to_string(i) + " data1 y";
		else if (offset == createKey(&saveData->worldData.indices[i].data1.z)) return "worldData indices " + std::to_string(i) + " data1 z";
		else if (offset == createKey(&saveData->worldData.indices[i].data1.w)) return "worldData indices " + std::to_string(i) + " data1 w";
        else if (offset == createKey(&saveData->worldData.indices[i].data2.x)) return "worldData indices " + std::to_string(i) + " data2 x";
        else if (offset == createKey(&saveData->worldData.indices[i].data2.y)) return "worldData indices " + std::to_string(i) + " data2 y";
		else if (offset == createKey(&saveData->worldData.indices[i].data2.z)) return "worldData indices " + std::to_string(i) + " data2 z";
		else if (offset == createKey(&saveData->worldData.indices[i].data2.w)) return "worldData indices " + std::to_string(i) + " data2 w";
		else if (offset == createKey(&saveData->worldData.indices[i].data3.x)) return "worldData indices " + std::to_string(i) + " data3 x";
		else if (offset == createKey(&saveData->worldData.indices[i].data3.y)) return "worldData indices " + std::to_string(i) + " data3 y";
		else if (offset == createKey(&saveData->worldData.indices[i].data3.z)) return "worldData indices " + std::to_string(i) + " data3 z";
		else if (offset == createKey(&saveData->worldData.indices[i].data3.w)) return "worldData indices " + std::to_string(i) + " data3 w";
		else if (offset == createKey(&saveData->worldData.indices[i].data4.x)) return "worldData indices " + std::to_string(i) + " data4 x";
		else if (offset == createKey(&saveData->worldData.indices[i].data4.y)) return "worldData indices " + std::to_string(i) + " data4 y";
		else if (offset == createKey(&saveData->worldData.indices[i].data4.z)) return "worldData indices " + std::to_string(i) + " data4 z";
		else if (offset == createKey(&saveData->worldData.indices[i].data4.w)) return "worldData indices " + std::to_string(i) + " data4 w";
		else if (offset == createKey(&saveData->worldData.indices[i].int1)) return "worldData indices " + std::to_string(i) + " int1";
		else if (offset == createKey(&saveData->worldData.indices[i].int2)) return "worldData indices " + std::to_string(i) + " int2";
        else if (offset == createKey(&saveData->worldData.indices[i].int3)) return "worldData indices " + std::to_string(i) + " int3";
        else if (offset == createKey(&saveData->worldData.indices[i].int4)) return "worldData indices " + std::to_string(i) + " int4";
		else if (offset == createKey(&saveData->worldData.indices[i].int5)) return "worldData indices " + std::to_string(i) + " int5";
		else if (offset == createKey(&saveData->worldData.indices[i].int6)) return "worldData indices " + std::to_string(i) + " int6";
		else if (offset == createKey(&saveData->worldData.indices[i].int7)) return "worldData indices " + std::to_string(i) + " int7";
		else if (offset == createKey(&saveData->worldData.indices[i].int8)) return "worldData indices " + std::to_string(i) + " int8";
		else if (offset == createKey(&saveData->worldData.indices[i].index)) return "worldData indices " + std::to_string(i) + " index";
		else if (offset == createKey(&saveData->worldData.indices[i].type)) return "worldData indices " + std::to_string(i) + " type";
    }
    std::cout << "Attribute not found" << std::endl;
    return "";
}

void imguiWindow::createPlayPopup1F(std::string animationKey, float* val, const char* title) {
    if (ImGui::BeginPopupContextItem(title))
    {   
        if(animations->find(animationKey) != animations->end()){
            ImGui::DragFloat("Cur Val", &animations->at(animationKey).curF, 0.001f, -10000000000.0, 10000000000.0, "%.5f", 0);
			ImGui::DragFloat("Min", &animations->at(animationKey).minF, glm::abs(animations->at(animationKey).stepF));
			ImGui::DragFloat("Max", &animations->at(animationKey).maxF, glm::abs(animations->at(animationKey).stepF));
			ImGui::DragFloat("Step", &animations->at(animationKey).stepF, 0.001f, 0.0, 10000000000.0, "%.5f", 0);
            ImGui::DragFloat("Time Between Steps", &animations->at(animationKey).timePerStep, 0.001f, 0.0f, 10000000000.0, "%.5f", 0);
            ImGui::Checkbox("Increasing", &animations->at(animationKey).increasing);
			if (ImGui::Button("Stop")) {
				animations->erase(animationKey);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
			return;
		}
        static float cur = *val, min = 0.0f, max = glm::tau<float>(), step = 0.0f;
        static float timePerStep = 0.016f;
        static bool increasing = true;
        ImGui::DragFloat("Cur Val", &cur, 0.001f, -10000000000.0, 10000000000.0, "%.5f", 0);
        ImGui::DragFloat("Min", &min, glm::abs(step));
        ImGui::DragFloat("Max", &max, glm::abs(step));
        ImGui::DragFloat("Step", &step, 0.001f, 0.0, 10000000000.0, "%.5f", 0);
        ImGui::DragFloat("Time Between Steps", &timePerStep, 0.001f, 0.0f, 10000000000.0, "%.5f", 0);
        ImGui::Checkbox("Increasing", &increasing);
        if (ImGui::Button("Play - Loop")) {
            createAnimationDataF(animationKey, 1, 0, cur, step, min, max, timePerStep, increasing);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Play - Bounce")) {
			// Create animation struct and add it to the currently playing list
			createAnimationDataF(animationKey, 1, 1, cur, step, min, max, timePerStep, increasing);
			ImGui::CloseCurrentPopup();
		}
        ImGui::EndPopup();
    }
}

void imguiWindow::createPlayPopup1I(std::string animationKey, int* val, const char* title) {
    if (ImGui::BeginPopupContextItem(title))
	{
        if(animations->find(animationKey) != animations->end()){
            ImGui::DragInt("Cur Val", &animations->at(animationKey).curI, 1, -10000000000, 10000000000);
            ImGui::DragInt("Min", &animations->at(animationKey).minI, glm::abs(animations->at(animationKey).stepI));
            ImGui::DragInt("Max", &animations->at(animationKey).maxI, glm::abs(animations->at(animationKey).stepI));
            ImGui::DragInt("Step", &animations->at(animationKey).stepI, 1, 0, 10000000000);
            ImGui::DragFloat("Time Between Steps", &animations->at(animationKey).timePerStep, 0.001f, 0.0f, 10000000000.0, "%.5f", 0);
            ImGui::Checkbox("Increasing", &animations->at(animationKey).increasing);
            if (ImGui::Button("stop")) {
				animations->erase(animationKey);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
			return;
		}
		static int cur = *val, min = 30, max = 80, step = 1;
        static float timePerStep = 0.016f;
        static bool increasing = true;
        ImGui::DragInt("Cur Val", &cur, 1, -10000000000, 10000000000);
		ImGui::DragInt("Min", &min, glm::abs(step));
		ImGui::DragInt("Max", &max, glm::abs(step));
		ImGui::DragInt("Step", &step, 1, 0, 10000000000);
		ImGui::DragFloat("Time Between Steps", &timePerStep, 0.001f, 0.0f, 10000000000.0, "%.5f", 0);
        ImGui::Checkbox("Increasing", &increasing);
		if (ImGui::Button("Play - Loop")) {
			createAnimationDataI(animationKey, 0, 0, cur, step, min, max, timePerStep, increasing);
            ImGui::CloseCurrentPopup();
		}
        if (ImGui::Button("Play - Bounce")) {
			createAnimationDataI(animationKey, 0, 1, cur, step, min, max, timePerStep, increasing);
            ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void imguiWindow::createPlayPopup3F(std::string animationKey, float* arr, const char* title) {
    std::string xKey = animationKey + " x";
    std::string yKey = animationKey + " y";
    std::string zKey = animationKey + " z";
    if (ImGui::BeginPopupContextItem(title)) {
        if (animations->find(xKey) != animations->end()) {
            float cur[3] = { animations->at(xKey).curF, animations->at(yKey).curF, animations->at(zKey).curF };
            float min[3] = { animations->at(xKey).minF, animations->at(yKey).minF, animations->at(zKey).minF };
            float max[3] = { animations->at(xKey).maxF, animations->at(yKey).maxF, animations->at(zKey).maxF };
            float step[3] = { animations->at(xKey).stepF, animations->at(yKey).stepF, animations->at(zKey).stepF };
            float timePerStep[3] = { animations->at(xKey).timePerStep, animations->at(yKey).timePerStep, animations->at(zKey).timePerStep };
            ImGui::DragFloat3("Cur Val", cur, 0.001f, -10000000000.0, 10000000000.0, "%.5f", 0);
            ImGui::DragFloat3("Min", min, glm::abs(step[0]));
            ImGui::DragFloat3("Max", max, glm::abs(step[0]));
            ImGui::DragFloat3("Step", step, 0.001f, 0.0, 10000000000.0, "%.5f", 0);
            ImGui::DragFloat3("Time Between Steps", timePerStep, 0.001f, 0.0f, 10000000000.0, "%.5f", 0);
            ImGui::Checkbox("Inc x ", &animations->at(xKey).increasing);
            ImGui::SameLine();
            ImGui::Checkbox("Inc y ", &animations->at(yKey).increasing);
            ImGui::SameLine();
            ImGui::Checkbox("Inc z ", &animations->at(zKey).increasing);
            if (ImGui::Button("Stop")) {
                animations->erase(xKey);
                animations->erase(yKey);
                animations->erase(zKey);
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                return;

            }
            animations->at(xKey).curF = cur[0];
            animations->at(yKey).curF = cur[1];
            animations->at(zKey).curF = cur[2];
            animations->at(xKey).minF = min[0];
            animations->at(yKey).minF = min[1];
            animations->at(zKey).minF = min[2];
            animations->at(xKey).maxF = max[0];
            animations->at(yKey).maxF = max[1];
            animations->at(zKey).maxF = max[2];
            animations->at(xKey).stepF = step[0];
            animations->at(yKey).stepF = step[1];
            animations->at(zKey).stepF = step[2];
            animations->at(xKey).timePerStep = timePerStep[0];
            animations->at(yKey).timePerStep = timePerStep[1];
            animations->at(zKey).timePerStep = timePerStep[2];

            ImGui::EndPopup();
            return;
        }
        static float cur[3] = { arr[0], arr[1], arr[2]};
        static float min[3] = { 0.0f, 0.0f, 0.0f };
        static float max[3] = { glm::tau<float>(), glm::tau<float>(), glm::tau<float>() };
        static float step[3] = { 0.0f, 0.0f, 0.0f };
        static float timePerStep[3] = { 0.016f, 0.016f, 0.016f };
        static bool increasing[3] = { true, true, true };
        ImGui::DragFloat3("Cur Val", cur, 0.001f, -10000000000.0, 10000000000.0, "%.5f", 0);
        ImGui::DragFloat3("Min", min, glm::abs(step[0]));
        ImGui::DragFloat3("Max", max, glm::abs(step[0]));
        ImGui::DragFloat3("Step", step, 0.001f, 0.0, 10000000000.0, "%.5f", 0);
        ImGui::DragFloat3("Time Between Steps", timePerStep, 0.001f, 0.0f, 10000000000.0, "%.5f", 0);
        ImGui::Checkbox("Inc x ", &increasing[0]);
        ImGui::SameLine();
        ImGui::Checkbox("Inc y ", &increasing[1]);
        ImGui::SameLine();
        ImGui::Checkbox("Inc z ", &increasing[2]);
        if (ImGui::Button("Play - Loop")) {
            createAnimationDataF(xKey, 1, 0, cur[0], step[0], min[0], max[0], timePerStep[0], increasing[0]);
            createAnimationDataF(yKey, 1, 0, cur[1], step[1], min[1], max[1], timePerStep[1], increasing[1]);
            createAnimationDataF(zKey, 1, 0, cur[2], step[2], min[2], max[2], timePerStep[2], increasing[2]);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Play - Bounce")) {
            createAnimationDataF(xKey, 1, 1, arr[0], step[0], min[0], max[0], timePerStep[0], increasing[0]);
            createAnimationDataF(yKey, 1, 1, arr[1], step[1], min[1], max[1], timePerStep[1], increasing[1]);
            createAnimationDataF(zKey, 1, 1, arr[2], step[2], min[2], max[2], timePerStep[2], increasing[2]);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void imguiWindow::createPlayPopup4F(std::string animationKey, float* arr, const char* title) {
    std::string xKey = animationKey + " x";
    std::string yKey = animationKey + " y";
    std::string zKey = animationKey + " z";
    std::string wKey = animationKey + " w";
    if (ImGui::BeginPopupContextItem(title)){
        if (animations->find(xKey) != animations->end()) {
            float cur[4] = { animations->at(xKey).curF, animations->at(yKey).curF, animations->at(zKey).curF, animations->at(wKey).curF };
            float min[4] = { animations->at(xKey).minF, animations->at(yKey).minF, animations->at(zKey).minF, animations->at(wKey).minF };
            float max[4] = { animations->at(xKey).maxF, animations->at(yKey).maxF, animations->at(zKey).maxF, animations->at(wKey).maxF };
            float step[4] = { animations->at(xKey).stepF, animations->at(yKey).stepF, animations->at(zKey).stepF, animations->at(wKey).stepF };
            float timePerStep[4] = { animations->at(xKey).timePerStep, animations->at(yKey).timePerStep, animations->at(zKey).timePerStep, animations->at(wKey).timePerStep };
            ImGui::DragFloat4("Cur Val", cur, 0.001f, -10000000000.0, 10000000000.0, "%.5f", 0);
            ImGui::DragFloat4("Min", min, glm::abs(step[0]));
            ImGui::DragFloat4("Max", max, glm::abs(step[0]));
            ImGui::DragFloat4("Step", step, 0.001f, 0.0, 10000000000.0, "%.5f", 0);
            ImGui::DragFloat4("Time Between Steps", timePerStep, 0.001f, 0.0f, 10000000000.0, "%.5f", 0);
            ImGui::Checkbox("Inc x ", &animations->at(xKey).increasing);
            ImGui::SameLine();
            ImGui::Checkbox("Inc y ", &animations->at(yKey).increasing);
            ImGui::SameLine();
            ImGui::Checkbox("Inc z ", &animations->at(zKey).increasing);
            ImGui::SameLine();
            ImGui::Checkbox("Inc w ", &animations->at(wKey).increasing);
            if (ImGui::Button("Stop")) {
                animations->erase(xKey);
                animations->erase(yKey);
                animations->erase(zKey);
                animations->erase(wKey);
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                return;

            }
            animations->at(xKey).curF = cur[0];
            animations->at(yKey).curF = cur[1];
            animations->at(zKey).curF = cur[2];
            animations->at(wKey).curF = cur[3];
            animations->at(xKey).minF = min[0];
            animations->at(yKey).minF = min[1];
            animations->at(zKey).minF = min[2];
            animations->at(wKey).minF = min[3];
            animations->at(xKey).maxF = max[0];
            animations->at(yKey).maxF = max[1];
            animations->at(zKey).maxF = max[2];
            animations->at(wKey).maxF = max[3];
            animations->at(xKey).stepF = step[0];
            animations->at(yKey).stepF = step[1];
            animations->at(zKey).stepF = step[2];
            animations->at(wKey).stepF = step[3];
            animations->at(xKey).timePerStep = timePerStep[0];
            animations->at(yKey).timePerStep = timePerStep[1];
            animations->at(zKey).timePerStep = timePerStep[2];
            animations->at(wKey).timePerStep = timePerStep[3];

            ImGui::EndPopup();
            return;
        }
        static float cur[4] = { arr[0], arr[1], arr[2], arr[3] };
        static float min[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static float max[4] = { glm::tau<float>(), glm::tau<float>(), glm::tau<float>(), glm::tau<float>() };
        static float step[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static float timePerStep[4] = { 0.016f, 0.016f, 0.016f, 0.016f };
        static bool increasing[4] = { true, true, true, true };
        ImGui::DragFloat4("Cur Val", cur, 0.001f, -10000000000.0, 10000000000.0, "%.5f", 0);
        ImGui::DragFloat4("Min", min, glm::abs(step[0]));
        ImGui::DragFloat4("Max", max, glm::abs(step[0]));
        ImGui::DragFloat4("Step", step, 0.001f, 0.0, 10000000000.0, "%.5f", 0);
        ImGui::DragFloat4("Time Between Steps", timePerStep, 0.001f, 0.0f, 10000000000.0, "%.5f", 0);
        ImGui::Checkbox("Inc x", &increasing[0]);
        ImGui::SameLine();
        ImGui::Checkbox("Inc y ", &increasing[1]);
        ImGui::SameLine();
        ImGui::Checkbox("Inc z ", &increasing[2]);
        ImGui::SameLine();
        ImGui::Checkbox("Inc w ", &increasing[3]);
        if (ImGui::Button("Play - Loop")) {
            createAnimationDataF(xKey, 1, 0, cur[0], step[0], min[0], max[0], timePerStep[0], increasing[0]);
            createAnimationDataF(yKey, 1, 0, cur[1], step[1], min[1], max[1], timePerStep[1], increasing[1]);
            createAnimationDataF(zKey, 1, 0, cur[2], step[2], min[2], max[2], timePerStep[2], increasing[2]);
            createAnimationDataF(wKey, 1, 0, cur[3], step[3], min[3], max[3], timePerStep[3], increasing[3]);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Play - Bounce")) {
            createAnimationDataF(xKey, 1, 1, cur[0], step[0], min[0], max[0], timePerStep[0], increasing[0]);
            createAnimationDataF(yKey, 1, 1, cur[1], step[1], min[1], max[1], timePerStep[1], increasing[1]);
            createAnimationDataF(zKey, 1, 1, cur[2], step[2], min[2], max[2], timePerStep[2], increasing[2]);
            createAnimationDataF(wKey, 1, 1, cur[3], step[3], min[3], max[3], timePerStep[3], increasing[3]);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void imguiWindow::drawSubstructure(int type, int index, int depth, std::string prefix, int* keys) {
    if (depth > MAX_SUBSTRUCTURE_DEPTH) {
		return;
	}
    int prevKeysValue = keys[type];
    keys[type] = index;
    switch (type) {
        case 1:
            drawObject(index, depth+1, prefix, keys);
			break;
        case 2:
			drawCombineModifier(index, depth + 1, keys);
            break;
		case 3:
            drawDomainModifier(index, depth + 1, keys);
            break;

    }
    keys[type] = prevKeysValue;
}

void imguiWindow::drawObject(int i, int depth, std::string prefix, int* keys) {

    std::string collasping_header = prefix + "Object " + std::to_string(i) + " - " + worldObjectTypes[saveData->worldData.objects[i].type];

    if (!alreadyOpen[keys[0]][keys[1]][keys[2]][keys[3]][keys[4]] && ImGui::TreeNode((void*)(intptr_t)i, collasping_header.c_str())) {
        ImGui::ListBox("Object Type", &saveData->worldData.objects[i].type, [](void* data, int idx, const char** out_text) {
            auto& worlds = *static_cast<std::vector<std::string>*>(data);
            *out_text = worlds[idx].c_str();
            return true;
            }, &worldObjectTypes, worldObjectTypes.size(), 3);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The type of the object, select a type to see its settings");
        ImGui::SliderInt("Bump Map Index", &saveData->worldData.objects[i].int3, 0, MAX_IMAGES - 1);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The texture index of the bump map, setting to 0 will use no bump map");
        ImGui::SliderInt("Texture Index", &saveData->worldData.objects[i].textureIndex, 0, MAX_IMAGES - 1);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The texture index of the object, setting to 0 will allow for solid color");
        if (saveData->worldData.objects[i].textureIndex != 0 || saveData->worldData.objects[i].int3 != 0) {
            ImGui::DragFloat3("Texture Scale", &saveData->worldData.objects[i].data1[0], 0.001f, -100.0f, 100.0f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The scale of the texture");
            createPlayPopup3F("worldData objects " + std::to_string(i) + " data1", (float*)(&saveData->worldData.objects[i].data1[0]), "Texture Scale");
            ImGui::DragFloat3("Texture Offset", &saveData->worldData.objects[i].data2[0], 0.001f, -100.0f, 100.0f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The offset of the texture");
            createPlayPopup3F("worldData objects " + std::to_string(i) + " data2", (float*)(&saveData->worldData.objects[i].data2[0]), "Texture Offset");   
        }
        if (saveData->worldData.objects[i].textureIndex != 0) {
            ImGui::SliderInt("Texture Coordinate Type", &saveData->worldData.objects[i].int2, 0, 3);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Allows you to pick different ways the texture can be applied to the object.");
        }
        ImGui::ColorEdit3("Color", &saveData->worldData.objects[i].color[0]);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the object");
        createPlayPopup3F("worldData objects " + std::to_string(i) + " color", (float*)(&saveData->worldData.objects[i].color), "Color");
        if (ImGui::Button("Normalize Color")) {
            saveData->worldData.objects[i].color[0] = -2.0f;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Makes the color of the object based of the angle of the surface");
        ImGui::SliderInt("Object Based Normal", &saveData->worldData.objects[i].int4, 0, 1);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Switches between the normal being based off the object or off the world.");
        ImGui::DragFloat3("Center Pos", &saveData->worldData.objects[i].center[0], 0.1f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The center position of the object");
        createPlayPopup3F("worldData objects " + std::to_string(i) + " center", (float*)(&saveData->worldData.objects[i].center), "Center Pos");
        formatObject(i);
        //ImGui::SliderInt("Is Negated?", &saveData->worldData.objects[i].is_negated, 0, 1);
        //createPlayPopup1I("worldData objects " + std::to_string(i) + " is_negated", &saveData->worldData.objects[i].is_negated, "Is Negated?");
        ImGui::DragFloat("Shadow Blur", &saveData->worldData.objects[i].shadow_blur, 0.1, -0.1, 20);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("How sharp the shadow is, negative number means no shadow");
        createPlayPopup1F("worldData objects " + std::to_string(i) + " shadow_blur", &saveData->worldData.objects[i].shadow_blur, "Has Shadow?");
        if (saveData->worldData.objects[i].shadow_blur >= 0.0f) {
            ImGui::DragFloat("Shadow Intensity", &saveData->worldData.objects[i].shadow_intensity, 0.001, 0, 1);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("How dark the shadow is");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " shadow_intensity", &saveData->worldData.objects[i].shadow_intensity, "Has Shadow?");
        }
        ImGui::DragFloat("Reflectivity", &saveData->worldData.objects[i].reflectivity, 0.001f, 0.0f, 1.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("How much reflective a object is");
        createPlayPopup1F("worldData objects " + std::to_string(i) + " reflectivity", &saveData->worldData.objects[i].reflectivity, "Reflectivity");
        ImGui::DragFloat("Transparency", &saveData->worldData.objects[i].transparency, 0.001f, 0.0f, 1.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("How transparent a object is");
        createPlayPopup1F("worldData objects " + std::to_string(i) + " transparency", &saveData->worldData.objects[i].transparency, "Transparency");
        ImGui::DragFloat("Diffuse Intensity", &saveData->worldData.objects[i].diffuse_intensity, 0.001f, 0.0f, 1.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("How light is lost based on its angle verse the light");
        createPlayPopup1F("worldData objects " + std::to_string(i) + " diffuse_intensity", &saveData->worldData.objects[i].diffuse_intensity, "Diffuse Intensity");
        ImGui::DragFloat("Refractive Index", &saveData->worldData.objects[i].refractive_index, 0.001f, 0.0f, 2.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("How much light is bent when it enters the object");
        createPlayPopup1F("worldData objects " + std::to_string(i) + " refractive_index", &saveData->worldData.objects[i].refractive_index, "Refractive Index");
        ImGui::TreePop();
    }
    alreadyOpen[keys[0]][keys[1]][keys[2]][keys[3]][keys[4]] = true;
}

void imguiWindow::formatObject(int i) {
    switch (saveData->worldData.objects[i].type) {
        case 1:
            ImGui::DragFloat3("Normal Vector", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The normal vector of the plane. This is the direction the plane will face");
            createPlayPopup3F("worldData objects " + std::to_string(i) + " size", (float*)(& saveData->worldData.objects[i].size), "Normal Vector");
            break;
        case 2:
            ImGui::DragFloat("Radius", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the sphere");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size x", &saveData->worldData.objects[i].size[0], "Radius");
            break;
        case 3:
            ImGui::DragFloat3("Size", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The dimensions of the box");
            createPlayPopup3F("worldData objects " + std::to_string(i) + " size", (float*)(&saveData->worldData.objects[i].size), "Size");
            break;
        case 4:
            ImGui::DragFloat("X Size", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The X size of the pillar");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size x", &saveData->worldData.objects[i].size[0], "X Size");
            ImGui::DragFloat("Z Size", &saveData->worldData.objects[i].size[1], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The Z size of the pillar");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size y", &saveData->worldData.objects[i].size[1], "Z Size");
            break;
        case 5:
        case 6:
            break;
        case 7:
        case 8:
            ImGui::DragFloat("Radius", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the cylinder");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size x", &saveData->worldData.objects[i].size[0], "Radius");
            ImGui::DragFloat("Height", &saveData->worldData.objects[i].size[1], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The height of the cylinder");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size y", &saveData->worldData.objects[i].size[1], "Height");
            break;
        case 9:
            ImGui::DragFloat("Radius", &saveData->worldData.objects[i].size[1], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the torus");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size y", &saveData->worldData.objects[i].size[1], "Radius");
            ImGui::DragFloat("Thickness", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The thickness of the torus");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size x", &saveData->worldData.objects[i].size[0], "Thickness");
            break;
        case 10:
        case 11:
            ImGui::DragFloat("Radius", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the ring");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size x", &saveData->worldData.objects[i].size[0], "Radius");
            break;
        case 12:
        case 13:
        case 14:
            ImGui::DragFloat("Radius", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the circle");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size x", &saveData->worldData.objects[i].size[0], "Radius");
            ImGui::DragFloat("Height", &saveData->worldData.objects[i].size[1], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The height of the prism");
            createPlayPopup1F("worldData objects " + std::to_string(i) + " size y", &saveData->worldData.objects[i].size[1], "Height");
            break;
        case 15:
			ImGui::DragFloat("Radius", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the sphere");
			createPlayPopup1F("worldData objects " + std::to_string(i) + " size x", &saveData->worldData.objects[i].size[0], "Radius");
			break;
        case 16:
            ImGui::DragFloat3("Normal Vector", &saveData->worldData.objects[i].size[0], 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The normal vector of the plane. This is the direction the plane will face");
            createPlayPopup3F("worldData objects " + std::to_string(i) + " size", (float*)(&saveData->worldData.objects[i].size), "Normal Vector");
            break;
    }

}

void imguiWindow::descriptionObject(int i) {
    switch (i) {
    case 0: 
		ImGui::TextWrapped("No object type selected");
		break;
    case 1:
        ImGui::TextWrapped("A plane is a flat surface that extends infinitely in all directions. It is defined by a point and a normal vector");
        break;
    case 2:
        ImGui::TextWrapped("A simple ball");
        break;
    case 3:
        ImGui::TextWrapped("A simple box");
        break;
    case 4:
        ImGui::TextWrapped("A square that extends infinitely up and down");
        break;
    case 5:
        ImGui::TextWrapped("A box that extends infinitely from a corner");
        break;
    case 6:
        ImGui::TextWrapped("A sphere with bumps on it, you can not define its radius at this time");
        break;
    case 7:
        ImGui::TextWrapped("A simple cylinder");
		break;
    case 8:
		ImGui::TextWrapped("A cylinder with rounded ends");
		break;
    case 9:
        ImGui::TextWrapped("A donut shape");
        break;
    case 10:
        ImGui::TextWrapped("A 2D ring, a torus with 0 thickness");
        break;
    case 11:
		ImGui::TextWrapped("A 2D circle, filled in");
		break;
    case 12:
		ImGui::TextWrapped("A hexagonal prism, with the sides tangent to the circle");
        break;
    case 13:
		ImGui::TextWrapped("A hexagonal prism, with vertices on the circle");
		break;
    case 14:
        ImGui::TextWrapped("A simple cone");
		break;
    case 15:
        ImGui::TextWrapped("A sphere that wiggles over time");
		break;
    case 16:
		ImGui::TextWrapped("A plane that wiggles over time");
        break;
    }

}

void imguiWindow::drawCombineModifier(int i, int depth, int* keys) {
    std::string collasping_header = "Combine Modifier " + std::to_string(i) + " - " + worldObjectCombineModifierTypes[saveData->worldData.combineModifiers[i].type];


    if (!alreadyOpen[keys[0]][keys[1]][keys[2]][keys[3]][keys[4]] && ImGui::TreeNode((void*)(intptr_t)((i)+32), collasping_header.c_str())) {
        ImGui::SliderInt("Combine Obj #", &saveData->worldData.combineModifiers[i].index2, 0, MAX_OBJECTS);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The id of the object to be used for combining");
        ImGui::ListBox("Combine Type", &saveData->worldData.combineModifiers[i].type, [](void* data, int idx, const char** out_text) {
            auto& worlds = *static_cast<std::vector<std::string>*>(data);
            *out_text = worlds[idx].c_str();
            return true;
            }, &worldObjectCombineModifierTypes, worldObjectCombineModifierTypes.size(), 3);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The type of combination");
        formatCombineModifier(i);
        keys[4] = 1;
        drawSubstructure(1, saveData->worldData.combineModifiers[i].index2, depth, "Combine ", keys);
        keys[4] = 0;
        ImGui::SliderInt("Substructure #", &saveData->worldData.combineModifiers[i].index1, 0, MAX_OBJECTS);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The id of the substructure that will be combined with");
        ImGui::ListBox("Substructure Type", &saveData->worldData.combineModifiers[i].index1Type, [](void* data, int idx, const char** out_text) {
            auto& worlds = *static_cast<std::vector<std::string>*>(data);
            *out_text = worlds[idx].c_str();
            return true;
            }, &worldObjectIndexTypes, worldObjectIndexTypes.size(), 3);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The type of substructure");
        ImGui::TreePop();
    }
    alreadyOpen[keys[0]][keys[1]][keys[2]][keys[3]][keys[4]] = true;
    drawSubstructure(saveData->worldData.combineModifiers[i].index1Type, saveData->worldData.combineModifiers[i].index1, depth, "", keys);
}

void imguiWindow::formatCombineModifier(int i) {
    switch (saveData->worldData.combineModifiers[i].type) {
        case 4:
        case 5:
        case 6:
            ImGui::DragFloat("Depth", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The depth of the chamfer");
            createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1", &saveData->worldData.combineModifiers[i].data1.x, "Chamfer Depth");
            break;
        case 7:
        case 8:
        case 9:
            ImGui::DragFloat("Radius", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the round");
			createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1", &saveData->worldData.combineModifiers[i].data1.x, "Round Radius");
			break;
        case 10:
        case 11:
        case 12:
            ImGui::DragFloat("Columns", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The number of layers of columns");
			createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1", &saveData->worldData.combineModifiers[i].data1.x, "Columns");
            ImGui::DragFloat("Column Width", &saveData->worldData.combineModifiers[i].data1.y, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The width of the columns");
            createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1 y", &saveData->worldData.combineModifiers[i].data1.y, "Column Width");
			break;
		case 13:
		case 14:
		case 15:
			ImGui::DragFloat("Stairs", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The number of stairs");
            createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1", &saveData->worldData.combineModifiers[i].data1.x, "Stairs");
            ImGui::DragFloat("Stair Height", &saveData->worldData.combineModifiers[i].data1.y, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The height of the stairs");
            createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1 y", &saveData->worldData.combineModifiers[i].data1.y, "Stair Height");
            break;
        case 16:
            ImGui::DragFloat("Radius", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the round");
            createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1", &saveData->worldData.combineModifiers[i].data1.x, "Radius");
            break;
        case 17:
			ImGui::DragFloat("Radius", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The radius of the round");
			createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1", &saveData->worldData.combineModifiers[i].data1.x, "Radius");
			ImGui::DragFloat("Width", &saveData->worldData.combineModifiers[i].data1.y, 0.1f);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The width of the round");
			createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1 y", &saveData->worldData.combineModifiers[i].data1.y, "Width");
			break;
        case 18:
        case 19:
			ImGui::DragFloat("Depth", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The depth of the cut");
			createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1", &saveData->worldData.combineModifiers[i].data1.x, "Depth");
			break;
        case 20:
            ImGui::DragFloat("Depth", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The depth of the cut");
            createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1", &saveData->worldData.combineModifiers[i].data1.x, "Depth");
            ImGui::DragFloat("Width", &saveData->worldData.combineModifiers[i].data1.y, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The width of the cut");
            createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1 y", &saveData->worldData.combineModifiers[i].data1.y, "Width");
            break;
        case 21:
            ImGui::DragFloat("Buffer Depth", &saveData->worldData.combineModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("How close to the buffer until it checks the full object");
			createPlayPopup1F("worldData combineModifiers " + std::to_string(i) + " data1 x", &saveData->worldData.combineModifiers[i].data1.x, "Buffer Depth");
			break;
    }
}

void imguiWindow::descriptionCombineModifier(int i) {
    switch (i) {
    case 0:
        ImGui::TextWrapped("No Modifier Type Selected");
        break;
    case 1:
        ImGui::TextWrapped("A simple combination of two objects");
        break;
    case 2:
        ImGui::TextWrapped("The intersection of two objects");
        break;
    case 3:
        ImGui::TextWrapped("Cut the combination object out of the substructure");
		break;
    case 4:
		ImGui::TextWrapped("Combination of objects with chamfered edges");
		break;
    case 5:
        ImGui::TextWrapped("Intersection of objects with chamfered edges");
        break;
    case 6:
        ImGui::TextWrapped("Difference of objects with chamfered edges");
		break;
    case 7:
		ImGui::TextWrapped("Combination of objects with rounded edges");
        break;
    case 8:
        ImGui::TextWrapped("Intersection of objects with rounded edges");
		break;
    case 9:
		ImGui::TextWrapped("Difference of objects with rounded edges");
        break;
    case 10:
		ImGui::TextWrapped("Combination of objects with rings around edges");
        break;
    case 11:
		ImGui::TextWrapped("Intersection of objects with rings around edges");
        break;
    case 12:
        ImGui::TextWrapped("Difference of objects with rings around edges");
		break;
    case 13:
		ImGui::TextWrapped("Combination of objects with staired edges");
		break;
    case 14:
		ImGui::TextWrapped("Intersection of objects with staired edges");
        break;
    case 15:
        ImGui::TextWrapped("Difference of objects with staired edges");
        break;
    case 16:
        ImGui::TextWrapped("Combination of objects with better rounded edges");
		break;
    case 17:
		ImGui::TextWrapped("Intersection of objects with the intersections covered in piping");
		break;
    case 18:
		ImGui::TextWrapped("Difference of objects with the interections used to cut - triagle cut");
		break;
    case 19:
        ImGui::TextWrapped("Intersection of objects with it cut off bases on the disance from one of the objects");
        break;
    case 20:
        ImGui::TextWrapped("Checks the combination object first and if the ray is too far away it skips calculating the full object");
        break;
    case 21:
        ImGui::TextWrapped("Cut the substructure out of the combination object");
        break;
    }
}

void imguiWindow::drawDomainModifier(int i, int depth, int* keys) {
    std::string collasping_header = "Domain Modifier " + std::to_string(i) + " - " + worldObjectDomainModifierTypes[saveData->worldData.domainModifiers[i].type];

    if (!alreadyOpen[keys[0]][keys[1]][keys[2]][keys[3]][keys[4]] && ImGui::TreeNode((void*)(intptr_t)((i)+64), collasping_header.c_str())) {
        ImGui::ListBox("Domain Type", &saveData->worldData.domainModifiers[i].type, [](void* data, int idx, const char** out_text) {
            auto& worlds = *static_cast<std::vector<std::string>*>(data);
            *out_text = worlds[idx].c_str();
            return true;
            }, &worldObjectDomainModifierTypes, worldObjectDomainModifierTypes.size(), 3);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The type of domain modifier");
        formatDomainModifier(i);
        ImGui::SliderInt("Substructure #", &saveData->worldData.domainModifiers[i].index1, 0, MAX_OBJECTS);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The id of the substructure that this domain modifier will be applied to");
        ImGui::ListBox("Substructure Type", &saveData->worldData.domainModifiers[i].index1Type, [](void* data, int idx, const char** out_text) {
            auto& worlds = *static_cast<std::vector<std::string>*>(data);
            *out_text = worlds[idx].c_str();
            return true;
            }, &worldObjectIndexTypes, worldObjectIndexTypes.size(), 3);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The type of the substructure that this domain modifier will be applied to");
        ImGui::TreePop();
    }
    alreadyOpen[keys[0]][keys[1]][keys[2]][keys[3]][keys[4]] = true;
    drawSubstructure(saveData->worldData.domainModifiers[i].index1Type, saveData->worldData.domainModifiers[i].index1, depth, "", keys);
}

void imguiWindow::formatDomainModifier(int i) {
    switch (saveData->worldData.domainModifiers[i].type) {
        case 1: // Translate 3D
            ImGui::DragFloat3("Pos Delta", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The amount to move the substructure");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Pos Delta");
            break;
        case 2: // Scale 3D
			ImGui::DragFloat3("Multiplier", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The ratio to scale the substructure by");
			createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Multiplier");
			break;
        case 3: // Rotate 3D
            ImGui::DragFloat3("Angle", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The angle to rotate the substructure by");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Angle");
            break;
        case 4: // Reflect Plane
            ImGui::DragFloat3("Mirror Normal Vector", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The normal vector to mirror the substructure across");
			createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Mirror Normal Vector");
			break;
        case 5: // Repeat 3D
        case 6: // Repeat Mirror 3D
        case 7: // Extend 3D
            ImGui::DragFloat3("Spacing", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The spacing between instances of the substructures");
			createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Spacing");
			break;
        case 8: // Multiply 3D
            ImGui::DragFloat3("Spacing", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The spacing between instances of the substructures");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Spacing");
            ImGui::DragFloat3("Count", &saveData->worldData.domainModifiers[i].data2.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The number of instances of the substructures");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data2", (float*)(&saveData->worldData.domainModifiers[i].data2.x), "Count");
            break;
        case 9: // Ring 3D
            ImGui::DragFloat3("Count", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The number of slices");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Count");
			break;
        case 10: // Octant 3D
            ImGui::DragFloat3("Gap 1", &saveData->worldData.domainModifiers[i].data2.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The gap between the substructure");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data2", (float*)(&saveData->worldData.domainModifiers[i].data2.x), "Gap 1");
            ImGui::DragFloat3("Gap 2", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The gap between the substructure");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Gap 2");
            break;
        case 11: // Twist 3D
        case 12: // Bend 3D
            ImGui::DragFloat3("idk... 1", &saveData->worldData.domainModifiers[i].data2.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Some of these work, so are wonky, explore");
			createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data2", (float*)(&saveData->worldData.domainModifiers[i].data2.x), "idk... 1");
			ImGui::DragFloat3("idk... 2", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
        	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Some of these work, so are wonky, test them out");
			createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "idk... 2");
			break;
        case 13: // Wiggle World Move 3D
        case 14: // Wiggle Cam Move 3D
            ImGui::DragFloat3("Wave Width 1", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The width of the wave");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Wave Width 1");
            ImGui::DragFloat3("Wave Height 1", &saveData->worldData.domainModifiers[i].data2.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The height of the wave");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data2", (float*)(&saveData->worldData.domainModifiers[i].data2.x), "Wave Height 1");
            ImGui::DragFloat3("Wave Width 2", &saveData->worldData.domainModifiers[i].data3.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The width of the wave");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data3", (float*)(&saveData->worldData.domainModifiers[i].data3.x), "Wave Width 2");
            ImGui::DragFloat3("Wave Height 2", &saveData->worldData.domainModifiers[i].data4.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The height of the wave");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data4", (float*)(&saveData->worldData.domainModifiers[i].data4.x), "Wave Height 2");
            ImGui::DragFloat("Wave Speed x", &saveData->worldData.domainModifiers[i].data1.w, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The speed of the wave on the x axis");
            createPlayPopup1F("worldData domainModifiers " + std::to_string(i) + " data1 w", &saveData->worldData.domainModifiers[i].data1.w, "Wave Speed x");
            ImGui::DragFloat("Wave Speed y", &saveData->worldData.domainModifiers[i].data2.w, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The speed of the wave on the y axis");
            createPlayPopup1F("worldData domainModifiers " + std::to_string(i) + " data2 w", &saveData->worldData.domainModifiers[i].data2.w, "Wave Speed y");
            ImGui::DragFloat("Wave Speed z", &saveData->worldData.domainModifiers[i].data3.w, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The speed of the wave on the z axis");
            createPlayPopup1F("worldData domainModifiers " + std::to_string(i) + " data3 w", &saveData->worldData.domainModifiers[i].data3.w, "Wave Speed z");
            break;
        case 15: // Rotate World Move 3D
        case 16: // Rotate Cam Move 3D
            ImGui::DragFloat3("Rotation Amount", &saveData->worldData.domainModifiers[i].data1.x, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The amount to rotate the space by");
            createPlayPopup3F("worldData domainModifiers " + std::to_string(i) + " data1", (float*)(&saveData->worldData.domainModifiers[i].data1.x), "Rotation Amount");
            ImGui::DragFloat("Rotation Speed x", &saveData->worldData.domainModifiers[i].data1.w, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The speed of the rotation on the x axis");
            createPlayPopup1F("worldData domainModifiers " + std::to_string(i) + " data1 w", &saveData->worldData.domainModifiers[i].data1.w, "Wave Speed x");
            ImGui::DragFloat("Rotation Speed y", &saveData->worldData.domainModifiers[i].data2.w, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The speed of the rotation on the y axis");
            createPlayPopup1F("worldData domainModifiers " + std::to_string(i) + " data2 w", &saveData->worldData.domainModifiers[i].data2.w, "Wave Speed y");
            ImGui::DragFloat("Rotation Speed z", &saveData->worldData.domainModifiers[i].data3.w, 0.1f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The speed of the rotation on the z axis");
            createPlayPopup1F("worldData domainModifiers " + std::to_string(i) + " data3 w", &saveData->worldData.domainModifiers[i].data3.w, "Wave Speed z");
			break;
        case 17: // Look At Camera
		break;
    }
}

void imguiWindow::descriptionDomainModifier(int i) {
    switch (i) {
        case 0:
            ImGui::TextWrapped("No Modifier Type Selected"); //Nothing
            break;
        case 1:
            ImGui::TextWrapped("Move the substructure by a set amount"); // Translate 3D
            break;
        case 2:
            ImGui::TextWrapped("Scale the substructure by a set amount"); // Scale 3D
            break;
        case 3:
            ImGui::TextWrapped("Rotate the substructure by a set amount"); // Rotate 3D
            break;
        case 4:
            ImGui::TextWrapped("Mirror the substructure over a plane"); // Reflect Plane
            break;
        case 5:
            ImGui::TextWrapped("Repeat the substructure in a grid"); // Repeat 3D
            break;
        case 6:
            ImGui::TextWrapped("Repeat the substructure in a grid but flip it everytime"); // Repeat Mirror 3D
            break;
        case 7:
            ImGui::TextWrapped("Repeat the substructure in a grid on one corner"); // Extend 3D
            break;
        case 8:
            ImGui::TextWrapped("Repeat the substructure in a grid on one corner a set number of times"); // Multiply 3D
            break;
        case 9:
            ImGui::TextWrapped("Repeat the substructure around the origin a set number of times"); // Ring 3D
            break;
        case 10:
            ImGui::TextWrapped("Mirror the substructure around the origin"); // Octant 3D
            break;
        case 11:
            ImGui::TextWrapped("Not working - Twist the object around the origin"); // Twist 3D
            break;
        case 12:
            ImGui::TextWrapped("Not working - Bend the object around the origin"); // Bend 3D
            break;
        case 13:
            ImGui::TextWrapped("Make the world wiggle - Wiggles based off position to the origin"); // Wiggle World Move 3D
            break;
        case 14:
            ImGui::TextWrapped("Make the world wiggle - Wiggles based off position to the camera"); // Wiggle Cam Move 3D
            break;
        case 15:
            ImGui::TextWrapped("Rotates the world - Axis based off the origin"); // Rotate World Move 3D
            break;
        case 16:
            ImGui::TextWrapped("Rotates the world - Axis based off the camera"); // Rotate Cam Move 3D
            break;
        case 17:
            ImGui::TextWrapped("Makes the object face the camera/ray"); // Look At Camera
            break;
    }
}

void imguiWindow::drawIndex(int i, int depth, int* keys) {
    keys[0] = i;
    std::string collasping_header = "Sturcture " + std::to_string(i);

    if (ImGui::TreeNode((void*)(intptr_t)(i+96), collasping_header.c_str())) {
        ImGui::SliderInt("Substructure #", &saveData->worldData.indices[i].index, 0, MAX_OBJECTS);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The id of the top substructure");
        ImGui::ListBox("Substructure Type", &saveData->worldData.indices[i].type, [](void* data, int idx, const char** out_text) {
            auto& worlds = *static_cast<std::vector<std::string>*>(data);
            *out_text = worlds[idx].c_str();
            return true;
            }, &worldObjectIndexTypes, worldObjectIndexTypes.size(), 3);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("The type of substructure");
        drawSubstructure(saveData->worldData.indices[i].type, saveData->worldData.indices[i].index, depth, "", keys);
        ImGui::TreePop();
    }
}

void imguiWindow::descriptionIndex(int i) {
    switch (i) {
    case 0:
        ImGui::TextWrapped("No Substructure Type Selected");
		break;
    case 1:
        ImGui::TextWrapped("A base object, these are the things that are actually drawing");
        break;
    case 2:
        ImGui::TextWrapped("A combine modifier, these allow you to combine an object with another substructure");
		break;
    case 3:
        ImGui::TextWrapped("A domain modifier, these allow you to modify space");
		break;
    }
}

bool imguiWindow::fileExists(char* fileNameIn, std::string extension) {
    char fullPath[256]; // Buffer to hold the full path
    snprintf(fullPath, sizeof(fullPath), "%s%s%s%s", worldSavesPath, version, fileNameIn, extension.c_str());
    //std::cout << fullPath << std::endl;
    std::ifstream file(fullPath);
    return file.good();
}

bool imguiWindow::textureExists(std::string textureN) {
    std::ifstream file(textureN);
    return file.good();
}

bool imguiWindow::validFileName(){
    for (int i = 0; i < strlen(fileName); i++) {
		if (fileName[i] == '\\' || fileName[i] == '/' || fileName[i] == ':' || fileName[i] == '*' || fileName[i] == '?' || fileName[i] == '"' || fileName[i] == '<' || fileName[i] == '>' || fileName[i] == '|') {
			return false;
		}
	}
    return true;
}

size_t imguiWindow::createKey(void* memberPtr) {
    return reinterpret_cast<size_t>(memberPtr) - reinterpret_cast<size_t>(saveData);
}

void imguiWindow::drawWindow() {
    if (*inputEnabled == true) {
        return;
    }

    //ImGui::ShowDemoWindow();

    int screenWidth, screenHeight;
    glfwGetWindowSize(vkRenderer->getWindow(), &screenWidth, &screenHeight);

    // Set a static width and dynamic height
    float staticWidth = 400.0f; // Example static width
    float dynamicHeight = static_cast<float>(screenHeight); // Use the full height of the window

    ImGui::SetNextWindowSize(ImVec2(staticWidth, dynamicHeight), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    //ImGui::SetNextWindowSize(ImVec2(staticWidth, dynamicHeight), ImGuiCond_Always);
    //ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGui::Begin("World Builder", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImGui::PushItemWidth(200.0f);

    // Create the tab bar
    if (ImGui::BeginTabBar("MyTabBar"))
    {
        // First tab
        if (ImGui::BeginTabItem("World"))
        {
            if (ImGui::Button("Test/Enter World")) {
                enableInput();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("This Button allows you to enter the world and move around");
            ImGui::SameLine();
            ImGui::TextWrapped("Press 'Esc' to reenable the UI");

            if (ImGui::CollapsingHeader("Save/Load")) {
                ImGui::InputText("Input Text", fileName, IM_ARRAYSIZE(fileName));
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Input the name of the file you want to save/load/delete");

                std::vector<std::string> savedWorlds;
                getSavedWorlds(savedWorlds);

                static int selected = -1;

                ImGui::ListBox("SavedWorlds", &selected, [](void* data, int idx, const char** out_text) {
                    auto& worlds = *static_cast<std::vector<std::string>*>(data);
                    *out_text = worlds[idx].c_str();
                    return true;
                    }, &savedWorlds, savedWorlds.size(), 4);

                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select a saved world to load or delete");

                if (selected >= 0 && selected < savedWorlds.size()) {
                    snprintf(fileName, sizeof(fileName), "%s", savedWorlds[selected].c_str());
                    selected = -1;
                }

                if (ImGui::Button("Save World Data")) {
                    if (strlen(fileName) == 0 || !validFileName()) {
                        ImGui::OpenPopup("InvalidName");
                    }
                    else if (fileExists(fileName, fileExtension)) {
                        ImGui::OpenPopup("SaveWorldData");
                    }
                    else {
                        saveWorldDataJson();
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Save the current world data to the inputted file name");

                // Set the popup position before opening it
                if (ImGui::IsPopupOpen("SaveWorldData")) {
                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    ImVec2 center = ImVec2(viewport->GetCenter().x - 200, viewport->GetCenter().y - 100); // Approximate window size
                    ImGui::SetNextWindowPos(center, ImGuiCond_Always);
                }
                if (ImGui::BeginPopupModal("SaveWorldData", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextWrapped((std::string("Are you sure you want to overwrite \"") + std::string(fileName) + std::string("\", the old save will be lost?")).c_str());
                    ImGui::Separator();
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        saveWorldDataJson();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Load World Data")) {
                    if (strlen(fileName) == 0 || !fileExists(fileName, fileExtension)) {
                        ImGui::OpenPopup("InvalidName");
                    }
                    else {
                        ImGui::OpenPopup("LoadWorldData");
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Load the world data from the inputted file name");

                // Set the popup position before opening it
                if (ImGui::IsPopupOpen("LoadWorldData")) {
                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    ImVec2 center = ImVec2(viewport->GetCenter().x - 200, viewport->GetCenter().y - 100); // Approximate window size
                    ImGui::SetNextWindowPos(center, ImGuiCond_Always);
                }
                if (ImGui::BeginPopupModal("LoadWorldData", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextWrapped((std::string("Are you sure you want to load \"") + std::string(fileName) + std::string("\", your current world will be lost?")).c_str());
                    ImGui::Separator();
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        loadWorldDataJson(false);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete World Data")) {
                    if (strlen(fileName) == 0 || !fileExists(fileName, fileExtension)) {
                        ImGui::OpenPopup("InvalidName");
                    }
                    else {
                        ImGui::OpenPopup("DeleteWorldData");
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete the world data from the inputted file name");

                // Set the popup position before opening it
                if (ImGui::IsPopupOpen("DeleteWorldData")) {
                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    ImVec2 center = ImVec2(viewport->GetCenter().x - 200, viewport->GetCenter().y - 100); // Approximate window size
                    ImGui::SetNextWindowPos(center, ImGuiCond_Always);
                }
                if (ImGui::BeginPopupModal("DeleteWorldData", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextWrapped((std::string("Are you sure you want to delete \"") + std::string(fileName) + std::string("\", the old save will be lost?")).c_str());
                    ImGui::Separator();
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        deleteWorld();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }

                // Set the popup position before opening it
                if (ImGui::IsPopupOpen("InvalidName")) {
                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    ImVec2 center = ImVec2(viewport->GetCenter().x - 200, viewport->GetCenter().y - 100); // Approximate window size
                    ImGui::SetNextWindowPos(center, ImGuiCond_Always);
                }
                if (ImGui::BeginPopupModal("InvalidName", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextWrapped((std::string("The inputted name \"") + std::string(fileName) + std::string("\" is invalid.")).c_str());
                    if (ImGui::Button("OK", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
            }

            if (ImGui::CollapsingHeader("Textures")) {
                std::vector<std::string> textureNames = { INIT_TEXTURE };
                getTextures(textureNames);

                static int selected = 0;
                static int prevSelected = 0;
                static int textureIndex = 1;

                ImGui::SliderInt("Texture Index", &textureIndex, 1, MAX_IMAGES-1);

                ImGui::ListBox("Texture", &selected, [](void* data, int idx, const char** out_text) {
                    auto& worlds = *static_cast<std::vector<std::string>*>(data);
                    worlds[idx] = worlds[idx].substr(worlds[idx].find_first_of('\\') + 1, worlds[idx].find_last_of('.') - (worlds[idx].find_first_of('\\') + 1));
                    *out_text = worlds[idx].c_str();
                    return true;
                    }, & textureNames, textureNames.size(), 3);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select a image to for this texture number. These can be selected to be used by objects.");
                if (selected != prevSelected) {
                    textureNames = { INIT_TEXTURE };
                    getTextures(textureNames);
                    strcpy(textureName[textureIndex], textureNames[selected].c_str());
                    vkRenderer->swapTexture(textureName[textureIndex], textureIndex);
                    prevSelected = selected;
                }
            }

            if (ImGui::CollapsingHeader("Sky Box")) {

                std::vector<std::string> worldSkyBoxTypes = { INIT_SKYBOX };
                getSkyBoxes(worldSkyBoxTypes);

                static int selected = 0;
                static int prevSelected = 0;

                ImGui::ListBox("Sky Box Type", &selected, [](void* data, int idx, const char** out_text) {
                    auto& worlds = *static_cast<std::vector<std::string>*>(data);
                    worlds[idx] = worlds[idx].substr(worlds[idx].find_first_of('\\') + 1, worlds[idx].find_last_of('.') - (worlds[idx].find_first_of('\\') + 1));
                    *out_text = worlds[idx].c_str();
                    return true;
                    }, &worldSkyBoxTypes, worldSkyBoxTypes.size(), 3);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select a skybox to use in the world. This will be the background of the scene.");
                if (selected != prevSelected) {
                    worldSkyBoxTypes = { INIT_SKYBOX };
                    getSkyBoxes(worldSkyBoxTypes);
                    strcpy(textureName[0], worldSkyBoxTypes[selected].c_str());
                    vkRenderer->swapTexture(textureName[0], 0);
                    prevSelected = selected;
                }
            }

            if (ImGui::CollapsingHeader("Camera Data")) {

                ImGui::DragFloat3("Camera Pos", &saveData->camData.camera_pos[0], 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The position of the camera in the world");
                createPlayPopup3F("camData camera_pos", (float*)(&saveData->camData.camera_pos), "Camera Pos");
                ImGui::DragFloat3("Camera Rot", &saveData->camData.camera_rot[0], 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The rotation of the camera in the world");
                createPlayPopup3F("camData camera_rot", (float*)(&saveData->camData.camera_rot), "Camera Rot");
                ImGui::DragFloat3("Light Pos", &saveData->camData.light_pos[0], 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The position of the light in the world");
                createPlayPopup3F("camData light_pos", (float*)(&saveData->camData.light_pos), "Light Pos");
                ImGui::DragInt("Number of Steps", &saveData->camData.num_steps, 1, 1, 1000);    
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The max number of steps a ray will take");
                createPlayPopup1I("camData num_steps", &saveData->camData.num_steps, "Number of Steps");
                ImGui::DragFloat("Min Step", &saveData->camData.min_step, 0.00001f, 0.00001f, .1f, "%.5f");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The distance from a surface that is considered a hit");
                createPlayPopup1F("camData min_step", &saveData->camData.min_step, "Min Step");
                ImGui::DragFloat("Max Dist", &saveData->camData.max_dist, 10.0f, 0.0f, 2000.0f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The max distance a ray will travel");
                createPlayPopup1F("camData max_dist", &saveData->camData.max_dist, "Max Dist");
                ImGui::SliderInt("Ray Bounces/Refractions", &saveData->camData.ray_depth, 0, 5);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The max number of bounces/refractions a ray will take");
                createPlayPopup1I("camData ray_depth", &saveData->camData.ray_depth, "Ray Bounces/Refractions");
                ImGui::DragFloat("Time", &saveData->camData.time, 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The time of the world");
                createPlayPopup1F("camData time", &saveData->camData.time, "Time");
                ImGui::DragFloat("Time Multiplier", &saveData->camData.data4.w, 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("A multiplier for the time");
                createPlayPopup1F("camData data4 w", &saveData->camData.data4.w, "Time Multiplier");
                ImGui::DragFloat("Normal Offset", &saveData->camData.data4.z, 0.00001f, 0.0f, 1.0f, "%.5f");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("A offset used for calculating the reflection angle. Higher values result in smoother angles");
                createPlayPopup1F("camData data4 z", &saveData->camData.data4.z, "Normal Angle");
                ImGui::DragFloat("Player Speed", &saveData->camData.data4.y, 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The speed of the player");
                createPlayPopup1F("camData data4 y", &saveData->camData.data4.y, "Player Speed");
                ImGui::DragFloat("Field of View", &saveData->camData.data4.x, 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The vertical field of view of the camera");
                createPlayPopup1F("camData data4 x", &saveData->camData.data4.x, "Field of View");
                ImGui::DragFloat("Fog Density", &saveData->camData.data3.w, 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The density of the fog. Fog starts at 'Max Dist'");
                createPlayPopup1F("camData data3 w", &saveData->camData.data3.w, "Fog Density");
                ImGui::DragFloat3("Normal Direction", &saveData->camData.data1.x, 0.1f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The angle to base the normal color off");
                createPlayPopup3F("camData data1", (float*)(&saveData->camData.data1.x), "Normal Direction");
                ImGui::DragFloat("Bump Map Min Dist", &saveData->camData.data3.z, 0.01f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The min distance to use the bump map");
                createPlayPopup1F("camData data3 z", &saveData->camData.data3.z, "Bump Map Min Dist");
                ImGui::DragFloat("Bump Map Height", &saveData->camData.data3.y, 0.01f);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("The height of the bump map");
                createPlayPopup1F("camData data3 y", &saveData->camData.data3.y, "Bump Map Height");
            }

            for (int i = 0; i <= MAX_OBJECTS; i++) {
                for (int j = 0; j <= MAX_OBJECTS; j++) {
                    for (int k = 0; k <= MAX_OBJECTS; k++) {
                        for (int l = 0; l <= MAX_OBJECTS; l++) {
                            alreadyOpen[i][j][k][l][0] = false;
                            alreadyOpen[i][j][k][l][1] = false;
                        }
                    }
                }
            }
            int keys[5] = { 0, MAX_OBJECTS, MAX_OBJECTS, MAX_OBJECTS, 0 };
            ImGui::SliderInt("Number of Structures", &saveData->worldData.num_indices, 0, MAX_OBJECTS);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("The number of unique structures in the world");
            for (int i = 0; i < saveData->worldData.num_indices; i++) {
                drawIndex(i, 0, keys);
            }
            ImGui::EndTabItem();
        }

        // Second tab
        if (ImGui::BeginTabItem("Help"))
        {
            createHelpPage();
            ImGui::EndTabItem();
        }

        // End the tab bar
        ImGui::EndTabBar();
    }

    ImGui::PopItemWidth();

    ImGui::End();

}

void imguiWindow::createHelpPage() {
    ImGui::TextWrapped("This is the help page.");
    ImGui::TextWrapped("Learn about the different sections of the builder.");
    ImGui::TextWrapped("Hover over something in the builder to see what its for.");
    ImGui::TextWrapped("I suggest playing around with the saved worlds.");
    ImGui::TextWrapped("Try loading one and messing around with the settings.");
    if (ImGui::CollapsingHeader("How the Graphics Work")) {
		ImGui::TextWrapped("The graphics are ray traced/marched.");
		ImGui::TextWrapped("This means that light rays are cast from the camera for each pixel to bounce around the scene.");
        ImGui::TextWrapped("The world is made up of objects in which the distance to the surface is calculated.");
        ImGui::TextWrapped("The distance is then used to move the ray closer to the surface.");
        ImGui::TextWrapped("This repeats until the ray hits the surface.");
		ImGui::TextWrapped("The color of the surface is then calculated based on what the ray hits");
    }

    if (ImGui::CollapsingHeader("How to Build a World")) {
        ImGui::TextWrapped("Worlds are built of structures");
        ImGui::TextWrapped("Stuctures are any unique tower of substructures");
        ImGui::TextWrapped("Each substructure acts on all the substructures below it");
        ImGui::TextWrapped("Substructures are objects and modifiers");
        ImGui::TextWrapped("Objects are the things that are actually drawn, different 3D shapes");
        ImGui::TextWrapped("Modifiers are broken down into 2 types:");
        ImGui::TextWrapped("  Combine Modifiers - Combine a substructure with an object");
        ImGui::TextWrapped("  Domain Modifiers - Modify the space of the world");
        
    }

    if (ImGui::CollapsingHeader("How to Make Animations")){
		ImGui::TextWrapped("Most values can be animated");
        ImGui::TextWrapped("To animate a value, right click on the value");
        ImGui::TextWrapped("Set the min and max values to animate between");
        ImGui::TextWrapped("Set the amount it changes every step");
        ImGui::TextWrapped("Set the time between each step");
        ImGui::TextWrapped("Check if it is increasing or not");
        ImGui::TextWrapped("Finally pick a type of animation:");
        ImGui::TextWrapped("  Loop - Wrap around when value is out of range");
        ImGui::TextWrapped("  Bounce - Reverse direction");
	}

    if (ImGui::CollapsingHeader("Controls/Playing")) {
        ImGui::Button("Test/Enter World");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("This Button allows you to enter the world and move around");
        ImGui::SameLine();
        ImGui::TextWrapped("Press 'Esc' to reenable the UI");
        ImGui::TextWrapped("Here are the controls:");
        ImGui::TextWrapped("WASD - Move");
        ImGui::TextWrapped("Space - Move Up");
        ImGui::TextWrapped("Ctrl - Move Down");
        ImGui::TextWrapped("Mouse - Look Around");
        ImGui::TextWrapped("Shift - Move Faster");
        ImGui::TextWrapped("Esc - Reenable UI");
    }

    if (ImGui::CollapsingHeader("Camera Data")) {
        ImGui::TextWrapped("Camera Data handles settings related to the camera.");
        ImGui::TextWrapped("Changes made here happen to the whole screen.");
    }


}

void imguiWindow::getSavedWorlds(std::vector<std::string>& savedWorlds) {
    for (const auto& entry : std::filesystem::directory_iterator(worldSavesPath)) {
        // In entry.path() the strings look like this: "worlds\\v0.0.1-EmptyWorld.wrld"
        // I need only the "EmptyWorld" part added to the savedWorlds vector
        std::string path = entry.path().string();
        if(path.find(version) == std::string::npos) continue; // If the version is not found in the path, skip it
        std::string worldName = path.substr(path.find_first_of("-") + 1, path.find_last_of(".") - path.find_first_of("-") - 1);
        char tempFileName[128];
        snprintf(tempFileName, sizeof(tempFileName), "%s", worldName.c_str());
        if (!fileExists(tempFileName, fileExtension)) continue; // If the file is not found, skip it
        savedWorlds.push_back(worldName);
    }
}

void imguiWindow::getSkyBoxes(std::vector<std::string>& skyBoxes) {
	for (const auto& entry : std::filesystem::directory_iterator(skyBoxPath)) {
		std::string path = entry.path().string();
        if (path.compare(INIT_SKYBOX) == 0) continue; // Skip the initial skybox
		skyBoxes.push_back(path);
	}
}

void imguiWindow::getTextures(std::vector<std::string>& textures) {
	for (const auto& entry : std::filesystem::directory_iterator(texturePath)) {
		std::string path = entry.path().string();
        if (path.compare(INIT_TEXTURE) == 0) continue; // Skip the initial texture
		textures.push_back(path);
	}
}

void imguiWindow::saveWorldData() {
    char fullPath[256]; // Buffer to hold the full path
    snprintf(fullPath, sizeof(fullPath), "%s%s%s%s", worldSavesPath, version, fileName, fileExtension);
    try {
        FILE* fout = fopen(fullPath, "w");

        // Save saveData->camData and saveData->worldData
        fwrite(saveData, sizeof(*saveData), 1, fout);

        // Save animation data (size first, then the map data)
        size_t animationCount = animations->size();
        fwrite(&animationCount, sizeof(animationCount), 1, fout);

        for (const auto& [key, animationData] : *animations) {
            fwrite(&key, sizeof(key), 1, fout);
            fwrite(&animationData, sizeof(animationData), 1, fout);
        }

        fwrite(textureName[0], sizeof(char), 128, fout);

        fclose(fout);
        std::cout << "Saved World Data with animations" << std::endl;
        std::cout << "File Path: " << fullPath << std::endl;
    }
    catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void imguiWindow::saveWorldDataJson() {
    json j;
    dataToJson(&j);
    //std::cout << j.dump(4) << std::endl;
    char fullPath[256]; // Buffer to hold the full path
    snprintf(fullPath, sizeof(fullPath), "%s%s%s%s", worldSavesPath, version, fileName, ".json");
    try {
        std::fstream fout;
        fout.open(fullPath, std::ios::out);

        fout << j.dump(4);

        std::cout << "Saved World Data as json" << std::endl;
        std::cout << "File Path: " << fullPath << std::endl;
    }
    catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void imguiWindow::dataToJson(json *j) {
    j->clear();

    (*j)["MAX_OBJECTS"] = MAX_OBJECTS;
    (*j)["MAX_IMAGES"] = MAX_IMAGES;

    (*j)["camData"]["camera_pos"] = { saveData->camData.camera_pos[0], saveData->camData.camera_pos[1], saveData->camData.camera_pos[2] };
	(*j)["camData"]["camera_rot"] = { saveData->camData.camera_rot[0], saveData->camData.camera_rot[1], saveData->camData.camera_rot[2] };
	(*j)["camData"]["light_pos"] = { saveData->camData.light_pos[0], saveData->camData.light_pos[1], saveData->camData.light_pos[2] };
	(*j)["camData"]["data1"] = { saveData->camData.data1.x, saveData->camData.data1.y, saveData->camData.data1.z, saveData->camData.data1.w };
    (*j)["camData"]["data2"] = { saveData->camData.data2.x, saveData->camData.data2.y, saveData->camData.data2.z, saveData->camData.data2.w };
	(*j)["camData"]["data3"] = { saveData->camData.data3.x, saveData->camData.data3.y, saveData->camData.data3.z, saveData->camData.data3.w };
	(*j)["camData"]["data4"] = { saveData->camData.data4.x, saveData->camData.data4.y, saveData->camData.data4.z, saveData->camData.data4.w };
    (*j)["camData"]["resolution"] = { saveData->camData.resolution.x, saveData->camData.resolution.y };
    (*j)["camData"]["int1"] = saveData->camData.int1;
	(*j)["camData"]["int2"] = saveData->camData.int2;
	(*j)["camData"]["int3"] = saveData->camData.int3;
	(*j)["camData"]["int4"] = saveData->camData.int4;
	(*j)["camData"]["int5"] = saveData->camData.int5;
	(*j)["camData"]["int6"] = saveData->camData.int6;
	(*j)["camData"]["int7"] = saveData->camData.int7;
	(*j)["camData"]["int8"] = saveData->camData.int8;
	(*j)["camData"]["time"] = saveData->camData.time;
	(*j)["camData"]["num_steps"] = saveData->camData.num_steps;
	(*j)["camData"]["min_step"] = saveData->camData.min_step;
	(*j)["camData"]["max_dist"] = saveData->camData.max_dist;
	(*j)["camData"]["ray_depth"] = saveData->camData.ray_depth;

    (*j)["worldData"]["num_objects"] = saveData->worldData.num_objects;
	for (int i = 0; i < MAX_OBJECTS; i++) {
		(*j)["worldData"]["objects"][i]["center"] = { saveData->worldData.objects[i].center.x, saveData->worldData.objects[i].center.y, saveData->worldData.objects[i].center.z };
		(*j)["worldData"]["objects"][i]["size"] = { saveData->worldData.objects[i].size.x, saveData->worldData.objects[i].size.y, saveData->worldData.objects[i].size.z };
		(*j)["worldData"]["objects"][i]["color"] = { saveData->worldData.objects[i].color.x, saveData->worldData.objects[i].color.y, saveData->worldData.objects[i].color.z };
		(*j)["worldData"]["objects"][i]["data1"] = { saveData->worldData.objects[i].data1.x, saveData->worldData.objects[i].data1.y, saveData->worldData.objects[i].data1.z, saveData->worldData.objects[i].data1.w };
		(*j)["worldData"]["objects"][i]["data2"] = { saveData->worldData.objects[i].data2.x, saveData->worldData.objects[i].data2.y, saveData->worldData.objects[i].data2.z, saveData->worldData.objects[i].data2.w };
		(*j)["worldData"]["objects"][i]["data3"] = { saveData->worldData.objects[i].data3.x, saveData->worldData.objects[i].data3.y, saveData->worldData.objects[i].data3.z, saveData->worldData.objects[i].data3.w };
		(*j)["worldData"]["objects"][i]["data4"] = { saveData->worldData.objects[i].data4.x, saveData->worldData.objects[i].data4.y, saveData->worldData.objects[i].data4.z, saveData->worldData.objects[i].data4.w };
		(*j)["worldData"]["objects"][i]["textureIndex"] = saveData->worldData.objects[i].textureIndex;
		(*j)["worldData"]["objects"][i]["int2"] = saveData->worldData.objects[i].int2;
		(*j)["worldData"]["objects"][i]["int3"] = saveData->worldData.objects[i].int3;
		(*j)["worldData"]["objects"][i]["int4"] = saveData->worldData.objects[i].int4;
		(*j)["worldData"]["objects"][i]["int5"] = saveData->worldData.objects[i].int5;
		(*j)["worldData"]["objects"][i]["int6"] = saveData->worldData.objects[i].int6;
		(*j)["worldData"]["objects"][i]["int7"] = saveData->worldData.objects[i].int7;
		(*j)["worldData"]["objects"][i]["int8"] = saveData->worldData.objects[i].int8;
		(*j)["worldData"]["objects"][i]["type"] = saveData->worldData.objects[i].type;
		(*j)["worldData"]["objects"][i]["is_negated"] = saveData->worldData.objects[i].is_negated;
		(*j)["worldData"]["objects"][i]["shadow_blur"] = saveData->worldData.objects[i].shadow_blur;
		(*j)["worldData"]["objects"][i]["shadow_intensity"] = saveData->worldData.objects[i].shadow_intensity;
		(*j)["worldData"]["objects"][i]["reflectivity"] = saveData->worldData.objects[i].reflectivity;
		(*j)["worldData"]["objects"][i]["transparency"] = saveData->worldData.objects[i].transparency;
		(*j)["worldData"]["objects"][i]["diffuse_intensity"] = saveData->worldData.objects[i].diffuse_intensity;
		(*j)["worldData"]["objects"][i]["refractive_index"] = saveData->worldData.objects[i].refractive_index;
	}

    (*j)["worldData"]["num_combine_modifiers"] = saveData->worldData.num_combine_modifiers;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        (*j)["worldData"]["combineModifiers"][i]["data1"] = { saveData->worldData.combineModifiers[i].data1.x, saveData->worldData.combineModifiers[i].data1.y, saveData->worldData.combineModifiers[i].data1.z, saveData->worldData.combineModifiers[i].data1.w };
		(*j)["worldData"]["combineModifiers"][i]["data2"] = { saveData->worldData.combineModifiers[i].data2.x, saveData->worldData.combineModifiers[i].data2.y, saveData->worldData.combineModifiers[i].data2.z, saveData->worldData.combineModifiers[i].data2.w };
		(*j)["worldData"]["combineModifiers"][i]["data3"] = { saveData->worldData.combineModifiers[i].data3.x, saveData->worldData.combineModifiers[i].data3.y, saveData->worldData.combineModifiers[i].data3.z, saveData->worldData.combineModifiers[i].data3.w };
		(*j)["worldData"]["combineModifiers"][i]["data4"] = { saveData->worldData.combineModifiers[i].data4.x, saveData->worldData.combineModifiers[i].data4.y, saveData->worldData.combineModifiers[i].data4.z, saveData->worldData.combineModifiers[i].data4.w };
		(*j)["worldData"]["combineModifiers"][i]["int1"] = saveData->worldData.combineModifiers[i].int1;
		(*j)["worldData"]["combineModifiers"][i]["int2"] = saveData->worldData.combineModifiers[i].int2;
		(*j)["worldData"]["combineModifiers"][i]["int3"] = saveData->worldData.combineModifiers[i].int3;
		(*j)["worldData"]["combineModifiers"][i]["int4"] = saveData->worldData.combineModifiers[i].int4;
		(*j)["worldData"]["combineModifiers"][i]["int5"] = saveData->worldData.combineModifiers[i].int5;
		(*j)["worldData"]["combineModifiers"][i]["int6"] = saveData->worldData.combineModifiers[i].int6;
		(*j)["worldData"]["combineModifiers"][i]["int7"] = saveData->worldData.combineModifiers[i].int7;
		(*j)["worldData"]["combineModifiers"][i]["int8"] = saveData->worldData.combineModifiers[i].int8;
		(*j)["worldData"]["combineModifiers"][i]["index1"] = saveData->worldData.combineModifiers[i].index1;
        (*j)["worldData"]["combineModifiers"][i]["index2"] = saveData->worldData.combineModifiers[i].index2;
        (*j)["worldData"]["combineModifiers"][i]["type"] = saveData->worldData.combineModifiers[i].type;
        (*j)["worldData"]["combineModifiers"][i]["index1Type"] = saveData->worldData.combineModifiers[i].index1Type;
    }

	(*j)["worldData"]["num_domain_modifiers"] = saveData->worldData.num_domain_modifiers;
    for(int i = 0; i < MAX_OBJECTS; i++) {
		(*j)["worldData"]["domainModifiers"][i]["data1"] = { saveData->worldData.domainModifiers[i].data1.x, saveData->worldData.domainModifiers[i].data1.y, saveData->worldData.domainModifiers[i].data1.z, saveData->worldData.domainModifiers[i].data1.w };
        (*j)["worldData"]["domainModifiers"][i]["data2"] = { saveData->worldData.domainModifiers[i].data2.x, saveData->worldData.domainModifiers[i].data2.y, saveData->worldData.domainModifiers[i].data2.z, saveData->worldData.domainModifiers[i].data2.w };
		(*j)["worldData"]["domainModifiers"][i]["data3"] = { saveData->worldData.domainModifiers[i].data3.x, saveData->worldData.domainModifiers[i].data3.y, saveData->worldData.domainModifiers[i].data3.z, saveData->worldData.domainModifiers[i].data3.w };
		(*j)["worldData"]["domainModifiers"][i]["data4"] = { saveData->worldData.domainModifiers[i].data4.x, saveData->worldData.domainModifiers[i].data4.y, saveData->worldData.domainModifiers[i].data4.z, saveData->worldData.domainModifiers[i].data4.w };
		(*j)["worldData"]["domainModifiers"][i]["int1"] = saveData->worldData.domainModifiers[i].int1;
		(*j)["worldData"]["domainModifiers"][i]["int2"] = saveData->worldData.domainModifiers[i].int2;
		(*j)["worldData"]["domainModifiers"][i]["int3"] = saveData->worldData.domainModifiers[i].int3;
		(*j)["worldData"]["domainModifiers"][i]["int4"] = saveData->worldData.domainModifiers[i].int4;
		(*j)["worldData"]["domainModifiers"][i]["int5"] = saveData->worldData.domainModifiers[i].int5;
		(*j)["worldData"]["domainModifiers"][i]["int6"] = saveData->worldData.domainModifiers[i].int6;
		(*j)["worldData"]["domainModifiers"][i]["int7"] = saveData->worldData.domainModifiers[i].int7;
		(*j)["worldData"]["domainModifiers"][i]["int8"] = saveData->worldData.domainModifiers[i].int8;
		(*j)["worldData"]["domainModifiers"][i]["index1"] = saveData->worldData.domainModifiers[i].index1;
		(*j)["worldData"]["domainModifiers"][i]["type"] = saveData->worldData.domainModifiers[i].type;
        (*j)["worldData"]["domainModifiers"][i]["index1Type"] = saveData->worldData.domainModifiers[i].index1Type;
	}

	(*j)["worldData"]["num_indices"] = saveData->worldData.num_indices;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        (*j)["worldData"]["indices"][i]["data1"] = { saveData->worldData.indices[i].data1.x, saveData->worldData.indices[i].data1.y, saveData->worldData.indices[i].data1.z, saveData->worldData.indices[i].data1.w };
        (*j)["worldData"]["indices"][i]["data2"] = { saveData->worldData.indices[i].data2.x, saveData->worldData.indices[i].data2.y, saveData->worldData.indices[i].data2.z, saveData->worldData.indices[i].data2.w };
        (*j)["worldData"]["indices"][i]["data3"] = { saveData->worldData.indices[i].data3.x, saveData->worldData.indices[i].data3.y, saveData->worldData.indices[i].data3.z, saveData->worldData.indices[i].data3.w };
        (*j)["worldData"]["indices"][i]["data4"] = { saveData->worldData.indices[i].data4.x, saveData->worldData.indices[i].data4.y, saveData->worldData.indices[i].data4.z, saveData->worldData.indices[i].data4.w };
        (*j)["worldData"]["indices"][i]["int1"] = saveData->worldData.indices[i].int1;
        (*j)["worldData"]["indices"][i]["int2"] = saveData->worldData.indices[i].int2;
        (*j)["worldData"]["indices"][i]["int3"] = saveData->worldData.indices[i].int3;
        (*j)["worldData"]["indices"][i]["int4"] = saveData->worldData.indices[i].int4;
        (*j)["worldData"]["indices"][i]["int5"] = saveData->worldData.indices[i].int5;
        (*j)["worldData"]["indices"][i]["int6"] = saveData->worldData.indices[i].int6;
        (*j)["worldData"]["indices"][i]["int7"] = saveData->worldData.indices[i].int7;
        (*j)["worldData"]["indices"][i]["int8"] = saveData->worldData.indices[i].int8;
        (*j)["worldData"]["indices"][i]["index"] = saveData->worldData.indices[i].index;
        (*j)["worldData"]["indices"][i]["type"] = saveData->worldData.indices[i].type;
    }
    (*j)["animations"] = json::object();
    for (const auto& [key, animationData] : *animations) {
        (*j)["animations"][key]["offset"] = animationData.offset;
        (*j)["animations"][key]["curF"] = animationData.curF;
		(*j)["animations"][key]["minF"] = animationData.minF;
		(*j)["animations"][key]["maxF"] = animationData.maxF;
		(*j)["animations"][key]["stepF"] = animationData.stepF;
		(*j)["animations"][key]["timePerStep"] = animationData.timePerStep;
        (*j)["animations"][key]["timeSinceLastStep"] = animationData.timeSinceLastStep;
        (*j)["animations"][key]["curI"] = animationData.curI;
		(*j)["animations"][key]["minI"] = animationData.minI;
		(*j)["animations"][key]["maxI"] = animationData.maxI;
		(*j)["animations"][key]["stepI"] = animationData.stepI;
		(*j)["animations"][key]["typeVal"] = animationData.typeVal;
		(*j)["animations"][key]["typeAnimation"] = animationData.typeAnimation;
        (*j)["animations"][key]["increasing"] = animationData.increasing;
	}
	(*j)["textures"] = json::array();
    for (int i = 0; i < MAX_IMAGES; i++) {
        (*j)["textures"].push_back(textureName[i]);
	}
}

void printsize_tAndData(std::string* key, AnimationData* animationData) {
	std::cout << "Key: " << *key << std::endl;
	std::cout << "DataF: " << animationData->minF << " " << animationData->maxF << " " << animationData->stepF << " " << animationData->timePerStep << std::endl;
    std::cout << "DataI: " << animationData->minI << " " << animationData->maxI << " " << animationData->stepI << " " << animationData->timePerStep << std::endl;
    std::cout << "DataType: " << animationData->typeVal << " " << animationData->typeAnimation << std::endl;
}

void imguiWindow::loadWorldDataJson(bool isStartUp) {
    char fullPath[256]; // Buffer to hold the full path
    snprintf(fullPath, sizeof(fullPath), "%s%s%s%s", worldSavesPath, version, fileName, ".json");
    try {
        std::ifstream f(fullPath);
        json data = json::parse(f);

        jsonToData(&data);

        std::cout << "File Path: " << fullPath << std::endl;
    }
    catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void imguiWindow::jsonToData(json* j) {
    int max_objects = std::min((*j).value("MAX_OBJECTS", 0), MAX_OBJECTS);
    int max_images = std::min((*j).value("MAX_IMAGES", 0), MAX_IMAGES);

    saveData->camData.camera_pos[0] = (*j)["camData"]["camera_pos"][0];
	saveData->camData.camera_pos[1] = (*j)["camData"]["camera_pos"][1];
	saveData->camData.camera_pos[2] = (*j)["camData"]["camera_pos"][2];
	saveData->camData.camera_rot[0] = (*j)["camData"]["camera_rot"][0];
	saveData->camData.camera_rot[1] = (*j)["camData"]["camera_rot"][1];
	saveData->camData.camera_rot[2] = (*j)["camData"]["camera_rot"][2];
	saveData->camData.light_pos[0] = (*j)["camData"]["light_pos"][0];
	saveData->camData.light_pos[1] = (*j)["camData"]["light_pos"][1];
	saveData->camData.light_pos[2] = (*j)["camData"]["light_pos"][2];
	saveData->camData.data1.x = (*j)["camData"]["data1"][0];
	saveData->camData.data1.y = (*j)["camData"]["data1"][1];
	saveData->camData.data1.z = (*j)["camData"]["data1"][2];
	saveData->camData.data1.w = (*j)["camData"]["data1"][3];
	saveData->camData.data2.x = (*j)["camData"]["data2"][0];
	saveData->camData.data2.y = (*j)["camData"]["data2"][1];
	saveData->camData.data2.z = (*j)["camData"]["data2"][2];
	saveData->camData.data2.w = (*j)["camData"]["data2"][3];
	saveData->camData.data3.x = (*j)["camData"]["data3"][0];
	saveData->camData.data3.y = (*j)["camData"]["data3"][1];
	saveData->camData.data3.z = (*j)["camData"]["data3"][2];
	saveData->camData.data3.w = (*j)["camData"]["data3"][3];
	saveData->camData.data4.x = (*j)["camData"]["data4"][0];
    saveData->camData.data4.y = (*j)["camData"]["data4"][1];
    saveData->camData.data4.z = (*j)["camData"]["data4"][2];
    saveData->camData.data4.w = (*j)["camData"]["data4"][3];
    saveData->camData.resolution.x = (*j)["camData"]["resolution"][0];
    saveData->camData.resolution.y = (*j)["camData"]["resolution"][1];
    saveData->camData.int1 = (*j)["camData"]["int1"];
    saveData->camData.int2 = (*j)["camData"]["int2"];
    saveData->camData.int3 = (*j)["camData"]["int3"];
    saveData->camData.int4 = (*j)["camData"]["int4"];
    saveData->camData.int5 = (*j)["camData"]["int5"];
    saveData->camData.int6 = (*j)["camData"]["int6"];
    saveData->camData.int7 = (*j)["camData"]["int7"];
    saveData->camData.int8 = (*j)["camData"]["int8"];
    saveData->camData.time = (*j)["camData"]["time"];
    saveData->camData.num_steps = (*j)["camData"]["num_steps"];
    saveData->camData.min_step = (*j)["camData"]["min_step"];
    saveData->camData.max_dist = (*j)["camData"]["max_dist"];
    saveData->camData.ray_depth = (*j)["camData"]["ray_depth"];

    saveData->worldData.num_objects = (*j)["worldData"]["num_objects"];
    for (int i = 0; i < max_objects; i++) {
        saveData->worldData.objects[i].center.x = (*j)["worldData"]["objects"][i]["center"][0];
        saveData->worldData.objects[i].center.y = (*j)["worldData"]["objects"][i]["center"][1];
        saveData->worldData.objects[i].center.z = (*j)["worldData"]["objects"][i]["center"][2];
        saveData->worldData.objects[i].size.x = (*j)["worldData"]["objects"][i]["size"][0];
        saveData->worldData.objects[i].size.y = (*j)["worldData"]["objects"][i]["size"][1];
        saveData->worldData.objects[i].size.z = (*j)["worldData"]["objects"][i]["size"][2];
        saveData->worldData.objects[i].color.x = (*j)["worldData"]["objects"][i]["color"][0];
        saveData->worldData.objects[i].color.y = (*j)["worldData"]["objects"][i]["color"][1];
        saveData->worldData.objects[i].color.z = (*j)["worldData"]["objects"][i]["color"][2];
        saveData->worldData.objects[i].data1.x = (*j)["worldData"]["objects"][i]["data1"][0];
        saveData->worldData.objects[i].data1.y = (*j)["worldData"]["objects"][i]["data1"][1];
        saveData->worldData.objects[i].data1.z = (*j)["worldData"]["objects"][i]["data1"][2];
        saveData->worldData.objects[i].data1.w = (*j)["worldData"]["objects"][i]["data1"][3];
        saveData->worldData.objects[i].data2.x = (*j)["worldData"]["objects"][i]["data2"][0];
        saveData->worldData.objects[i].data2.y = (*j)["worldData"]["objects"][i]["data2"][1];
        saveData->worldData.objects[i].data2.z = (*j)["worldData"]["objects"][i]["data2"][2];
        saveData->worldData.objects[i].data2.w = (*j)["worldData"]["objects"][i]["data2"][3];
        saveData->worldData.objects[i].data3.x = (*j)["worldData"]["objects"][i]["data3"][0];
        saveData->worldData.objects[i].data3.y = (*j)["worldData"]["objects"][i]["data3"][1];
        saveData->worldData.objects[i].data3.z = (*j)["worldData"]["objects"][i]["data3"][2];
        saveData->worldData.objects[i].data3.w = (*j)["worldData"]["objects"][i]["data3"][3];
        saveData->worldData.objects[i].data4.x = (*j)["worldData"]["objects"][i]["data4"][0];
        saveData->worldData.objects[i].data4.y = (*j)["worldData"]["objects"][i]["data4"][1];
        saveData->worldData.objects[i].data4.z = (*j)["worldData"]["objects"][i]["data4"][2];
        saveData->worldData.objects[i].data4.w = (*j)["worldData"]["objects"][i]["data4"][3];
        saveData->worldData.objects[i].textureIndex = (*j)["worldData"]["objects"][i]["textureIndex"];
        saveData->worldData.objects[i].int2 = (*j)["worldData"]["objects"][i]["int2"];
        saveData->worldData.objects[i].int3 = (*j)["worldData"]["objects"][i]["int3"];
        saveData->worldData.objects[i].int4 = (*j)["worldData"]["objects"][i]["int4"];
        saveData->worldData.objects[i].int5 = (*j)["worldData"]["objects"][i]["int5"];
        saveData->worldData.objects[i].int6 = (*j)["worldData"]["objects"][i]["int6"];
        saveData->worldData.objects[i].int7 = (*j)["worldData"]["objects"][i]["int7"];
        saveData->worldData.objects[i].int8 = (*j)["worldData"]["objects"][i]["int8"];
        saveData->worldData.objects[i].type = (*j)["worldData"]["objects"][i]["type"];
        saveData->worldData.objects[i].is_negated = (*j)["worldData"]["objects"][i]["is_negated"];
        saveData->worldData.objects[i].shadow_blur = (*j)["worldData"]["objects"][i]["shadow_blur"];
        saveData->worldData.objects[i].shadow_intensity = (*j)["worldData"]["objects"][i]["shadow_intensity"];
        saveData->worldData.objects[i].reflectivity = (*j)["worldData"]["objects"][i]["reflectivity"];
        saveData->worldData.objects[i].transparency = (*j)["worldData"]["objects"][i]["transparency"];
        saveData->worldData.objects[i].diffuse_intensity = (*j)["worldData"]["objects"][i]["diffuse_intensity"];
        saveData->worldData.objects[i].refractive_index = (*j)["worldData"]["objects"][i]["refractive_index"];
    }

    saveData->worldData.num_combine_modifiers = (*j)["worldData"]["num_combine_modifiers"];
    for (int i = 0; i < max_objects; i++) {
        saveData->worldData.combineModifiers[i].data1.x = (*j)["worldData"]["combineModifiers"][i]["data1"][0];
		saveData->worldData.combineModifiers[i].data1.y = (*j)["worldData"]["combineModifiers"][i]["data1"][1];
		saveData->worldData.combineModifiers[i].data1.z = (*j)["worldData"]["combineModifiers"][i]["data1"][2];
		saveData->worldData.combineModifiers[i].data1.w = (*j)["worldData"]["combineModifiers"][i]["data1"][3];
		saveData->worldData.combineModifiers[i].data2.x = (*j)["worldData"]["combineModifiers"][i]["data2"][0];
		saveData->worldData.combineModifiers[i].data2.y = (*j)["worldData"]["combineModifiers"][i]["data2"][1];
		saveData->worldData.combineModifiers[i].data2.z = (*j)["worldData"]["combineModifiers"][i]["data2"][2];
		saveData->worldData.combineModifiers[i].data2.w = (*j)["worldData"]["combineModifiers"][i]["data2"][3];
		saveData->worldData.combineModifiers[i].data3.x = (*j)["worldData"]["combineModifiers"][i]["data3"][0];
		saveData->worldData.combineModifiers[i].data3.y = (*j)["worldData"]["combineModifiers"][i]["data3"][1];
		saveData->worldData.combineModifiers[i].data3.z = (*j)["worldData"]["combineModifiers"][i]["data3"][2];
		saveData->worldData.combineModifiers[i].data3.w = (*j)["worldData"]["combineModifiers"][i]["data3"][3];
		saveData->worldData.combineModifiers[i].data4.x = (*j)["worldData"]["combineModifiers"][i]["data4"][0];
		saveData->worldData.combineModifiers[i].data4.y = (*j)["worldData"]["combineModifiers"][i]["data4"][1];
		saveData->worldData.combineModifiers[i].data4.z = (*j)["worldData"]["combineModifiers"][i]["data4"][2];
		saveData->worldData.combineModifiers[i].data4.w = (*j)["worldData"]["combineModifiers"][i]["data4"][3];
		saveData->worldData.combineModifiers[i].int1 = (*j)["worldData"]["combineModifiers"][i]["int1"];
        saveData->worldData.combineModifiers[i].int2 = (*j)["worldData"]["combineModifiers"][i]["int2"];
        saveData->worldData.combineModifiers[i].int3 = (*j)["worldData"]["combineModifiers"][i]["int3"];
        saveData->worldData.combineModifiers[i].int4 = (*j)["worldData"]["combineModifiers"][i]["int4"];
        saveData->worldData.combineModifiers[i].int5 = (*j)["worldData"]["combineModifiers"][i]["int5"];
        saveData->worldData.combineModifiers[i].int6 = (*j)["worldData"]["combineModifiers"][i]["int6"];
        saveData->worldData.combineModifiers[i].int7 = (*j)["worldData"]["combineModifiers"][i]["int7"];
        saveData->worldData.combineModifiers[i].int8 = (*j)["worldData"]["combineModifiers"][i]["int8"];
        saveData->worldData.combineModifiers[i].index1 = (*j)["worldData"]["combineModifiers"][i]["index1"];
        saveData->worldData.combineModifiers[i].index2 = (*j)["worldData"]["combineModifiers"][i]["index2"];
        saveData->worldData.combineModifiers[i].type = (*j)["worldData"]["combineModifiers"][i]["type"];
        saveData->worldData.combineModifiers[i].index1Type = (*j)["worldData"]["combineModifiers"][i]["index1Type"];
    }

    saveData->worldData.num_domain_modifiers = (*j)["worldData"]["num_domain_modifiers"];
    for (int i = 0; i < max_objects; i++) {
        saveData->worldData.domainModifiers[i].data1.x = (*j)["worldData"]["domainModifiers"][i]["data1"][0];
        saveData->worldData.domainModifiers[i].data1.y = (*j)["worldData"]["domainModifiers"][i]["data1"][1];
        saveData->worldData.domainModifiers[i].data1.z = (*j)["worldData"]["domainModifiers"][i]["data1"][2];
        saveData->worldData.domainModifiers[i].data1.w = (*j)["worldData"]["domainModifiers"][i]["data1"][3];
        saveData->worldData.domainModifiers[i].data2.x = (*j)["worldData"]["domainModifiers"][i]["data2"][0];
        saveData->worldData.domainModifiers[i].data2.y = (*j)["worldData"]["domainModifiers"][i]["data2"][1];
        saveData->worldData.domainModifiers[i].data2.z = (*j)["worldData"]["domainModifiers"][i]["data2"][2];
        saveData->worldData.domainModifiers[i].data2.w = (*j)["worldData"]["domainModifiers"][i]["data2"][3];
        saveData->worldData.domainModifiers[i].data3.x = (*j)["worldData"]["domainModifiers"][i]["data3"][0];
        saveData->worldData.domainModifiers[i].data3.y = (*j)["worldData"]["domainModifiers"][i]["data3"][1];
        saveData->worldData.domainModifiers[i].data3.z = (*j)["worldData"]["domainModifiers"][i]["data3"][2];
        saveData->worldData.domainModifiers[i].data3.w = (*j)["worldData"]["domainModifiers"][i]["data3"][3];
        saveData->worldData.domainModifiers[i].data4.x = (*j)["worldData"]["domainModifiers"][i]["data4"][0];
        saveData->worldData.domainModifiers[i].data4.y = (*j)["worldData"]["domainModifiers"][i]["data4"][1];
        saveData->worldData.domainModifiers[i].data4.z = (*j)["worldData"]["domainModifiers"][i]["data4"][2];
        saveData->worldData.domainModifiers[i].data4.w = (*j)["worldData"]["domainModifiers"][i]["data4"][3];
        saveData->worldData.domainModifiers[i].int1 = (*j)["worldData"]["domainModifiers"][i]["int1"];
        saveData->worldData.domainModifiers[i].int2 = (*j)["worldData"]["domainModifiers"][i]["int2"];
        saveData->worldData.domainModifiers[i].int3 = (*j)["worldData"]["domainModifiers"][i]["int3"];
        saveData->worldData.domainModifiers[i].int4 = (*j)["worldData"]["domainModifiers"][i]["int4"];
        saveData->worldData.domainModifiers[i].int5 = (*j)["worldData"]["domainModifiers"][i]["int5"];
        saveData->worldData.domainModifiers[i].int6 = (*j)["worldData"]["domainModifiers"][i]["int6"];
        saveData->worldData.domainModifiers[i].int7 = (*j)["worldData"]["domainModifiers"][i]["int7"];
        saveData->worldData.domainModifiers[i].int8 = (*j)["worldData"]["domainModifiers"][i]["int8"];
        saveData->worldData.domainModifiers[i].index1 = (*j)["worldData"]["domainModifiers"][i]["index1"];
        saveData->worldData.domainModifiers[i].type = (*j)["worldData"]["domainModifiers"][i]["type"];
        saveData->worldData.domainModifiers[i].index1Type = (*j)["worldData"]["domainModifiers"][i]["index1Type"];
    }

    saveData->worldData.num_indices = (*j)["worldData"]["num_indices"];
    for (int i = 0; i < max_objects; i++) {
        saveData->worldData.indices[i].data1.x = (*j)["worldData"]["indices"][i]["data1"][0];
		saveData->worldData.indices[i].data1.y = (*j)["worldData"]["indices"][i]["data1"][1];
		saveData->worldData.indices[i].data1.z = (*j)["worldData"]["indices"][i]["data1"][2];
		saveData->worldData.indices[i].data1.w = (*j)["worldData"]["indices"][i]["data1"][3];
		saveData->worldData.indices[i].data2.x = (*j)["worldData"]["indices"][i]["data2"][0];
		saveData->worldData.indices[i].data2.y = (*j)["worldData"]["indices"][i]["data2"][1];
		saveData->worldData.indices[i].data2.z = (*j)["worldData"]["indices"][i]["data2"][2];
		saveData->worldData.indices[i].data2.w = (*j)["worldData"]["indices"][i]["data2"][3];
		saveData->worldData.indices[i].data3.x = (*j)["worldData"]["indices"][i]["data3"][0];
		saveData->worldData.indices[i].data3.y = (*j)["worldData"]["indices"][i]["data3"][1];
		saveData->worldData.indices[i].data3.z = (*j)["worldData"]["indices"][i]["data3"][2];
		saveData->worldData.indices[i].data3.w = (*j)["worldData"]["indices"][i]["data3"][3];
		saveData->worldData.indices[i].data4.x = (*j)["worldData"]["indices"][i]["data4"][0];
		saveData->worldData.indices[i].data4.y = (*j)["worldData"]["indices"][i]["data4"][1];
		saveData->worldData.indices[i].data4.z = (*j)["worldData"]["indices"][i]["data4"][2];
		saveData->worldData.indices[i].data4.w = (*j)["worldData"]["indices"][i]["data4"][3];
		saveData->worldData.indices[i].int1 = (*j)["worldData"]["indices"][i]["int1"];
		saveData->worldData.indices[i].int2 = (*j)["worldData"]["indices"][i]["int2"];
        saveData->worldData.indices[i].int3 = (*j)["worldData"]["indices"][i]["int3"];
        saveData->worldData.indices[i].int4 = (*j)["worldData"]["indices"][i]["int4"];
        saveData->worldData.indices[i].int5 = (*j)["worldData"]["indices"][i]["int5"];
        saveData->worldData.indices[i].int6 = (*j)["worldData"]["indices"][i]["int6"];
        saveData->worldData.indices[i].int7 = (*j)["worldData"]["indices"][i]["int7"];
        saveData->worldData.indices[i].int8 = (*j)["worldData"]["indices"][i]["int8"];
        saveData->worldData.indices[i].index = (*j)["worldData"]["indices"][i]["index"];
        saveData->worldData.indices[i].type = (*j)["worldData"]["indices"][i]["type"];
    }

    animations->clear();
    for (auto& element : (*j)["animations"].items()) {
        std::string key = element.key();
		AnimationData animationData;
        animationData.offset = mapAttributeForAnimation(key);
        if(animationData.offset == -1) animationData.offset = element.value()["offset"];
        animationData.curF = element.value()["curF"];
		animationData.minF = element.value()["minF"];
		animationData.maxF = element.value()["maxF"];
		animationData.stepF = element.value()["stepF"];
		animationData.timePerStep = element.value()["timePerStep"];
        animationData.timeSinceLastStep = element.value()["timeSinceLastStep"];
        animationData.curI = element.value()["curI"];
		animationData.minI = element.value()["minI"];
		animationData.maxI = element.value()["maxI"];
		animationData.stepI = element.value()["stepI"];
		animationData.typeVal = element.value()["typeVal"];
		animationData.typeAnimation = element.value()["typeAnimation"];
		animationData.increasing = element.value()["increasing"];
		animations->insert(std::make_pair(key, animationData));
	}
    std::cout << animations->size() << " animations loaded" << std::endl;

    std::vector<std::string> textureNames = { INIT_TEXTURE };
    getTextures(textureNames);
    

    for (int i = 0; i < max_images; i++) {
        if (textureExists((*j)["textures"][i]) && std::strcmp(textureName[i], (*j)["textures"][i].get<std::string>().c_str())) {
            strcpy(textureName[i], (*j)["textures"][i].get<std::string>().c_str());
            vkRenderer->swapTexture(textureName[i], i);
		}
	}
}

void imguiWindow::updateOldSaves_v0_1_2() {
    for (const auto& entry : std::filesystem::directory_iterator(worldSavesPath)) {
        std::string path = entry.path().string();

        // Ensure the file path contains the correct version identifier
        if (path.find("v0.1.2") == std::string::npos) continue; 

        // Extract the filename correctly
        size_t dashPos = path.find_first_of("-");
        size_t dotPos = path.find_last_of(".");
        
        // Make sure dash and dot positions are valid
        if (dashPos == std::string::npos || dotPos == std::string::npos || dotPos <= dashPos) {
            continue; // Skip invalid file names
        }
        
        // Extract the filename without the version part or extension
        std::string fName = path.substr(dashPos + 1, dotPos - dashPos - 1);

        // Construct the full path correctly
        char fullPath[256]; // Buffer to hold the full path
        snprintf(fullPath, sizeof(fullPath), "%s%s-%s%s", worldSavesPath, "v0.1.2", fName.c_str(), ".wrld");
        try{
            // Try to open the file
            FILE* fin = fopen(fullPath, "r");
            if (!fin) {
                std::cerr << "Failed to open file: " << fullPath << std::endl;
                continue; // Skip if the file could not be opened
            }

            CameraData_v0_1_2 camData;
            WorldObjectsData_v0_1_2 worldData;

            // Load saveData->camData and saveData->worldData
            fread(&camData, sizeof(CameraData_v0_1_2), 1, fin);
            fread(&worldData, sizeof(WorldObjectsData_v0_1_2), 1, fin);

            SaveData_v0_1_3 saveDataTemp;

            camDataCopy_v0_1_2_to_v0_1_3(&camData, &saveDataTemp.camData);
            worldDataCopy_v0_1_2_to_v0_1_3(&worldData, &saveDataTemp.worldData);

            fclose(fin);
            remove(path.c_str());

            snprintf(fullPath, sizeof(fullPath), "%s%s%s%s", worldSavesPath, "v0.1.3-", fName.c_str(), ".wrld");

            FILE* fout = fopen(fullPath, "w");

            // Save saveData->camData and saveData->worldData
            fwrite(&saveDataTemp, sizeof(saveDataTemp), 1, fout);

            // Save animation data (size first, then the map data)
            size_t animationCount = animations->size();
            fwrite(&animationCount, sizeof(animationCount), 1, fout);

            for (const auto& [key, animationData] : *animations) {
                fwrite(&key, sizeof(key), 1, fout);
                fwrite(&animationData, sizeof(animationData), 1, fout);
            }

            fclose(fout);
            std::cout << "Saved World Data without animations" << std::endl;
            std::cout << "File Path: " << fullPath << std::endl;

        }
        catch (std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

void imguiWindow::updateOldSaves_v0_1_3(bool isStartUp) {
    /**/
    for (const auto& entry : std::filesystem::directory_iterator(worldSavesPath)) {
        std::string path = entry.path().string();

        // Ensure the file path contains the correct version identifier
        if (path.find("v0.1.3") == std::string::npos) continue;

        // Extract the filename correctly
        size_t dashPos = path.find_first_of("-");
        size_t dotPos = path.find_last_of(".");

        // Make sure dash and dot positions are valid
        if (dashPos == std::string::npos || dotPos == std::string::npos || dotPos <= dashPos) {
            continue; // Skip invalid file names
        }

        // Extract the filename without the version part or extension
        std::string fName = path.substr(dashPos + 1, dotPos - dashPos - 1);

        char fullPath[256]; // Buffer to hold the full path
        snprintf(fullPath, sizeof(fullPath), "%s%s%s%s", worldSavesPath, "v0.1.3-", fName.c_str(), ".wrld");
        /**/

        SaveData_v0_1_3 saveDataTemp;

        FILE* fin;
        try {
            fin = fopen(fullPath, "r");

            // Load saveData
            fread(&saveDataTemp, sizeof(saveDataTemp), 1, fin);
            /**
            // Get current position to ensure proper pointer placement
            long currentPosition = ftell(fin);

            // Check if there's more data in the file (i.e., animations or string)
            fseek(fin, 0, SEEK_END);
            long fileSize = ftell(fin);
            fseek(fin, currentPosition, SEEK_SET);

            // Calculate expected size if there are no animations or string
            long expectedSizeWithoutAnimations = sizeof(*saveData);

            //std::unordered_map<size_t, AnimationData>* animations;

            // If the file is larger, it has animations or a string, otherwise clear animations
            if (fileSize > expectedSizeWithoutAnimations) {
                // First, check if there's enough data to even read the animation count
                if (fileSize - currentPosition >= sizeof(size_t)) {
                    // Load animation data
                    size_t animationCount;
                    fread(&animationCount, sizeof(animationCount), 1, fin);
                    std::cout << "Animation Count: " << animationCount << std::endl;

                    // Sanity check: If the animationCount is unreasonably large, stop
                    if (animationCount > 1000) {  // Arbitrary large number to catch corrupted data
                        throw std::runtime_error("Invalid animation count detected. Possibly corrupted file.");
                    }

                    animations->clear(); // Clear existing animations

                    // Ensure we have enough space to read all animation data
                    size_t requiredDataSize = animationCount * (sizeof(size_t) + sizeof(AnimationDataOld));
                    if (fileSize - currentPosition >= requiredDataSize) {
                        for (size_t i = 0; i < animationCount; ++i) {
                            size_t key;
                            AnimationDataOld animationData;
                            fread(&key, sizeof(key), 1, fin);
                            fread(&animationData, sizeof(animationData), 1, fin);
                            AnimationData animationDataNew;
                            animationDataNew.offset = key;
                            animationDataNew.curF = animationData.curF;
                            animationDataNew.minF = animationData.minF;
                            animationDataNew.maxF = animationData.maxF;
                            animationDataNew.stepF = animationData.stepF;
                            animationDataNew.timePerStep = animationData.timePerStep;
                            animationDataNew.timeSinceLastStep = animationData.timeSinceLastStep;
                            animationDataNew.curI = animationData.curI;
                            animationDataNew.minI = animationData.minI;
                            animationDataNew.maxI = animationData.maxI;
                            animationDataNew.stepI = animationData.stepI;
                            animationDataNew.typeVal = animationData.typeVal;
                            animationDataNew.typeAnimation = animationData.typeAnimation;
                            animationDataNew.increasing = animationData.increasing;
                            animations->insert(std::make_pair(mapAnimationForAttribute(key), animationDataNew));
                        }

                        std::cout << "Loaded World Data with animations" << std::endl;
                    }
                    else {
                        std::cout << "File Size: " << fileSize << " Current Position: " << currentPosition << " Required Data Size: " << requiredDataSize << " Difference: " << fileSize - currentPosition << std::endl;
                        throw std::runtime_error("Not enough data to read all animations. Possibly corrupted file.");
                    }

                    // Update the current position after reading animations
                    currentPosition = ftell(fin);
                }
            }
            else {
                // No animation data in the file, so clear any existing animations
                animations->clear();
                std::cout << "Loaded World Data without animations (old format)" << std::endl;
            }
            /**
            // Check if there's space for the 64-character string after animations
            if (fileSize - currentPosition >= 128) {
                char skybox[128]; // Buffer for 64 characters + null terminator
                fread(textureName[0], sizeof(char), 128, fin);
                textureName[0][127] = '\0'; // Ensure null termination
                std::cout << "Loaded SkyBox: " << skybox << std::endl;
                if (!isStartUp) vkRenderer->swapTexture(skybox);
            }
            else {
                std::cout << "No Skybox Found." << std::endl;
                if(!isStartUp) vkRenderer->swapTexture("skyboxes\\Black.png");
            }/**/

            fclose(fin);
            std::cout << "File Path: " << fullPath << std::endl;
        }
        catch (std::exception& e) {
            fclose(fin);
            std::cout << "Error: " << e.what() << std::endl;
        }

        camDataCopy_v0_1_3_to_v0_2_0(&saveDataTemp.camData, &saveData->camData);
        worldDataCopy_v0_1_3_to_v0_2_0(&saveDataTemp.worldData, &saveData->worldData);

        remove(path.c_str());

        saveData->camData.data3 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); //data3.y: bump map height, data3.z: bump map min dist, data3.w: fog density
        saveData->camData.data4 = glm::vec4(90.0f, 1.5f, 0.001f, 1.0f); //data4.x: FOV, data4.y: Player Speed, data4.z: Normal Offset, data4.w: timeMultiplier

        json j;
        dataToJson(&j);
        //std::cout << j.dump(4) << std::endl;
        char newPath[256]; // Buffer to hold the full path
        snprintf(newPath, sizeof(newPath), "%s%s%s%s", worldSavesPath, version, fName.c_str(), fileExtension);
        try {
            std::fstream fout;
            fout.open(newPath, std::ios::out);

            fout << j.dump(4);

            std::cout << "Saved World Data as json" << std::endl;
            std::cout << "File Path: " << newPath << std::endl;
        }
        catch (std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }

    }
    // Load empty world
    snprintf(fileName, sizeof(fileName), "%s", "EmptyWorld");
    loadWorldDataJson(true);
    snprintf(fileName, sizeof(fileName), "%s", "");
}

void imguiWindow::deleteWorld(){
    char fullPath[256]; // Buffer to hold the full path
	snprintf(fullPath, sizeof(fullPath), "%s%s%s%s", worldSavesPath, version, fileName, fileExtension);
	try {
		remove(fullPath);
		std::cout << "Deleted World Data" << std::endl;
		std::cout << "File Path: " << fullPath << std::endl;
	}
	catch (std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
	}

}

void imguiWindow::camDataCopy_v0_1_2_to_v0_1_3(CameraData_v0_1_2* camDataOld, CameraData_v0_1_3* camData) {

    int width = 0, height = 0;
    glfwGetFramebufferSize(vkRenderer->getWindow(), &width, &height);
    camData->camera_pos = camDataOld->u_camera_pos;
    camData->camera_rot = camDataOld->u_camera_rot;
    camData->light_pos = camDataOld->u_light_pos;
    camData->data1 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    camData->data2 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    camData->data3 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); //data3.y: bump map height, data3.z: bump map min dist, data3.w: fog density
    camData->data4 = glm::vec4(90.0f, 1.5f, 0.001f, 1.0f); //data4.x: FOV, data4.y: Player Speed, data4.z: Normal Offset, data4.w: timeMultiplier
    camData->resolution = glm::vec2(width, height);
    camData->int1 = 0;
    camData->int2 = 0;
    camData->int3 = 0;
    camData->int4 = 0;
    camData->int5 = 0;
    camData->int6 = 0;
    camData->int7 = 0;
    camData->int8 = 0;
    camData->time = camDataOld->u_time;
    camData->num_steps = camDataOld->u_num_steps;
    camData->min_step = camDataOld->u_min_step;
    camData->max_dist = camDataOld->u_max_dist;
    camData->ray_depth = 2;
}

void imguiWindow::worldDataCopy_v0_1_2_to_v0_1_3(WorldObjectsData_v0_1_2* worldDataOld, WorldObjectsData_v0_1_3* worldData) {
	worldData->num_indices = worldDataOld->num_indices;
	for (int i = 0; i < 32; i++) {
        worldData->indices[i].data1 = glm::vec4(0.0);
        worldData->indices[i].data2 = glm::vec4(0.0);
        worldData->indices[i].data3 = glm::vec4(0.0);
        worldData->indices[i].data4 = glm::vec4(0.0);
        worldData->indices[i].int1 = 0;
        worldData->indices[i].int2 = 0;
        worldData->indices[i].int3 = 0;
        worldData->indices[i].int4 = 0;
        worldData->indices[i].int5 = 0;
        worldData->indices[i].int6 = 0;
        worldData->indices[i].int7 = 0;
        worldData->indices[i].int8 = 0;
        worldData->indices[i].index = worldDataOld->indices[i].index;
		worldData->indices[i].type = worldDataOld->indices[i].type;
	}
    worldData->num_objects = worldDataOld->num_objects;
    for (int i = 0; i < 32; i++) {
        worldData->objects[i].center = worldDataOld->objects[i].center;
		worldData->objects[i].size = worldDataOld->objects[i].size;
		worldData->objects[i].color = worldDataOld->objects[i].color;
        worldData->objects[i].data1 = glm::vec4(0.0);
        worldData->objects[i].data2 = glm::vec4(0.0);
        worldData->objects[i].data3 = glm::vec4(0.0);
        worldData->objects[i].data4 = glm::vec4(0.0);
        worldData->objects[i].textureIndex = 0;
        worldData->objects[i].int2 = 0;
        worldData->objects[i].int3 = 0;
        worldData->objects[i].int4 = 0;
        worldData->objects[i].int5 = 0;
        worldData->objects[i].int6 = 0;
        worldData->objects[i].int7 = 0;
        worldData->objects[i].int8 = 0;
		worldData->objects[i].type = worldDataOld->objects[i].type;
        worldData->objects[i].is_negated = worldDataOld->objects[i].is_negated;
        worldData->objects[i].shadow_blur = -1.0f;
        worldData->objects[i].shadow_intensity = 0.0f;
        worldData->objects[i].reflectivity = worldDataOld->objects[i].reflectivity;
        worldData->objects[i].transparency = worldDataOld->objects[i].transparency;
        worldData->objects[i].diffuse_intensity = worldDataOld->objects[i].diffuse_intensity;
        worldData->objects[i].refractive_index = 1.0f;
	}
    worldData->num_combine_modifiers = worldDataOld->num_combine_modifiers;
    for (int i = 0; i < 32; i++) {
		worldData->combineModifiers[i].data1 = worldDataOld->combineModifiers[i].data1;
        worldData->combineModifiers[i].data2 = glm::vec4(0.0);
        worldData->combineModifiers[i].data3 = glm::vec4(0.0);
        worldData->combineModifiers[i].data4 = glm::vec4(0.0);
        worldData->combineModifiers[i].int1 = 0;
        worldData->combineModifiers[i].int2 = 0;
        worldData->combineModifiers[i].int3 = 0;
        worldData->combineModifiers[i].int4 = 0;
        worldData->combineModifiers[i].int5 = 0;
        worldData->combineModifiers[i].int6 = 0;
        worldData->combineModifiers[i].int7 = 0;
        worldData->combineModifiers[i].int8 = 0;
		worldData->combineModifiers[i].index1 = worldDataOld->combineModifiers[i].index1;
		worldData->combineModifiers[i].index2 = worldDataOld->combineModifiers[i].index2;
		worldData->combineModifiers[i].type = worldDataOld->combineModifiers[i].type;
		worldData->combineModifiers[i].index1Type = worldDataOld->combineModifiers[i].index1Type;
        }
	worldData->num_domain_modifiers = worldDataOld->num_domain_modifiers;
	for (int i = 0; i < 32; i++) {
        worldData->domainModifiers[i].data1 = worldDataOld->domainModifiers[i].data1;
        worldData->domainModifiers[i].data2 = worldDataOld->domainModifiers[i].data2;
        worldData->domainModifiers[i].data3 = glm::vec4(0.0);
        worldData->domainModifiers[i].data4 = glm::vec4(0.0);
        worldData->domainModifiers[i].int1 = 0;
        worldData->domainModifiers[i].int2 = 0;
        worldData->domainModifiers[i].int3 = 0;
        worldData->domainModifiers[i].int4 = 0;
        worldData->domainModifiers[i].int5 = 0;
        worldData->domainModifiers[i].int6 = 0;
        worldData->domainModifiers[i].int7 = 0;
        worldData->domainModifiers[i].int8 = 0;
		worldData->domainModifiers[i].index1 = worldDataOld->domainModifiers[i].index1;
		worldData->domainModifiers[i].type = worldDataOld->domainModifiers[i].type;
		worldData->domainModifiers[i].index1Type = worldDataOld->domainModifiers[i].index1Type;
	}
}

void imguiWindow::camDataCopy_v0_1_3_to_v0_2_0(CameraData_v0_1_3* camDataOld, CameraData* camData) {
	camData->camera_pos = camDataOld->camera_pos;
	camData->camera_rot = camDataOld->camera_rot;
	camData->light_pos = camDataOld->light_pos;
	camData->data1 = camDataOld->data1;
	camData->data2 = camDataOld->data2;
	camData->data3 = camDataOld->data3;
	camData->data4 = camDataOld->data4;
	camData->resolution = camDataOld->resolution;
	camData->int1 = camDataOld->int1;
	camData->int2 = camDataOld->int2;
	camData->int3 = camDataOld->int3;
	camData->int4 = camDataOld->int4;
	camData->int5 = camDataOld->int5;
	camData->int6 = camDataOld->int6;
	camData->int7 = camDataOld->int7;
	camData->int8 = camDataOld->int8;
	camData->time = camDataOld->time;
	camData->num_steps = camDataOld->num_steps;
	camData->min_step = camDataOld->min_step;
	camData->max_dist = camDataOld->max_dist;
	camData->ray_depth = camDataOld->ray_depth;
}

void imguiWindow::worldDataCopy_v0_1_3_to_v0_2_0(WorldObjectsData_v0_1_3* worldDataOld, WorldObjectsData* worldData) {
	worldData->num_indices = worldDataOld->num_indices;
	for (int i = 0; i < 32; i++) {
		worldData->indices[i].data1 = worldDataOld->indices[i].data1;
        worldData->indices[i].data2 = worldDataOld->indices[i].data2;
        worldData->indices[i].data3 = worldDataOld->indices[i].data3;
        worldData->indices[i].data4 = worldDataOld->indices[i].data4;
        worldData->indices[i].int1 = worldDataOld->indices[i].int1;
        worldData->indices[i].int2 = worldDataOld->indices[i].int2;
        worldData->indices[i].int3 = worldDataOld->indices[i].int3;
        worldData->indices[i].int4 = worldDataOld->indices[i].int4;
        worldData->indices[i].int5 = worldDataOld->indices[i].int5;
        worldData->indices[i].int6 = worldDataOld->indices[i].int6;
        worldData->indices[i].int7 = worldDataOld->indices[i].int7;
        worldData->indices[i].int8 = worldDataOld->indices[i].int8;
        worldData->indices[i].index = worldDataOld->indices[i].index;
        worldData->indices[i].type = worldDataOld->indices[i].type;
	}
	worldData->num_objects = worldDataOld->num_objects;
	for (int i = 0; i < 32; i++) {
		worldData->objects[i].center = worldDataOld->objects[i].center;
        worldData->objects[i].size = worldDataOld->objects[i].size;
        worldData->objects[i].color = worldDataOld->objects[i].color;
        worldData->objects[i].data1 = worldDataOld->objects[i].data1;
        worldData->objects[i].data2 = worldDataOld->objects[i].data2;
        worldData->objects[i].data3 = worldDataOld->objects[i].data3;
        worldData->objects[i].data4 = worldDataOld->objects[i].data4;
        worldData->objects[i].textureIndex = worldDataOld->objects[i].textureIndex;
        worldData->objects[i].int2 = worldDataOld->objects[i].int2;
        worldData->objects[i].int3 = worldDataOld->objects[i].int3;
        worldData->objects[i].int4 = worldDataOld->objects[i].int4;
        worldData->objects[i].int5 = worldDataOld->objects[i].int5;
        worldData->objects[i].int6 = worldDataOld->objects[i].int6;
        worldData->objects[i].int7 = worldDataOld->objects[i].int7;
        worldData->objects[i].int8 = worldDataOld->objects[i].int8;
        worldData->objects[i].type = worldDataOld->objects[i].type;
        worldData->objects[i].is_negated = worldDataOld->objects[i].is_negated;
        worldData->objects[i].shadow_blur = worldDataOld->objects[i].shadow_blur;
        worldData->objects[i].shadow_intensity = worldDataOld->objects[i].shadow_intensity;
        worldData->objects[i].reflectivity = worldDataOld->objects[i].reflectivity;
        worldData->objects[i].transparency = worldDataOld->objects[i].transparency;
        worldData->objects[i].diffuse_intensity = worldDataOld->objects[i].diffuse_intensity;
        worldData->objects[i].refractive_index = worldDataOld->objects[i].refractive_index;
	}
	worldData->num_combine_modifiers = worldDataOld->num_combine_modifiers;
	for (int i = 0; i < 32; i++) {
		worldData->combineModifiers[i].data1 = worldDataOld->combineModifiers[i].data1;
		worldData->combineModifiers[i].data2 = worldDataOld->combineModifiers[i].data2;
		worldData->combineModifiers[i].data3 = worldDataOld->combineModifiers[i].data3;
		worldData->combineModifiers[i].data4 = worldDataOld->combineModifiers[i].data4;
		worldData->combineModifiers[i].int1 = worldDataOld->combineModifiers[i].int1;
		worldData->combineModifiers[i].int2 = worldDataOld->combineModifiers[i].int2;
		worldData->combineModifiers[i].int3 = worldDataOld->combineModifiers[i].int3;
		worldData->combineModifiers[i].int4 = worldDataOld->combineModifiers[i].int4;
		worldData->combineModifiers[i].int5 = worldDataOld->combineModifiers[i].int5;
		worldData->combineModifiers[i].int6 = worldDataOld->combineModifiers[i].int6;
		worldData->combineModifiers[i].int7 = worldDataOld->combineModifiers[i].int7;
		worldData->combineModifiers[i].int8 = worldDataOld->combineModifiers[i].int8;
		worldData->combineModifiers[i].index1 = worldDataOld->combineModifiers[i].index1;
		worldData->combineModifiers[i].index2 = worldDataOld->combineModifiers[i].index2;
		worldData->combineModifiers[i].type = worldDataOld->combineModifiers[i].type;
		worldData->combineModifiers[i].index1Type = worldDataOld->combineModifiers[i].index1Type;
	}
	worldData->num_domain_modifiers = worldDataOld->num_domain_modifiers;
	for (int i = 0; i < 32; i++) {
		worldData->domainModifiers[i].data1 = worldDataOld->domainModifiers[i].data1;
        worldData->domainModifiers[i].data2 = worldDataOld->domainModifiers[i].data2;
        worldData->domainModifiers[i].data3 = worldDataOld->domainModifiers[i].data3;
        worldData->domainModifiers[i].data4 = worldDataOld->domainModifiers[i].data4;
        worldData->domainModifiers[i].int1 = worldDataOld->domainModifiers[i].int1;
        worldData->domainModifiers[i].int2 = worldDataOld->domainModifiers[i].int2;
        worldData->domainModifiers[i].int3 = worldDataOld->domainModifiers[i].int3;
        worldData->domainModifiers[i].int4 = worldDataOld->domainModifiers[i].int4;
        worldData->domainModifiers[i].int5 = worldDataOld->domainModifiers[i].int5;
        worldData->domainModifiers[i].int6 = worldDataOld->domainModifiers[i].int6;
        worldData->domainModifiers[i].int7 = worldDataOld->domainModifiers[i].int7;
        worldData->domainModifiers[i].int8 = worldDataOld->domainModifiers[i].int8;
        worldData->domainModifiers[i].index1 = worldDataOld->domainModifiers[i].index1;
        worldData->domainModifiers[i].type = worldDataOld->domainModifiers[i].type;
        worldData->domainModifiers[i].index1Type = worldDataOld->domainModifiers[i].index1Type;
	}
}

// Public functions

void VulkanRenderer::run(GLFWwindow* window) {
    initWindow(window);
    initVulkan();
    initImgui();
    mainLoop();
    cleanup();
}

void VulkanRenderer::initRenderer(GLFWwindow* window) {
    initWindow(window);
    initVulkan();
    initImgui();
}

void VulkanRenderer::initUniforms(SaveData* saveDataIn) {
    this->saveData = saveDataIn;
    true;
}

void VulkanRenderer::initGui(bool* inputEnabled, void (*enableInput)(), std::unordered_map<std::string, AnimationData>* animationsIn) {
	imgui.initGui(this, saveData, inputEnabled, enableInput, animationsIn);
}

void VulkanRenderer::drawFrame() {
    drawFramePrivate();
}

void VulkanRenderer::cleanupRenderer() {
    cleanup();
}

bool VulkanRenderer::uiWantsMouse(){
	return ImGui::GetIO().WantCaptureMouse;
}

bool VulkanRenderer::uiWantsKeyboard(){
	return ImGui::GetIO().WantCaptureKeyboard;
}

int VulkanRenderer::getWindowWidth() {
	int width, height;
	glfwGetWindowSize(window, &width, &height);
    return width;
}

int VulkanRenderer::getWindowHeight() {
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	return height;
}

void VulkanRenderer::swapTexture(const std::string& newTexturePath, size_t index) {

    vkDeviceWaitIdle(device);

    // Cleanup old texture
    cleanupTexture(index);

    // Create new texture
    createTextureImage(newTexturePath, index);
    createTextureImageView(index);
    createTextureSampler();

    vkDeviceWaitIdle(device);

    updateDescriptorSets();

    //recordCommandBuffers();

    vkDeviceWaitIdle(device);
}

void VulkanRenderer::updateOldSaves() {
	imgui.updateOldSaves_v0_1_2();
    imgui.updateOldSaves_v0_1_3(false);
}

void VulkanRenderer::wait() {
	vkDeviceWaitIdle(device);
}

GLFWwindow* VulkanRenderer::getWindow() {
	return window;
}

// Private functions

// Create Window
void VulkanRenderer::initWindow(GLFWwindow* window) {
    this->window = window;
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}


// Init Vulkan 
void VulkanRenderer::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createTextureImage(INIT_SKYBOX, 0);
    createTextureImageView(0);
    for (int i = 1; i < MAX_IMAGES; i++) {
        createTextureImage(INIT_TEXTURE, i);
		createTextureImageView(i);
    }
    createTextureSampler();
    //loadModel();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
    swapTexture(INIT_SKYBOX, 0);
    swapTexture(TEST_TEXTURE, 1);
}


// Init ImGUI
void VulkanRenderer::initImgui()
{
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo info;
    info.DescriptorPool = descriptorPool;
    info.RenderPass = renderPass;
    info.Device = device;
    info.PhysicalDevice = physicalDevice;
    info.ImageCount = MAX_FRAMES_IN_FLIGHT;
    info.MsaaSamples = msaaSamples;
    ImGui_ImplVulkan_Init(&info);

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    endSingleTimeCommands(commandBuffer);

    vkDeviceWaitIdle(device);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}


// Create Single Time Commands
VkCommandBuffer VulkanRenderer::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}


// Create Vulkan Instance
void VulkanRenderer::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}


// Enable Validation/Debugging
bool VulkanRenderer::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void VulkanRenderer::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}


// Create Render Surface
void VulkanRenderer::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}


// Pick Physics Device (GPU)
void VulkanRenderer::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}


// Create Logical Device
void VulkanRenderer::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}


// Create Swap Chain
void VulkanRenderer::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

VulkanRenderer::SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanRenderer::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createColorResources();
    createDepthResources();
    createFramebuffers();
}

void VulkanRenderer::cleanupSwapChain() {
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);

    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}


// Create Image Views
void VulkanRenderer::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}


// Create The Graphics Pipeline - I might be able to remove most of this since i am only working/mainly in the fragment shader
void VulkanRenderer::createGraphicsPipeline() {
    auto vertShaderCode = readFile("vert.spv");
    auto fragShaderCode = readFile("frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = msaaSamples;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}


// Create Render Pass
void VulkanRenderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}


// Create Frame Buffers
void VulkanRenderer::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
            colorImageView,
            depthImageView,
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}


// Create Command Buffers
void VulkanRenderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void VulkanRenderer::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

// This is the main command list for each draw call, this is probably what will be updated to add new uniforms and stuff
void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //ImGui::ShowDemoWindow();

    imgui.drawWindow();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, 0, NULL);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}


// Create Synchronization Objects
void VulkanRenderer::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}


// Create Images
void VulkanRenderer::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

void VulkanRenderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}


// Create Textures
/**
void VulkanRenderer::createTextureImage(const std::string& texturePath) {
    int texWidth, texHeight, texChannels;
    std::cout << texturePath << std::endl;
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
}
/**/

void VulkanRenderer::createTextureImage(const std::string& texturePath, size_t index) {
    if (index >= MAX_IMAGES) {
        throw std::runtime_error("Texture index out of bounds.");
    }

    // Load texture from the file path
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    // Create staging buffer and transfer image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    // Cleanup previous texture at this index
    cleanupTexture(index);

    // Create new image for this texture
    createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage[index], textureImageMemory[index]);

    transitionImageLayout(textureImage[index], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(stagingBuffer, textureImage[index], static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    generateMipmaps(textureImage[index], VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
}

void VulkanRenderer::createTextureImageView(size_t index) {
    imageView[index] = createImageView(textureImage[index], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

void VulkanRenderer::createTextureSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}
/**
void VulkanRenderer::cleanupTexture() {
    vkDestroyImageView(device, textureImageView, nullptr);
    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
}
/**/

void VulkanRenderer::cleanupTexture(size_t index) {
    if (index >= MAX_IMAGES) {
        throw std::runtime_error("Texture index out of bounds.");
    }

    vkDestroyImageView(device, imageView[index], nullptr);
    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImage(device, textureImage[index], nullptr);
    vkFreeMemory(device, textureImageMemory[index], nullptr);
}


// Create Depth Buffer
void VulkanRenderer::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();

    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

VkFormat VulkanRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanRenderer::findDepthFormat() {
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool VulkanRenderer::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


// Create Model
/*
void VulkanRenderer::loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}
*/

// Create MipMaps
void VulkanRenderer::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    endSingleTimeCommands(commandBuffer);
}


// Setup Multisampling
VkSampleCountFlagBits VulkanRenderer::getMaxUsableSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void VulkanRenderer::createColorResources() {
    VkFormat colorFormat = swapChainImageFormat;

    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}


// Create Vertex Buffer - Not Really Needed for a Path Tracer
void VulkanRenderer::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}


// Create Uniforms
void VulkanRenderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding cameraLayoutBinding{};
    cameraLayoutBinding.binding = 0;
    cameraLayoutBinding.descriptorCount = 1;
    cameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraLayoutBinding.pImmutableSamplers = nullptr;
    cameraLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding worldObjectsLayoutBinding{};
    worldObjectsLayoutBinding.binding = 1;
    worldObjectsLayoutBinding.descriptorCount = 1;
    worldObjectsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    worldObjectsLayoutBinding.pImmutableSamplers = nullptr;
    worldObjectsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    /*
    VkDescriptorSetLayoutBinding worldModifiersLayoutBinding{};
    worldModifiersLayoutBinding.binding = 2;
    worldModifiersLayoutBinding.descriptorCount = 1;
    worldModifiersLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    worldModifiersLayoutBinding.pImmutableSamplers = nullptr;
    worldModifiersLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutBinding worldIndicesLayoutBinding{};
    worldIndicesLayoutBinding.binding = 3;
    worldIndicesLayoutBinding.descriptorCount = 1;
    worldIndicesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    worldIndicesLayoutBinding.pImmutableSamplers = nullptr;
    worldIndicesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    /**
    
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 4;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    */

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 4;
    samplerLayoutBinding.descriptorCount = MAX_IMAGES;  // Set max images
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    std::array<VkDescriptorSetLayoutBinding, 3> bindings = { cameraLayoutBinding, worldObjectsLayoutBinding, /* worldModifiersLayoutBinding, worldIndicesLayoutBinding,*/ samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanRenderer::createUniformBuffers() {
    VkDeviceSize cameraBufferSize = sizeof(CameraData);
    VkDeviceSize worldBufferSize = sizeof(WorldObjectsData);
    //VkDeviceSize worldModifiersBufferSize = sizeof(WorldModifiersData);
    //VkDeviceSize worldIndicesBufferSize = sizeof(WorldIndicesData);

    cameraUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    cameraUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    worldObjectsUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    worldObjectsUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    //worldModifiersUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    //worldModifiersUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    //worldIndicesUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    //worldIndicesUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(cameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, cameraUniformBuffers[i], cameraUniformBuffersMemory[i]);
        createBuffer(worldBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, worldObjectsUniformBuffers[i], worldObjectsUniformBuffersMemory[i]);
        //createBuffer(worldModifiersBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, worldModifiersUniformBuffers[i], worldModifiersUniformBuffersMemory[i]);
        //createBuffer(worldIndicesBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, worldIndicesUniformBuffers[i], worldIndicesUniformBuffersMemory[i]);
    }
}

void VulkanRenderer::updateUniformBuffer(uint32_t currentImage) {
    void* data;
    vkMapMemory(device, cameraUniformBuffersMemory[currentImage], 0, sizeof(CameraData), 0, &data);
    memcpy(data, &saveData->camData, sizeof(CameraData));
    vkUnmapMemory(device, cameraUniformBuffersMemory[currentImage]);

    vkMapMemory(device, worldObjectsUniformBuffersMemory[currentImage], 0, sizeof(WorldObjectsData), 0, &data);
    memcpy(data, &saveData->worldData, sizeof(WorldObjectsData));
    vkUnmapMemory(device, worldObjectsUniformBuffersMemory[currentImage]);

    //vkMapMemory(device, worldModifiersUniformBuffersMemory[currentImage], 0, sizeof(WorldModifiersData), 0, &data);
    //memcpy(data, worldModifiersData, sizeof(WorldModifiersData));
    //vkUnmapMemory(device, worldModifiersUniformBuffersMemory[currentImage]);
    
    //vkMapMemory(device, worldIndicesUniformBuffersMemory[currentImage], 0, sizeof(WorldObjectsData), 0, &data);
    //memcpy(data, worldIndicesData, sizeof(WorldObjectsData));
    //vkUnmapMemory(device, worldIndicesUniformBuffersMemory[currentImage]);
    
}

void VulkanRenderer::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2 * MAX_IMAGES;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}
/**
void VulkanRenderer::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo cameraBufferInfo{};
        cameraBufferInfo.buffer = cameraUniformBuffers[i];
        cameraBufferInfo.offset = 0;
        cameraBufferInfo.range = sizeof(CameraData);

        VkDescriptorBufferInfo worldObjectsBufferInfo{};
        worldObjectsBufferInfo.buffer = worldObjectsUniformBuffers[i];
        worldObjectsBufferInfo.offset = 0;
        worldObjectsBufferInfo.range = sizeof(WorldObjectsData);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &worldObjectsBufferInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 4;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}


void VulkanRenderer::updateDescriptorSets() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo cameraBufferInfo{};
        cameraBufferInfo.buffer = cameraUniformBuffers[i];
        cameraBufferInfo.offset = 0;
        cameraBufferInfo.range = sizeof(CameraData);

        VkDescriptorBufferInfo worldObjectsBufferInfo{};
        worldObjectsBufferInfo.buffer = worldObjectsUniformBuffers[i];
        worldObjectsBufferInfo.offset = 0;
        worldObjectsBufferInfo.range = sizeof(WorldObjectsData);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        // Camera uniform buffer descriptor
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;  // Binding 0: Camera uniform buffer
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

        // World objects uniform buffer descriptor
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;  // Binding 1: World objects uniform buffer
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &worldObjectsBufferInfo;

        // Combined image sampler descriptor (texture)
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 4;  // Binding 4: Combined image sampler
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &imageInfo;

        // Update the descriptor sets
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}
/**/

void VulkanRenderer::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo cameraBufferInfo{};
        cameraBufferInfo.buffer = cameraUniformBuffers[i];
        cameraBufferInfo.offset = 0;
        cameraBufferInfo.range = sizeof(CameraData);

        VkDescriptorBufferInfo worldObjectsBufferInfo{};
        worldObjectsBufferInfo.buffer = worldObjectsUniformBuffers[i];
        worldObjectsBufferInfo.offset = 0;
        worldObjectsBufferInfo.range = sizeof(WorldObjectsData);

        // Prepare image array
        std::vector<VkDescriptorImageInfo> imageInfos(MAX_IMAGES);
        for (size_t j = 0; j < MAX_IMAGES; j++) {
            imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[j].imageView = imageView[j];  // Assuming imageViews is a vector of VkImageView
            imageInfos[j].sampler = textureSampler;  // Assuming one sampler is reused
        }

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].pBufferInfo = &worldObjectsBufferInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 4;  // Binding for images
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = static_cast<uint32_t>(imageInfos.size());
        descriptorWrites[2].pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanRenderer::updateDescriptorSets() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo cameraBufferInfo{};
        cameraBufferInfo.buffer = cameraUniformBuffers[i];
        cameraBufferInfo.offset = 0;
        cameraBufferInfo.range = sizeof(CameraData);

        VkDescriptorBufferInfo worldObjectsBufferInfo{};
        worldObjectsBufferInfo.buffer = worldObjectsUniformBuffers[i];
        worldObjectsBufferInfo.offset = 0;
        worldObjectsBufferInfo.range = sizeof(WorldObjectsData);

        // Prepare image array for multiple textures
        std::vector<VkDescriptorImageInfo> imageInfos(MAX_IMAGES);
        for (size_t j = 0; j < MAX_IMAGES; j++) {
            imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[j].imageView = imageView[j];  // Assuming imageViews is a vector of VkImageView
            imageInfos[j].sampler = textureSampler;  // Assuming one sampler is reused for all images
        }

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        // Camera uniform buffer descriptor
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;  // Binding 0: Camera uniform buffer
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

        // World objects uniform buffer descriptor
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;  // Binding 1: World objects uniform buffer
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &worldObjectsBufferInfo;

        // Combined image sampler descriptor (array of textures)
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 4;  // Binding 4: Combined image sampler
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = static_cast<uint32_t>(imageInfos.size());
        descriptorWrites[2].pImageInfo = imageInfos.data();

        // Update the descriptor sets
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}


// The Main Draw Loop
void VulkanRenderer::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}


// Draw Frame
void VulkanRenderer::drawFramePrivate() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(currentFrame);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
    

//Clean Up
void VulkanRenderer::cleanup() {
    vkDeviceWaitIdle(device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::GetIO().BackendRendererUserData = nullptr;
    ImGui::DestroyContext();


    cleanupSwapChain();

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, cameraUniformBuffers[i], nullptr);
        vkFreeMemory(device, cameraUniformBuffersMemory[i], nullptr);
        vkDestroyBuffer(device, worldObjectsUniformBuffers[i], nullptr);
        vkFreeMemory(device, worldObjectsUniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    // Cleanup texture samplers, image views, and images
    for (size_t i = 0; i < MAX_IMAGES; i++) {
        vkDestroyImageView(device, imageView[i], nullptr);
        vkDestroyImage(device, textureImage[i], nullptr);
        vkFreeMemory(device, textureImageMemory[i], nullptr);
    }

    vkDestroySampler(device, textureSampler, nullptr);
    /**
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
    /**/
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
