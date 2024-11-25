#ifndef _WINDOW_H
#define _WINDOW_H

#include <vector>
#include <GLFW/glfw3.h>
#include <model/model.h>

class Window
{
public:
    Window(int width, int height, const char *title);
    ~Window();

    void addModel(Model &model);
    void run();

private:
    GLFWwindow *window;
    std::vector<Model *> models;
    unsigned int shaderProgram;

    void initOpenGL();
    void render();
    void processInput();
    unsigned int compileShader(const char *source, GLenum type);
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    void setupMesh(Model &model);
};

std::string readShaderFile(const std::string &filePath);

#endif