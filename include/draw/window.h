#ifndef _WINDOW_H
#define _WINDOW_H

#include <vector>
#include <GLFW/glfw3.h>
#include <model/model.h>
#include <draw/camera.h>
#include <draw/mouse.h>

class Window
{
public:
    Window(int width, int height, const char *title);
    ~Window();

    void addModel(Model &model);
    void run();
    Input _input;

private:
    GLFWwindow *window;
    std::vector<Model *> models;
    unsigned int shaderProgram;
    Camera *_camera;

    void initOpenGL();
    void render();
    void processInput();
    unsigned int compileShader(const char *source, GLenum type);
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    void setupMesh(Model &model);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow *window, double xPos, double yPos);
    static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
};

std::string readShaderFile(const std::string &filePath);

#endif