#include <iostream>
#include <windows.h>
#include <draw/window.h>
#include <model/model.h>
#include <string>

int main()
{
    Window window(1600, 1200, "Example");
    window.run();
    return 0;
}