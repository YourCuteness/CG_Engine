#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <model/model.h>
#define STB_IMAGE_IMPLEMENTATION
#include <draw/stb_image.h>
#include <algorithm> // for std::find_if
#include <iterator>  // for std::distance

#define STB_IMAGE_IMPLEMENTATION

bool Model::loadOBJ(const std::string &filePath)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // 忽略空行和注释
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v")
        {
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (token == "vn")
        {
            glm::vec3 norm;
            iss >> norm.x >> norm.y >> norm.z;
            normals.push_back(norm);
        }
        else if (token == "vt")
        {
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        }
        else if (token == "f")
        {
            std::vector<std::string> temp;
            std::vector<std::string> face;
            std::string faceStr;
            while (iss >> faceStr)
            {
                if (faceStr == "#")
                {
                    break;
                }
                temp.push_back(faceStr);
            }
            if (temp.size() == 4)
            {
                face.push_back(temp[0]);
                face.push_back(temp[1]);
                face.push_back(temp[2]);
                face.push_back(temp[0]);
                face.push_back(temp[2]);
                face.push_back(temp[3]);
            }
            else
            {
                face = temp;
            }
            processFace(face, positions, normals, texCoords);
        }
    }

    if (!vertices.empty())
    {
        glm::vec3 min = vertices[0].position;
        glm::vec3 max = vertices[0].position;

        for (const auto &vertex : vertices)
        {
            min = glm::min(min, vertex.position);
            max = glm::max(max, vertex.position);
        }

        glm::vec3 center = (min + max) / 2.0f;

        for (auto &vertex : vertices)
        {
            vertex.position -= center;
        }
    }

    return true;
}

void Model::processFace(const std::vector<std::string> &face,
                        const std::vector<glm::vec3> &positions,
                        const std::vector<glm::vec3> &normals,
                        const std::vector<glm::vec2> &texCoords)
{
    for (const auto &facePart : face)
    {
        std::istringstream faceStream(facePart);
        int vIndex = -1, tIndex = -1, nIndex = -1;
        char separator1, separator2;

        // 解析顶点、纹理、法线索引
        if (faceStream >> vIndex)
        {
            if (faceStream.peek() == '/')
            {
                faceStream >> separator1; // 跳过分隔符 '/'
                if (faceStream.peek() != '/')
                {
                    faceStream >> tIndex; // 纹理索引
                }
                if (faceStream.peek() == '/')
                {
                    faceStream >> separator2; // 跳过第二个 '/'
                    faceStream >> nIndex;     // 法线索引
                }
            }
            // 如果只有顶点
            if (faceStream.eof() && tIndex == -1 && nIndex == -1)
            {
                tIndex = nIndex = -1; // 设置默认值
            }
        }

        // 对索引进行调整，因为OBJ文件中的索引是从1开始的，C++数组是从0开始的
        if (vIndex > 0)
            vIndex--;
        if (tIndex > 0)
            tIndex--;
        if (nIndex > 0)
            nIndex--;

        // 如果索引无效，跳过该面
        if (vIndex < 0 || vIndex >= positions.size() ||
            (tIndex >= 0 && tIndex >= texCoords.size()) ||
            (nIndex >= 0 && nIndex >= normals.size()))
        {
            std::cerr << "Invalid face index in face part: " << facePart << std::endl;
            continue;
        }

        // 构建一个顶点（位置、法线、纹理坐标）的组合
        glm::vec3 norm = (nIndex >= 0) ? normals[nIndex] : glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec2 texCoord = (tIndex >= 0) ? texCoords[tIndex] : glm::vec2(0.0f, 0.0f);
        Vertex vertex(positions[vIndex], norm, texCoord);

        vertices.push_back(vertex);
        unsigned int index = static_cast<unsigned int>(vertices.size()) - 1;

        // 将当前顶点的索引添加到 indices 中
        indices.push_back(index);
    }
}

