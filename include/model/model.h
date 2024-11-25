#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

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

    bool loadOBJ(const std::string &filePath);

private:
    void processFace(const std::vector<std::string> &face,
                     const std::vector<glm::vec3> &positions,
                     const std::vector<glm::vec3> &normals,
                     const std::vector<glm::vec2> &texCoords);
};

#endif