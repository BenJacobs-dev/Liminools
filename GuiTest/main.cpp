#include "main.h"
#include "json.hpp"

const uint32_t WIDTH = 1200;
const uint32_t HEIGHT = 800;
const char* title = "Vulkan";

GLFWwindow* windowMain;

static SaveData saveData;

using controlFunc = void(*)(void);

std::unordered_map<int, controlFunc> controlActions;
std::unordered_set<int> controlsPressed;

std::unordered_map<std::string, AnimationData> animations = {};

VulkanRenderer app;

bool inputEnabled = false;

float hPI = glm::pi<float>() / 2.0f;

float deltaTimeModified = 0.0f;
float deltaTime = 0.0f;

float speedUp = 1.0f;

// Create the window outside the application class so i can set up input
void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    windowMain = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void configureInput() {
    glfwSetKeyCallback(windowMain, key_callback);
    glfwSetCursorPosCallback(windowMain, cursor_position_callback);
}

void enableInput() {
    glfwSetInputMode(windowMain, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    inputEnabled = true;
}

void disableInput() {
    glfwSetInputMode(windowMain, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    inputEnabled = false;
}

void initCamData() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(windowMain, &width, &height);
    saveData.camData.camera_pos = glm::vec3(0.0f, 0.0f, -3.5f);
    saveData.camData.camera_rot = glm::vec3(0.0f, 0.0f, 0.0f);
    saveData.camData.light_pos = glm::vec3(3.0f, 5.0f, 0.0f);
    saveData.camData.data1 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    saveData.camData.data2 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    saveData.camData.data3 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); //data3.y: bump map height, data3.z: bump map min dist, data3.w: fog density
    saveData.camData.data4 = glm::vec4(90.0f, 1.5f, 0.001f, 1.0f); //data4.x: FOV, data4.y: Player Speed, data4.z: Normal Offset, data4.w: timeMultiplier
    saveData.camData.resolution = glm::vec2(width, height);
    saveData.camData.int1 = 0;
    saveData.camData.int2 = 0;
    saveData.camData.int3 = 0;
    saveData.camData.int4 = 0;
    saveData.camData.int5 = 0;
    saveData.camData.int6 = 0;
    saveData.camData.int7 = 0;
    saveData.camData.int8 = 0;
    saveData.camData.time = 0.0;
    saveData.camData.num_steps = 256;
    saveData.camData.min_step = 0.0001f;
    saveData.camData.max_dist = 2000.0f;
    saveData.camData.ray_depth = 2;
}

void updateCamData() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    static auto prevTime = std::chrono::high_resolution_clock::now();
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    deltaTimeModified = deltaTime * saveData.camData.data4.w;
    prevTime = currentTime;

    int width = 0, height = 0;
    glfwGetFramebufferSize(windowMain, &width, &height);
    saveData.camData.resolution = glm::vec2(width, height);
    saveData.camData.time += deltaTimeModified;
    

    // For all the controls pressed in the controlsPressed set, run the corresponding function from the controls actions map
    for (auto& control : controlsPressed) {
        if (controlActions.find(control) != controlActions.end()) {
            controlActions[control]();
        }
    }
}

// Set Up Keyboard Input
void moveForwards(float speed) {
    speed *= deltaTime * speedUp;
    float x = std::sin(saveData.camData.camera_rot.x) * speed;
    float z = std::cos(saveData.camData.camera_rot.x) * speed;
    saveData.camData.camera_pos.x += x;
    saveData.camData.camera_pos.z += z;
}

void moveLeft(float speed) {
    speed *= deltaTime * speedUp;
    float x = std::sin(saveData.camData.camera_rot.x + glm::radians(90.0f)) * speed;
    float z = std::cos(saveData.camData.camera_rot.x + glm::radians(90.0f)) * speed;
    saveData.camData.camera_pos.x -= x;
    saveData.camData.camera_pos.z -= z;
}

void moveUp(float speed) {
	speed *= deltaTime * speedUp;
	saveData.camData.camera_pos.y += speed;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (app.uiWantsKeyboard()) {
        // ImGui is handling the keyboard, do not handle camera input
        return;
    }
    if (inputEnabled) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_LEFT_SHIFT) {
                speedUp = 1.5f;
			}
            else {
                controlsPressed.insert(key);
            }
        }
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_SHIFT) {
			speedUp = 1.0f;
		}
        else {
            controlsPressed.erase(key);
        }
    }
}

void createControls() {
    //controlActions[GLFW_KEY_SEMICOLON] = []() { enableInput();}; // Haha im dumb, input needs to be enabled for this to work so i can enable the keyboard with the keyboard
    controlActions[GLFW_KEY_ESCAPE] = []() { disableInput();};
    controlActions[GLFW_KEY_W] = []() { moveForwards(saveData.camData.data4.y); };
    controlActions[GLFW_KEY_S] = []() { moveForwards(-saveData.camData.data4.y); };
    controlActions[GLFW_KEY_A] = []() { moveLeft(saveData.camData.data4.y); };
    controlActions[GLFW_KEY_D] = []() { moveLeft(-saveData.camData.data4.y); };
    controlActions[GLFW_KEY_F] = []() { moveForwards(saveData.camData.data4.y); };
    controlActions[GLFW_KEY_G] = []() { moveLeft(saveData.camData.data4.y); };
    controlActions[GLFW_KEY_LEFT_CONTROL] = []() { moveUp(-saveData.camData.data4.y); };
    controlActions[GLFW_KEY_SPACE] = []() { moveUp(saveData.camData.data4.y); };
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (app.uiWantsMouse()) {
        // ImGui is handling the mouse, do not handle camera input
        return;
    }
    if (inputEnabled) {
        static double lastX = xpos;
        static double lastY = ypos;

        float xoffset = xpos - lastX ;
        float yoffset = ypos- lastY;

        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.005f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        saveData.camData.camera_rot.x += xoffset;
        saveData.camData.camera_rot.y += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (saveData.camData.camera_rot.y > hPI)
            saveData.camData.camera_rot.y = hPI;
        if (saveData.camData.camera_rot.y < -hPI)
            saveData.camData.camera_rot.y = -hPI;
    }
    //glfwSetCursorPos(window, app.getWindowWidth()/2, app.getWindowHeight()/2);
}

