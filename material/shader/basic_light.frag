#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 lightColor;
in vec3 lightPos;

uniform vec3 viewPos;
uniform sampler2D texture1; // 新增：纹理采样器
uniform bool useTexture;    // 是否使用纹理

void main()
{
    // 默认物体颜色
    vec3 defaultColor = vec3(0.5f, 0.5f, 0.5f);

    // 根据是否绑定纹理决定 objectColor
    vec3 objectColor;
    if (useTexture)
    {
        objectColor = texture(texture1, vec2(FragPos.x, FragPos.z)).rgb;
    }
    else
    {
        objectColor = defaultColor;
    }
    float shininess = 0.05f;

    float ambientStrength = 0.10;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = max(dot(viewDir, reflectDir), 0.0); // 镜面反射的亮度
    vec3 specular = spec * lightColor; // 镜面反射颜色

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 result = (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(result, 1.0);
}