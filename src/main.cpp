#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"

int main()
{
    auto app = Engine("OpenGL Project", SCREEN_WIDTH, SCREEN_HEIGHT);

    app.Loop();

    return 0;
}