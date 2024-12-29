#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <model/model.h>
#include <draw/window.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

Window::Window(int width, int height, const char *title)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(1);
    }
    _camera = new PerspectiveCamera(glm::radians(60.0f), 1.0f * width / height, 0.1f, 10000.0f);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(1);
    }

    // init imGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    initOpenGL();
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::addModel(Model &model)
{
    models.push_back(&model);
    setupMesh(model); // 在添加模型时设置顶点数据
}

void Window::initOpenGL()
{
    // 设置视口
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 读取着色器文件
    std::string vertexShaderSource = readShaderFile("../material/shader/basic_light.vert");
    std::string fragmentShaderSource = readShaderFile("../material/shader/basic_light.frag");

    // 编译和链接着色器程序
    unsigned int vertexShader = compileShader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);
    shaderProgram = linkProgram(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

unsigned int Window::compileShader(const char *source, GLenum type)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    return shader;
}

unsigned int Window::linkProgram(unsigned int vertexShader, unsigned int fragmentShader)
{
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    return program;
}

void Window::setupMesh(Model &model)
{
    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vertex), &model.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(unsigned int), &model.indices[0], GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(0);

    // 法线属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // 纹理坐标属性
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    model.VAO = VAO;
    model.VBO = VBO;
    model.EBO = EBO;
}

void Window::render()
{
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glm::mat4 view = _camera->getViewMatrix();
    glm::mat4 projection = _camera->getProjectionMatrix();
    // glm::vec3 cameraPosition = _camera->getPosition();

    for (auto &model : models)
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        unsigned int lightColorLoc = glGetUniformLocation(shaderProgram, "LightColor");
        unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "LightPos");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(_lightColor));
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(glm::vec3(light_r * sin(light_theta) * sin(light_phi), light_r * cos(light_theta), light_r * sin(light_theta) * cos(light_phi))));

        if (_wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 启用线框模式
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 启用填充模式
        }

        glBindVertexArray(model->VAO);
        glDrawElements(GL_TRIANGLES, static_cast<int>(model->indices.size()), GL_UNSIGNED_INT, 0);
    }
}

void Window::renderUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;

    if (!ImGui::Begin("Control Panel", nullptr, flags))
    {
        ImGui::End();
    }
    else
    {
        ImGui::Checkbox("wireframe", &_wireframe);
        ImGui::NewLine();
        ImGui::SliderFloat("r", &light_r, 0.0f, 10.0f);
        ImGui::SliderFloat("theta", &light_theta, 0.0f, 2 * PI);
        ImGui::SliderFloat("phi", &light_phi, 0.0f, 2 * PI);
        ImGui::ColorEdit3("light color", (float *)&_lightColor);
        ImGui::NewLine();
        ImGui::ColorEdit3("background", (float *)&_clearColor);

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    _camera->camera_control(window);
}

void Window::run()
{
    while (!glfwWindowShouldClose(window))
    {
        processInput();

        render();
        renderUI();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

std::string readShaderFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

void Window::cursorPosCallback(GLFWwindow *window, double xPos, double yPos)
{
    Window *_window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    _window->_input.mouse.move.xNow = static_cast<float>(xPos);
    _window->_input.mouse.move.yNow = static_cast<float>(yPos);
}

void Window::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    Window *_window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            _window->_input.mouse.press.left = true;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            _window->_input.mouse.press.middle = true;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            _window->_input.mouse.press.right = true;
            break;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            _window->_input.mouse.press.left = false;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            _window->_input.mouse.press.middle = false;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            _window->_input.mouse.press.right = false;
            break;
        }
    }
}

void Window::scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    Window *_window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    _window->_input.mouse.scroll.xOffset = static_cast<float>(xOffset);
    _window->_input.mouse.scroll.yOffset = static_cast<float>(yOffset);
}