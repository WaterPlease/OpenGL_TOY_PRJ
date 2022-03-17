#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"

int main()
{
    auto app = Engine("OpenGL Project");

    app.loop();

    return 0;
}