#include <iostream>
#include <vector>
#include <ctime>
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
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <draw\stb_image_write.h>
#include <model\voxel.h>

bool addobj = false;
char inputBuffer[256] = "";
char inputBuffer1[256] = "";

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
    glm::vec3 cameraPosition = _camera->getPosition();

    for (auto &model : models)
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        unsigned int lightColorLoc = glGetUniformLocation(shaderProgram, "LightColor");
        unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "LightPos");
        unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(_lightColor));
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(glm::vec3(light_r * sin(light_theta) * sin(light_phi), light_r * cos(light_theta), light_r * sin(light_theta) * cos(light_phi))));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPosition));

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
        ImGui::NewLine();
        ImGui::InputText("Model Path", inputBuffer, IM_ARRAYSIZE(inputBuffer));
        addobj = ImGui::Button("Add obj");
        ImGui::NewLine();
        ImGui::InputText("SAVE Path", inputBuffer1, IM_ARRAYSIZE(inputBuffer1));
        if (ImGui::Button("Save Scene as OBJ"))
        {
            saveSceneAsObj();
        }

        ImGui::End();
    }

    if (_selectedModel != -1)
    {
        ImGui::Begin("Model Info");
        ImGui::Text("Selected Model: %d", _selectedModel);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    addObj();
}

void Window::saveToFile(const std::string &filename, const std::vector<unsigned char> &pixels, int width, int height)
{
    if (stbi_write_png(filename.c_str(), width, height, 3, pixels.data(), width * 3))
    {
        std::cout << "Screenshot saved to " << filename << std::endl;
    }
    else
    {
        std::cerr << "Failed to save screenshot to " << filename << std::endl;
    }
}

void Window::captureScreen(const std::string &filename, int width, int height)
{
    // 分配内存存储像素数据
    std::vector<unsigned char> pixels(width * height * 3); // RGB format

    // 从帧缓冲区读取像素数据
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // 像素数据是从左下角开始的，需要翻转到右上角
    for (int y = 0; y < height / 2; ++y)
    {
        for (int x = 0; x < width * 3; ++x)
        {
            std::swap(pixels[y * width * 3 + x], pixels[(height - 1 - y) * width * 3 + x]);
        }
    }

    // 保存到文件（下一步实现保存为 PNG 或其他格式）
    saveToFile(filename, pixels, width, height);
}

void Window::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // 获取当前时间
        std::time_t now = std::time(nullptr);
        std::tm *localTime = std::localtime(&now);

        // 手动格式化时间为 YYYYMMDDHHMMSS
        char timestamp[16];
        std::snprintf(timestamp, sizeof(timestamp), "%04d%02d%02d%02d%02d%02d",
                      1900 + localTime->tm_year, // 年
                      1 + localTime->tm_mon,     // 月
                      localTime->tm_mday,        // 日
                      localTime->tm_hour,        // 时
                      localTime->tm_min,         // 分
                      localTime->tm_sec);        // 秒

        // 拼接文件名
        std::string filename = "..\\material\\screenshot\\" + std::string(timestamp) + ".png";

        captureScreen(filename, width, height);
    }
    if (_selectedModel == -1)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        _camera->camera_control(window);
    }
    else
    {
        glm::vec3 moveDelta(0.0f);

        // 获取相机方向向量
        glm::vec3 cameraFront = _camera->transform.getUp();    // 相机上向向量
        glm::vec3 cameraRight = _camera->transform.getRight(); // 相机右向向量
        float moveSpeed = 0.1f;                                // 调整移动速度

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            moveDelta += cameraFront * moveSpeed; // 沿着前向移动
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            moveDelta -= cameraFront * moveSpeed; // 沿着后向移动
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            moveDelta -= cameraRight * moveSpeed; // 沿着左向移动
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            moveDelta += cameraRight * moveSpeed; // 沿着右向移动

        // 更新模型位置
        glm::vec3 newPosition = moveDelta;
        models[_selectedModel]->setPosition(newPosition); // 使用 setPosition 更新模型位置

        models[_selectedModel]->updateTransform(); // 更新 GPU 数据或其他后续逻辑（如更新矩阵）
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!_isMousePressed)
        {
            dragTime = glfwGetTime();
            glfwGetCursorPos(window, &_lastX, &_lastY);
            _isMousePressed = true; // 标记鼠标按下
        }
        else
        {
            // 鼠标拖动
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // 计算鼠标的偏移量
            float offsetX = static_cast<float>(mouseX - _lastX);
            float offsetY = static_cast<float>(mouseY - _lastY); // Y轴翻转

            _lastX = mouseX;
            _lastY = mouseY;

            // 调整旋转速度
            float rotationSpeed = 0.1f;

            // 根据鼠标偏移量来旋转模型
            if (_selectedModel != -1)
            {
                Model &selectedModel = *models[_selectedModel];

                // 旋转矩阵
                glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(offsetX * rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f)); // 绕Y轴旋转
                rotation = glm::rotate(rotation, glm::radians(offsetY * rotationSpeed), glm::vec3(1.0f, 0.0f, 0.0f));                  // 绕X轴旋转

                glm::quat rotationQuat = glm::quat(glm::vec3(glm::radians(offsetY * rotationSpeed), glm::radians(offsetX * rotationSpeed), 0.0f));

                selectedModel.setRotation(rotationQuat); // 使用 setRotation 更新模型旋转
            }
        }
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && _isMousePressed)
    {
        // 鼠标按下后释放时触发
        _isMousePressed = false; // 重置状态
        float releaseTime = glfwGetTime();
        dragTime = releaseTime - dragTime;
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (dragTime < 0.5)
        {
            processMouseClick(mouseX, mouseY);
        }
    }
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

