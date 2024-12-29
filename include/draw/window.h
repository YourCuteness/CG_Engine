#ifndef _WINDOW_H
#define _WINDOW_H

#include <vector>
#include <GLFW/glfw3.h>
#include <model/model.h>
#include <draw/camera.h>
#include <draw/mouse.h>

enum class RenderMode
{
    AlphaTesting,
    AlphaBlending,
    DepthPeeling
};

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
    enum RenderMode _renderMode;
    const float PI = 3.14159265359f;
    float light_r = 5.0f;
    float light_theta = 0.5 * PI;
    float light_phi = 0.0f;
    glm::vec3 _lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec4 _clearColor = glm::vec4(0.26f, 0.61f, 1.0f, 1.0f);
    bool _wireframe = false;

    void initOpenGL();
    void renderUI();
    void render();
    void processInput();
    void addObj();
    unsigned int compileShader(const char *source, GLenum type);
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    void setupMesh(Model &model);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow *window, double xPos, double yPos);
    static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
    void saveSceneAsObj();
};

std::string readShaderFile(const std::string &filePath);

#endif