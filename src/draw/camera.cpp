#include <draw/camera.h>
#include <draw/window.h>
#include <glm/glm.hpp>

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
    constexpr float cameraRotateSpeed = 2.0f;
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

    Window *_window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    if (_window->_input.mouse.move.xNow != _window->_input.mouse.move.xOld)
    {
        this->transform.rotation *= glm::angleAxis(glm::radians((_window->_input.mouse.move.xNow - _window->_input.mouse.move.xOld) * cameraRotateSpeed * _deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (_window->_input.mouse.move.yNow != _window->_input.mouse.move.yOld)
    {
        this->transform.rotation *= glm::angleAxis(glm::radians((_window->_input.mouse.move.yNow - _window->_input.mouse.move.yOld) * cameraRotateSpeed * _deltaTime), this->transform.rotation * glm::vec3(1.0f, 0.0f, 0.0f));
    }
    _window->_input.forwardState();
}