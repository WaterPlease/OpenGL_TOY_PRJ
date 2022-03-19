#include "Renderer.h"

bool wireMode = false;
const char* gui_glsl_version = "#version 330";

Renderer::Renderer(const char* title, int width, int height):screenRes(width,height) {
    // Initialize member of Renderer
    clearColor = glm::vec4(0.29f, 0.66f, 0.85f, 1.0f);

    // Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //window = glfwCreateWindow(1920,1080, title, glfwGetPrimaryMonitor(), NULL);
    window = glfwCreateWindow(screenRes.x, screenRes.y, title, NULL, NULL);
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

    //glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, 1920, 1080, 144);

    // GUI initialization.

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(gui_glsl_version);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_3D);


    mainCam = new Camera();
    mainCam->aspect = (float)(this->screenRes.x) / (float)(this->screenRes.y);
    mainCam->fovy = 45.0f;// glm::radians(71.0f);
    mainCam->zNear = 0.1f;
    mainCam->zFar = 1000.0f;
    
    mainCam->target = glm::vec3(0.0f,5.0f,0.0f);
    mainCam->rad = 10.0f;
    mainCam->theta = 45.0f;
    mainCam->phi   = 45.0f;
    mainCam->updateRot();
    mainCam->calcViewCone();
}