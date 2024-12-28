#include <iostream>
#include <windows.h>
#include <draw/window.h>
#include <model/model.h>
#include <string>

int main()
{
    std::string pathToModel;
    std::cout << "Please input the path of the model:";
    std::cin >> pathToModel;

    Window window(1600, 1200, "Example");

    Model model1;
    if (!model1.loadOBJ(pathToModel))
    { // 请修改为你的 OBJ 文件路径
        std::cerr << "Failed to load model1" << std::endl;
        return -1;
    }

    window.addModel(model1);
    window.run();

    system("pause");
    return 0;
}