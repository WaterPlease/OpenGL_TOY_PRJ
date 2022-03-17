#include "Renderer.h"

bool wireMode = false;

Renderer::Renderer(const char* title, int width, int height):screenRes(width,height) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1920,1080, title, glfwGetPrimaryMonitor(), NULL);
    //window = glfwCreateWindow(screenRes.x, screenRes.y, title, NULL, NULL);
    if (window == NULL)
    {
        logger.Log(__FUNCTION__, __LINE__, "Failed to create GLFW window");
        glfwTerminate();
        exit(ERROR_APP_INIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logger.Log(__FUNCTION__, __LINE__, "Failed to initialize GLAD");
        exit(ERROR_APP_INIT_FAILURE);
    }

    glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, 1920, 1080, 144);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);


    mainCam = new Camera();
    mainCam->aspect = (float)(this->screenRes.x) / (float)(this->screenRes.y);
    mainCam->fovy = 45.0f;// glm::radians(71.0f);
    mainCam->zNear = 0.1f;
    mainCam->zFar = 1000.0f;
    mainCam->pos = glm::vec3(6.0f, 6.0f, 6.0f);
    mainCam->updateRot(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 2.0f, -1.0f));
    mainCam->calcViewCone();
}