bool Model::loadTexture(const std::string &texturePath)
{
    std::string cleanPath = texturePath;
    cleanPath.erase(std::remove(cleanPath.begin(), cleanPath.end(), '\"'), cleanPath.end());

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 设置纹理环绕和过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 加载图片
    int width, height, nrChannels;
    unsigned char *data = stbi_load(cleanPath.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        // 假设纹理是 RGB 格式
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture: " << cleanPath << std::endl;
        return false;
    }
    stbi_image_free(data);
    hasTexture = true;
    return true;
}

void Model::computeAABB()
{
    if (vertices.empty())
        return;

    minBounds = glm::vec3(FLT_MAX);
    maxBounds = glm::vec3(-FLT_MAX);

    for (const auto &vertex : vertices)
    {
        minBounds = glm::min(minBounds, vertex.position);
        maxBounds = glm::max(maxBounds, vertex.position);
    }
}

bool Model::isPointInsideAABB(const glm::vec3 &point)
{
    return (point.x >= minBounds.x && point.x <= maxBounds.x &&
            point.y >= minBounds.y && point.y <= maxBounds.y &&
            point.z >= minBounds.z && point.z <= maxBounds.z);
}

bool Model::intersectsRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir)
{
    float tmin = (minBounds.x - rayOrigin.x) / rayDir.x;
    float tmax = (maxBounds.x - rayOrigin.x) / rayDir.x;
    if (tmin > tmax)
        std::swap(tmin, tmax);

    float tymin = (minBounds.y - rayOrigin.y) / rayDir.y;
    float tymax = (maxBounds.y - rayOrigin.y) / rayDir.y;
    if (tymin > tymax)
        std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    tmin = glm::max(tmin, tymin);
    tmax = glm::min(tmax, tymax);

    float tzmin = (minBounds.z - rayOrigin.z) / rayDir.z;
    float tzmax = (maxBounds.z - rayOrigin.z) / rayDir.z;
    if (tzmin > tzmax)
        std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    return true;
}

void Model::setPosition(const glm::vec3 &newPosition)
{
    transform.position = newPosition;
    isPositionChanged = true;
}

void Model::setRotation(const glm::quat &newRotation)
{
    transform.rotation = newRotation;
    isRotationChanged = true;
}

void Model::setScale(const glm::vec3 &newScale)
{
    transform.scale = newScale;
    isScaleChanged = true;
}

void Model::updateTransform()
{
    bool needUpdate = false;

    // 只更新位置
    if (isPositionChanged)
    {
        needUpdate = true;
        for (auto &vertex : vertices)
        {
            vertex.position += transform.position; // 只更新位置，不应用旋转和缩放
        }
        isPositionChanged = false; // 重置位置变化标志
        this->computeAABB();       // 更新AABB包围盒
    }

    // 只更新旋转
    if (isRotationChanged)
    {
        needUpdate = true;
        glm::mat3 rotationMatrix = glm::mat3_cast(transform.rotation); // 从四元数获取旋转矩阵
        for (auto &vertex : vertices)
        {
            vertex.position = transform.position + rotationMatrix * (vertex.position - transform.position);
            vertex.normal = glm::normalize(rotationMatrix * vertex.normal); // 更新法线方向
        }
        isRotationChanged = false; // 重置旋转变化标志
        this->computeAABB();       // 更新AABB包围盒
    }

    // 只更新缩放
    if (isScaleChanged)
    {
        needUpdate = true;
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), transform.position) *
                                glm::mat4_cast(transform.rotation) *
                                glm::scale(glm::mat4(1.0f), transform.scale);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix))); // 计算法线变换矩阵

        for (auto &vertex : vertices)
        {
            vertex.position = transform.position + (vertex.position - transform.position) * transform.scale;
            vertex.normal = glm::normalize(normalMatrix * vertex.normal); // 更新法线方向
        }
        isScaleChanged = false; // 重置缩放变化标志
        this->computeAABB();    // 更新AABB包围盒
    }

    // 只有在有变化时才更新顶点数据
    if (needUpdate)
    {
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, this->vertices.size() * sizeof(Vertex), this->vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑缓冲区
    }
}
