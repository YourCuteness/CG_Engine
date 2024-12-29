#include <iostream>
#include <windows.h>
#include <draw/window.h>
#include <model/model.h>
#include <string>

int main()
{
    // std::string inPutString;
    // std::string pathToModel;
    // std::cout << "Please input the path of the model: (eg. C:\\bunny.obj)\n";
    // std::cin >> inPutString;
    // for (int i = 0; i < inPutString.size(); i++)
    // {
    //     if (inPutString[i] == '"')
    //     {
    //         continue;
    //     }
    //     else
    //     {
    //         pathToModel += inPutString[i];
    //     }
    // }

    std::string pathToModel = "C:\\Users\\22436\\Desktop\\ZJU_CG\\material\\models\\body.obj";

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