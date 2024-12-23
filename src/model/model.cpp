#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <algorithm> // for std::find_if
#include <iterator>  // for std::distance

class Model
{
public:
    bool loadOBJ(const std::string &filePath);

private:
    void processFace(const std::vector<std::string> &face,
                     const std::vector<glm::vec3> &positions,
                     const std::vector<glm::vec3> &normals,
                     const std::vector<glm::vec2> &texCoords);

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;

        Vertex(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &tex)
            : position(pos), normal(norm), texCoord(tex) {}
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

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