int calcFPS() {
    static int frameCount = 0;
    static double prevTime = glfwGetTime();
    double currentTime = glfwGetTime();
    frameCount++;
    if (currentTime - prevTime >= 1.0) {
        int fps = frameCount;
        frameCount = 0;
        prevTime = currentTime;
        return fps;
    }
    return 0;
}

void playAnimations() {

    for (auto& [key, animation] : animations) {
        if (animation.typeVal == 0) {
			if (animation.stepI == 0) continue;
		}
		else if (animation.typeVal == 1) {
			if (animation.stepF == 0) continue;
		}
        size_t valLoc = animation.offset;
        /**
        std::cout << "Playing Animation: " << key << std::endl;
        std::cout << "Type: " << animation.typeVal << std::endl;
        std::cout << "Type Animation: " << animation.typeAnimation << std::endl;
        std::cout << "CurI: " << animation.curI << std::endl;
        std::cout << "StepI: " << animation.stepI << std::endl;
        std::cout << "MinI: " << animation.minI << std::endl;
        std::cout << "MaxI: " << animation.maxI << std::endl;
        std::cout << "CurF: " << animation.curF << std::endl;
        std::cout << "StepF: " << animation.stepF << std::endl;
        std::cout << "MinF: " << animation.minF << std::endl;
        std::cout << "MaxF: " << animation.maxF << std::endl;
        std::cout << "Time Per Step: " << animation.timePerStep << std::endl;
        std::cout << "Time Since Last Step: " << animation.timeSinceLastStep << std::endl;
        std::cout << "Increasing: " << animation.increasing << std::endl;
        /**/

        animation.timeSinceLastStep += abs(deltaTimeModified);
        if(animation.timeSinceLastStep >= animation.timePerStep) {
            int numberOfSteps = static_cast<int>(animation.timeSinceLastStep / animation.timePerStep);
            animation.timeSinceLastStep -= numberOfSteps * animation.timePerStep;
            if (animation.typeVal == 0) { // Int
                if (animation.stepI == 0) continue;
                animation.curI += (animation.stepI*animation.increasing - animation.stepI*!animation.increasing) * numberOfSteps;
                if (animation.typeAnimation == 0) {
                    if (animation.curI > animation.maxI && animation.increasing) {
                        animation.curI = animation.minI;
                    }
                    else if (animation.curI < animation.minI && !animation.increasing) {
                        animation.curI = animation.maxI;
                    }
                }
                else if (animation.typeAnimation == 1) {
                    if (animation.curI < animation.minI) {
                        animation.curI = animation.minI;
                        animation.increasing = !animation.increasing;
                    }
                    if (animation.curI > animation.maxI) {
                        animation.curI = animation.maxI;
                        animation.increasing = !animation.increasing;
                    }
                }
                int* value = reinterpret_cast<int*>(reinterpret_cast<char*>(&saveData) + valLoc);
                *value = animation.curI;
            }
            else if (animation.typeVal == 1) { // Float
                if (animation.stepF == 0) continue;
                animation.curF += (animation.stepF*animation.increasing - animation.stepF*!animation.increasing) * numberOfSteps;
                if (animation.typeAnimation == 0) {
                    if (animation.curF > animation.maxF && animation.increasing) {
                        animation.curF = animation.minF;
                    }
                    else if (animation.curF < animation.minF && !animation.increasing) {
						animation.curF = animation.maxF;
					}
                }
                else if (animation.typeAnimation == 1) {
                    if (animation.curF < animation.minF) {
                        animation.curF = animation.minF;
                        animation.increasing = !animation.increasing;
                    }
                    if (animation.curF > animation.maxF) {
                        animation.curF = animation.maxF;
                        animation.increasing = !animation.increasing;
                    }
                }
                float* value = reinterpret_cast<float*>(reinterpret_cast<char*>(&saveData) + valLoc);
                *value = animation.curF;
            }
        }
    }
}

int main() {
    initWindow();
    initCamData();
    //initWorld();
    createControls();
    configureInput();
   


    try {
        app.initUniforms(&saveData);
        app.initGui(&inputEnabled, &enableInput, &animations);
        app.initRenderer(windowMain);
        app.updateOldSaves();

        while (!glfwWindowShouldClose(windowMain)) {
            glfwPollEvents();
            playAnimations();
            updateCamData();
            int fps = calcFPS();
            if(fps != 0) std::cout << fps << std::endl;
            app.drawFrame();
        }

        app.cleanupRenderer();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}