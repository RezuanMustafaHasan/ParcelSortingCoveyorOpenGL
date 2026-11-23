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
#include <algorithm>
#include <cmath>

namespace {
Camera camera;
FactoryScene* scene = nullptr;
bool firstMouse = true;
bool mouseCaptured = true;
bool gouraudShading = false; // Keep the original Phong shading on startup.
#ifdef __APPLE__
// macOS may move an exclusive fullscreen window to a different Space at
// startup. Open where the user launched the program; F11 still enters fullscreen.
bool fullscreen = false;
#else
bool fullscreen = true;
#endif
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
    if (key == GLFW_KEY_ESCAPE) {
        std::cout << "Factory closed with Esc.\n";
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_F11) toggleFullscreen(window);
    if (key == GLFW_KEY_TAB) setMouseCaptured(window, !mouseCaptured);
    if (!scene) return;
    if (key == GLFW_KEY_SPACE) scene->togglePaused();
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) scene->adjustSpeed(0.20f);
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) scene->adjustSpeed(-0.20f);
    if (key == GLFW_KEY_R) scene->reset();
    if (key == GLFW_KEY_G) scene->testGate();
    if (key == GLFW_KEY_L) scene->toggleMainLight();
    if (key == GLFW_KEY_P) {
        gouraudShading = !gouraudShading;
        std::cout << "Shading: " << (gouraudShading ? "Gouraud" : "Phong") << '\n' << std::flush;
    }
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

struct ShadowMap {
    GLuint framebuffer = 0, texture = 0;
    ShadowMap() {
        glGenFramebuffers(1, &framebuffer);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        const float border[] = {1,1,1,1};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Shadow framebuffer is incomplete.");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ~ShadowMap() {
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &framebuffer);
    }
};

void saveFrame(const std::string& path, int width, int height) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width)*height*3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    std::ofstream output(path, std::ios::binary);
    if (!output) throw std::runtime_error("Could not write capture: " + path);
    output << "P6\n" << width << ' ' << height << "\n255\n";
    for (int y=height-1; y>=0; --y)
        output.write(reinterpret_cast<const char*>(pixels.data()+static_cast<std::size_t>(y)*width*3), width*3);
}

void printControls() {
    std::cout
        << "\n=== Automated Multi-Level Conveyor Factory ===\n\n"
        << "SPACE       Pause / resume factory\n"
        << "+ / -       Increase / decrease conveyor speed\n"
        << "R           Reset simulation\n"
        << "G           Manually test sorting gate\n"
        << "L           Toggle overhead point light\n"
        << "P           Toggle Phong / Gouraud shading (default: Phong)\n"
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
    std::string capturePath;
    float previewTime = 18.0f;
    int preset = 1;
    try {
        for(int i=1;i<argc;++i) {
            const std::string arg(argv[i]);
            if(arg == "--windowed") fullscreen = false;
            else if(arg == "--gouraud") gouraudShading = true;
            else if(arg == "--capture" && i+1<argc) { capturePath=argv[++i]; fullscreen=false; }
            else if(arg == "--time" && i+1<argc) previewTime=std::stof(argv[++i]);
            else if(arg == "--view" && i+1<argc) preset=std::stoi(argv[++i]);
            else throw std::runtime_error("Usage: conveyor_factory [--windowed] [--gouraud] [--capture frame.ppm] [--time seconds] [--view 1|2|3]");
        }
        if (!std::isfinite(previewTime) || previewTime < 0 || previewTime > 3600 || preset < 1 || preset > 3)
            throw std::runtime_error("Time must be 0..3600 seconds; view must be 1, 2 or 3.");
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    camera.setPreset(preset);
    glfwSetErrorCallback([](int, const char* description) { std::cerr << "GLFW: " << description << '\n'; });
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    if (!capturePath.empty()) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
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
    glfwSetWindowCloseCallback(window, [](GLFWwindow*) {
        std::cout << "Factory window closed.\n";
    });
    setMouseCaptured(window, capturePath.empty());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.09f, 0.14f, 0.16f, 1.0f);

    try {
        const auto shaders = shaderDirectory(argv[0]);
        Shader shader(shaders + "/basic.vert", shaders + "/basic.frag");
        Shader depthShader(shaders + "/basic.vert", shaders + "/depth.frag");
        ShadowMap shadowMap;
        const glm::vec3 lightPosition(10.0f,24.0f,14.0f);
        const glm::mat4 lightView=glm::lookAt(lightPosition,glm::vec3(0,0,0),glm::vec3(0,1,0));
        const glm::mat4 lightProjection=glm::ortho(-19.0f,19.0f,-21.0f,21.0f,1.0f,65.0f);
        FactoryScene factory;
        scene = &factory;
        printControls();
        if (capturePath.empty())
            std::cout << "Factory window opened " << initialWidth << 'x' << initialHeight
                      << (fullscreen ? " fullscreen" : " windowed") << ".\n" << std::flush;
        if(!capturePath.empty()) {
            for(float remaining=previewTime; remaining>0.00001f;) {
                const float step=std::min(remaining,1.0f/60.0f);
                factory.update(step);
                remaining-=step;
            }
        }

        double previousTime = glfwGetTime();
        while (!glfwWindowShouldClose(window)) {
            const double now = glfwGetTime();
            const float dt = static_cast<float>(now - previousTime);
            previousTime = now;

            processContinuousInput(window, dt);
            if(capturePath.empty()) factory.update(dt);

            int width = 1, height = 1;
            glfwGetFramebufferSize(window, &width, &height);
            glBindFramebuffer(GL_FRAMEBUFFER,shadowMap.framebuffer);
            glViewport(0,0,2048,2048);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(2.0f,4.0f);
            depthShader.use();
            depthShader.setMat4("view",lightView);
            depthShader.setMat4("projection",lightProjection);
            factory.render(depthShader,camera.position());
            glDisable(GL_POLYGON_OFFSET_FILL);
            glBindFramebuffer(GL_FRAMEBUFFER,0);
            glViewport(0,0,width,height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            shader.setBool("useGouraud", gouraudShading);
            shader.setMat4("view", camera.viewMatrix());
            shader.setMat4("projection", glm::perspective(glm::radians(camera.zoom()),
                           static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1), 0.1f, 100.0f));
            shader.setVec3("viewPosition", camera.position());
            for(int i=0;i<4;++i) {
                const std::string prefix="pointLights["+std::to_string(i)+"].";
                shader.setVec3(prefix+"position", {i%2 ? 4.8f : -4.8f,6.75f,i<2 ? -6.0f : 5.0f});
                shader.setVec3(prefix+"color", {1.0f,0.91f,0.76f});
                shader.setFloat(prefix+"constant",1.0f);
                shader.setFloat(prefix+"linear",0.025f);
                shader.setFloat(prefix+"quadratic",0.012f);
            }
            shader.setVec3("directionalDirection",-lightPosition);
            shader.setVec3("directionalColor",{0.68f,0.66f,0.58f});
            shader.setMat4("lightSpaceMatrix",lightProjection*lightView);
            shader.setInt("shadowMap",0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,shadowMap.texture);
            factory.render(shader,camera.position());
            if(!capturePath.empty()) {
                glFinish();
                if(glGetError()!=GL_NO_ERROR) throw std::runtime_error("OpenGL error during capture.");
                saveFrame(capturePath,width,height);
                break;
            }

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
