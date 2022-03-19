#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"

int main()
{
    auto app = Engine("OpenGL Project",1280,720);

    app.loop();

    return 0;
}