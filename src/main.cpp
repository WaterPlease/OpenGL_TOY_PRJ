#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"

int main()
{
    auto app = Engine("OpenGL Project", screen_x, screen_y);

    app.loop();

    return 0;
}