#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <model/model.h>

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
            std::vector<std::string> face;
            std::string faceStr;
            while (iss >> faceStr)
            {
                face.push_back(faceStr);
            }
            processFace(face, positions, normals, texCoords);
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
        int vIndex, tIndex, nIndex;
        char separator;
        faceStream >> vIndex >> separator >> tIndex >> separator >> nIndex;

        vIndex--; // OBJ 索引从 1 开始，调整为从 0 开始
        tIndex--;
        nIndex--;

        vertices.push_back(Vertex(positions[vIndex], normals[nIndex], texCoords[tIndex]));
        indices.push_back(static_cast<unsigned int>(vertices.size()) - 1);
    }
}