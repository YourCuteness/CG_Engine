#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <draw/transform.h>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;

    Vertex(glm::vec3 pos, glm::vec3 n, glm::vec2 tc)
        : position(pos), normal(n), texCoord(tc) {}
};

class Model
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    Transform transform; // 添加 Transform 以控制模型的变换

    void computeAABB();                             // 计算 AABB 的方法
    bool isPointInsideAABB(const glm::vec3 &point); // 判断点是否在 AABB 内
    bool intersectsRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir);

    bool loadOBJ(const std::string &filePath);

    void updateTransform();
    void setPosition(const glm::vec3 &newPosition);
    void setRotation(const glm::quat &newRotation);
    void setScale(const glm::vec3 &newScale);

private:
    glm::vec3 minBounds; // AABB 最小点
    glm::vec3 maxBounds; // AABB 最大点

    bool isPositionChanged = false;
    bool isRotationChanged = false;
    bool isScaleChanged = false;

    void processFace(const std::vector<std::string> &face,
                     const std::vector<glm::vec3> &positions,
                     const std::vector<glm::vec3> &normals,
                     const std::vector<glm::vec2> &texCoords);
};

#endif