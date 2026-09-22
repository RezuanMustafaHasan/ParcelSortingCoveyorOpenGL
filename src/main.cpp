#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "FactoryScene.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
Camera camera;
FactoryScene* scene = nullptr;
bool firstMouse = true;
bool mouseCaptured = true;
bool fullscreen = true;
double lastMouseX = 640.0;
double lastMouseY = 360.0;
int windowedX = 100;
int windowedY = 100;
int windowedWidth = 1280;
int windowedHeight = 720;

void framebufferCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow*, double x, double y) {
    if (!mouseCaptured) return;
    if (firstMouse) {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
    }
    const float xOffset = static_cast<float>(x - lastMouseX);
    const float yOffset = static_cast<float>(lastMouseY - y);
    lastMouseX = x;
    lastMouseY = y;
    camera.processMouse(xOffset, yOffset);
}

void setMouseCaptured(GLFWwindow* window, bool captured) {
    mouseCaptured = captured;
    firstMouse = true;
    glfwSetInputMode(window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !mouseCaptured)
        setMouseCaptured(window, true);
}

void toggleFullscreen(GLFWwindow* window) {
    if (fullscreen) {
        glfwSetWindowMonitor(window, nullptr, windowedX, windowedY,
                             windowedWidth, windowedHeight, GLFW_DONT_CARE);
        fullscreen = false;
    } else {
        glfwGetWindowPos(window, &windowedX, &windowedY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
        if (monitor && mode) {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            fullscreen = true;
        }
    }
    firstMouse = true;
}

void scrollCallback(GLFWwindow*, double, double yOffset) {
    camera.processScroll(static_cast<float>(yOffset));
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_F11) toggleFullscreen(window);
    if (key == GLFW_KEY_TAB) setMouseCaptured(window, !mouseCaptured);
    if (!scene) return;
    if (key == GLFW_KEY_SPACE) scene->togglePaused();
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) scene->adjustSpeed(0.20f);
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) scene->adjustSpeed(-0.20f);
    if (key == GLFW_KEY_R) scene->reset();
    if (key == GLFW_KEY_G) scene->testGate();
    if (key == GLFW_KEY_L) scene->toggleMainLight();
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3) {
        camera.setPreset(key - GLFW_KEY_0);
        firstMouse = true;
    }
}

void processContinuousInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard(CameraMovement::Forward, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard(CameraMovement::Backward, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard(CameraMovement::Left, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard(CameraMovement::Right, dt);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.processKeyboard(CameraMovement::Down, dt);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.processKeyboard(CameraMovement::Up, dt);
}

std::string shaderDirectory(const char* executablePath) {
    std::string executable(executablePath ? executablePath : "");
    const std::size_t separator = executable.find_last_of("/\\");
    const std::string executableFolder = separator == std::string::npos ? "." : executable.substr(0, separator);
    const std::vector<std::string> candidates = {"shaders", executableFolder + "/shaders", "../shaders"};
    for (const auto& path : candidates) {
        std::ifstream test(path + "/basic.vert");
        if (test.good()) return path;
    }
    throw std::runtime_error("Could not locate the shaders directory.");
}

void printControls() {
    std::cout
        << "\n=== Automated Multi-Level Conveyor Factory ===\n\n"
        << "SPACE       Pause / resume factory\n"
        << "+ / -       Increase / decrease conveyor speed\n"
        << "R           Reset simulation\n"
        << "G           Manually test sorting gate\n"
        << "L           Toggle overhead point light\n"
        << "1 / 2 / 3   Perspective / top / side camera\n"
        << "W A S D     Move camera\n"
        << "Q / E       Move camera down / up\n"
        << "Mouse       Look around\n"
        << "Left click  Recapture mouse after releasing it\n"
        << "Wheel       Zoom\n"
        << "TAB         Capture / release mouse cursor\n"
        << "F11         Toggle fullscreen / windowed mode\n"
        << "ESC         Exit\n\n";
}
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--windowed") fullscreen = false;
    glfwSetErrorCallback([](int, const char* description) { std::cerr << "GLFW: " << description << '\n'; });
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = primaryMonitor ? glfwGetVideoMode(primaryMonitor) : nullptr;
    const int initialWidth = (fullscreen && videoMode) ? videoMode->width : windowedWidth;
    const int initialHeight = (fullscreen && videoMode) ? videoMode->height : windowedHeight;
    if (fullscreen && videoMode) {
        glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
    } else {
        fullscreen = false;
    }
    GLFWwindow* window = glfwCreateWindow(initialWidth, initialHeight,
        "Automated Multi-Level Conveyor Factory", (fullscreen && videoMode) ? primaryMonitor : nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwSetFramebufferSizeCallback(window, framebufferCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    setMouseCaptured(window, true);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.055f, 0.075f, 0.095f, 1.0f);

    try {
        const auto shaders = shaderDirectory(argv[0]);
        Shader shader(shaders + "/basic.vert", shaders + "/basic.frag");
        FactoryScene factory;
        scene = &factory;
        printControls();

        double previousTime = glfwGetTime();
        while (!glfwWindowShouldClose(window)) {
            const double now = glfwGetTime();
            const float dt = static_cast<float>(now - previousTime);
            previousTime = now;

            processContinuousInput(window, dt);
            factory.update(dt);

            int width = 1, height = 1;
            glfwGetFramebufferSize(window, &width, &height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            shader.setMat4("view", camera.viewMatrix());
            shader.setMat4("projection", glm::perspective(glm::radians(camera.zoom()),
                           static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1), 0.1f, 100.0f));
            shader.setVec3("viewPosition", camera.position());
            shader.setVec3("pointLight.position", {0.0f, 7.1f, 1.0f});
            shader.setVec3("pointLight.color", {1.0f, 0.91f, 0.76f});
            shader.setFloat("pointLight.constant", 1.0f);
            shader.setFloat("pointLight.linear", 0.055f);
            shader.setFloat("pointLight.quadratic", 0.012f);
            shader.setVec3("directionalDirection", {-0.35f, -1.0f, -0.20f});
            shader.setVec3("directionalColor", {0.26f, 0.30f, 0.35f});
            factory.render(shader);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        scene = nullptr;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        scene = nullptr;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
