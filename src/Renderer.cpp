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

    // GUI initialization.

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(gui_glsl_version);


    glEnable(GL_DEPTH_TEST);
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

    std::vector<std::string> cubeMapPaths{
        "Daylight Box_Right.bmp",
        "Daylight Box_Left.bmp",
        "Daylight Box_Top.bmp",
        "Daylight Box_Bottom.bmp",
        "Daylight Box_Front.bmp",
        "Daylight Box_Back.bmp",
    };
    skyboxTexture = TextureCubeFromFile(cubeMapPaths,
        std::string("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\x64\\Debug\\model\\skybox"));
    texture_skybox = skyboxTexture;
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    skyboxShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\skybox.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\skybox.fs");
    
    hdrPIPE = new HDRPIPE(this->screenRes, 0);
    ssrPIPE = new SSRPIPE(this->screenRes, hdrPIPE->fbo);
    defferedPIPE = new DEFFEREDPIPE(this->screenRes,ssrPIPE->fbo);
    //defferedPIPE = new DEFFEREDPIPE(this->screenRes, hdrPIPE->fbo);

    // PIPELINE binding
    ssrPIPE->gBufferBind(defferedPIPE->gPosition, defferedPIPE->gNormal);
}
