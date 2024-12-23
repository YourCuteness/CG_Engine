#version 330 core
out vec4 FragColor;


in vec3 Normal;
in vec3 FragPos;

void main()
{
    // 法线方向作为颜色输出
    vec3 norm = normalize(Normal);  // 确保法线是单位向量
    FragColor = vec4(norm * 0.5 + 0.5, 1.0);  // 将法线映射到 [0,1] 范围
}
