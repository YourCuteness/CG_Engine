#pragma once

#include "transform.h"
#include <GLFW/glfw3.h>

class Camera
{
public:
    Transform transform;

public:
    Camera();

    virtual ~Camera() = default;

    glm::mat4 getViewMatrix() const;

    virtual glm::mat4 getProjectionMatrix() const = 0;

    virtual void zoom(float zoomFactor) = 0; // 新增的纯虚函数，用于缩放

    void camera_control(GLFWwindow *window);

    glm::vec3 getPosition() const
    {
        return transform.position;
    }
};

class PerspectiveCamera : public Camera
{
public:
    float fovy;
    float aspect;
    float znear;
    float zfar;

public:
    PerspectiveCamera(float fovy, float aspect, float znear, float zfar);

    ~PerspectiveCamera() = default;

    glm::mat4 getProjectionMatrix() const override;

    void zoom(float zoomFactor) override;
};

class OrthographicCamera : public Camera
{
public:
    float left;
    float right;
    float bottom;
    float top;
    float znear;
    float zfar;

public:
    OrthographicCamera(float left, float right, float bottom, float top, float znear, float zfar);

    ~OrthographicCamera() = default;

    glm::mat4 getProjectionMatrix() const override;

    void zoom(float zoomFactor) override;
};