#include <draw/camera.h>
#include <draw/window.h>
#include <glm/glm.hpp>

Camera::Camera()
{
    transform.position = glm::vec3(0.0f, 0.0f, 5.0f);
    transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(
        transform.position, transform.position + transform.getFront(), transform.getUp());
}

PerspectiveCamera::PerspectiveCamera(float fovy, float aspect, float znear, float zfar)
    : fovy(fovy), aspect(aspect), znear(znear), zfar(zfar) {}

glm::mat4 PerspectiveCamera::getProjectionMatrix() const
{
    return glm::perspective(fovy, aspect, znear, zfar);
}

OrthographicCamera::OrthographicCamera(
    float left, float right, float bottom, float top, float znear, float zfar)
    : left(left), right(right), top(top), bottom(bottom), znear(znear), zfar(zfar) {}

glm::mat4 OrthographicCamera::getProjectionMatrix() const
{
    return glm::ortho(left, right, bottom, top, znear, zfar);
}

float lastFrame = 0.0f;
void Camera::camera_control(GLFWwindow *window)
{
    constexpr float cameraMoveSpeed = 5.0f;
    constexpr float cameraRotateSpeed = 10.0f;
    float currentFrame = glfwGetTime();
    float _deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        this->transform.position +=
            this->transform.getFront() * cameraMoveSpeed * _deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE)
    {
        this->transform.position -=
            this->transform.getRight() * cameraMoveSpeed * _deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE)
    {
        this->transform.position -=
            this->transform.getFront() * cameraMoveSpeed * _deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE)
    {
        this->transform.position +=
            this->transform.getRight() * cameraMoveSpeed * _deltaTime;
    }

    // 空格键：沿 Y 正方向移动
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        this->transform.position += glm::vec3(0.0f, cameraMoveSpeed * _deltaTime, 0.0f);
    }

    // Ctrl 键：沿 Y 负方向移动
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        this->transform.position -= glm::vec3(0.0f, cameraMoveSpeed * _deltaTime, 0.0f);
    }

    Window *_window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    if (_window->_input.mouse.move.xNow != _window->_input.mouse.move.xOld && _window->_input.mouse.press.left == true)
    {
        this->transform.rotation *= glm::angleAxis(glm::radians((_window->_input.mouse.move.xOld - _window->_input.mouse.move.xNow) * cameraRotateSpeed * _deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (_window->_input.mouse.move.yNow != _window->_input.mouse.move.yOld && _window->_input.mouse.press.left == true)
    {
        this->transform.rotation *= glm::angleAxis(glm::radians((_window->_input.mouse.move.yOld - _window->_input.mouse.move.yNow) * cameraRotateSpeed * _deltaTime), this->transform.rotation * glm::vec3(1.0f, 0.0f, 0.0f));
    }
    _window->_input.forwardState();
}

void PerspectiveCamera::zoom(float zoomFactor)
{
    // 调整视角，确保 fovy 在合理范围内
    fovy -= zoomFactor;
    if (fovy < 1.0f)
        fovy = 1.0f;
    if (fovy > 45.0f)
        fovy = 45.0f;
}

void OrthographicCamera::zoom(float zoomFactor)
{
    // 调整边界，确保缩放不导致反向
    left += zoomFactor;
    right -= zoomFactor;
    bottom += zoomFactor;
    top -= zoomFactor;

    if (left >= right)
    {
        left = -1.0f;
        right = 1.0f;
    }
    if (bottom >= top)
    {
        bottom = -1.0f;
        top = 1.0f;
    }
}