void Window::addObj()
{
    if (addobj)
    {
        std::string pathToModel;
        for (int i = 0; i < sizeof(inputBuffer); i++)
        {
            if (inputBuffer[i] == '"')
            {
                continue;
            }
            else
            {
                pathToModel += inputBuffer[i];
            }
        }
        Model *model1 = new Model();
        if (!model1->loadOBJ(pathToModel))
        { // 请修改为你的 OBJ 文件路径
            std::cerr << "Failed to load model1" << std::endl;
            return;
        }
        model1->computeAABB();

        this->addModel(*model1);
        addobj = false;
    }
}

void Window::addCube()
{
    Model cubeModel;
    auto [vertices, indices] = createCube();
    cubeModel.vertices = vertices;
    cubeModel.indices = indices;
    this->addModel(cubeModel);
}

void Window::addSphere()
{
    Model sphereModel;
    auto [vertices, indices] = createSphere(1.0f, 36, 18);
    sphereModel.vertices = vertices;
    sphereModel.indices = indices;
    this->addModel(sphereModel);
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

    float zoomFactor = static_cast<float>(yOffset) * 0.1f; // 调整缩放速度

    // 如果有选中的模型，优先缩放模型
    if (_window->_selectedModel != -1) // 假设 _selectedModel 存储选中模型的索引
    {
        Model &selectedModel = *_window->models[_window->_selectedModel];

        selectedModel.setScale(1.0f + zoomFactor * glm::vec3(1.0f)); // 使用 setScale 更新模型缩放
    }
    else if (_window->_camera) // 如果没有选中模型，则缩放相机
    {
        _window->_camera->zoom(zoomFactor);
    }
}

glm::vec3 Window::screenToWorldRay(double mouseX, double mouseY)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // 将鼠标位置标准化到 [-1, 1]
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height; // Y 方向反转
    float z = 1.0f;

    glm::vec3 rayNDC(x, y, z); // 规范化设备坐标 (NDC)

    // 从 NDC 转换到世界坐标系
    glm::vec4 rayClip(rayNDC.x, rayNDC.y, -1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(_camera->getProjectionMatrix()) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec3 rayWorld = glm::vec3(glm::inverse(_camera->getViewMatrix()) * rayEye);
    return glm::normalize(rayWorld);
}

glm::vec3 Window::getRayIntersectionWithPlane(const glm::vec3 &rayOrigin,
                                              const glm::vec3 &rayDir,
                                              const glm::vec3 &planeNormal,
                                              const glm::vec3 &planePoint)
{
    float denom = glm::dot(planeNormal, rayDir);
    if (fabs(denom) < 1e-6)
        return glm::vec3(FLT_MAX); // 平行，无交点

    float t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;
    return rayOrigin + t * rayDir;
}

void Window::processMouseClick(double mouseX, double mouseY)
{
    glm::vec3 rayOrigin = _camera->getPosition();
    glm::vec3 rayDir = screenToWorldRay(mouseX, mouseY);

    int count = 0;
    for (auto *model : models)
    {
        if (model->intersectsRay(rayOrigin, rayDir))
        {
            _selectedModel = count;
            return;
        }
        count++;
    }
    _selectedModel = -1;
}

void Window::saveSceneAsObj()
{
    std::string filePath;
    for (int i = 0; i < sizeof(inputBuffer1); i++)
    {
        if (inputBuffer1[i] == '"')
        {
            continue;
        }
        else
        {
            filePath += inputBuffer1[i];
        }
    }
    std::ofstream outFile(filePath);
    if (!outFile.is_open())
    {
        std::cerr << "Unable to open file for writing: " << filePath << std::endl;
        return;
    }

    int vertexOffset = 1;   // OBJ 文件顶点索引从1开始
    int normalOffset = 1;   // 法线索引
    int texCoordOffset = 1; // 纹理坐标索引

    // 遍历每个模型
    for (auto &model : models)
    {
        // 写入顶点数据
        for (const auto &vertex : model->vertices)
        {
            // 写入顶点坐标
            outFile << "v "
                    << vertex.position.x << " "
                    << vertex.position.y << " "
                    << vertex.position.z << std::endl;

            // 写入法线数据
            outFile << "vn "
                    << vertex.normal.x << " "
                    << vertex.normal.y << " "
                    << vertex.normal.z << std::endl;

            // 写入纹理坐标数据
            outFile << "vt "
                    << vertex.texCoord.x << " "
                    << vertex.texCoord.y << std::endl;
        }

        // 写入面数据
        for (size_t i = 0; i < model->indices.size(); i += 3)
        {
            // OBJ 文件的面格式为：f vertex/texcoord/normal vertex/texcoord/normal vertex/texcoord/normal
            outFile << "f "
                    << vertexOffset + model->indices[i] << "/"
                    << texCoordOffset + model->indices[i] << "/"
                    << normalOffset + model->indices[i] << " "
                    << vertexOffset + model->indices[i + 1] << "/"
                    << texCoordOffset + model->indices[i + 1] << "/"
                    << normalOffset + model->indices[i + 1] << " "
                    << vertexOffset + model->indices[i + 2] << "/"
                    << texCoordOffset + model->indices[i + 2] << "/"
                    << normalOffset + model->indices[i + 2] << std::endl;
        }

        // 更新偏移量
        vertexOffset += model->vertices.size();
        normalOffset += model->vertices.size();
        texCoordOffset += model->vertices.size();
    }

    outFile.close();
    std::cout << "Scene saved to " << filePath << std::endl;
}