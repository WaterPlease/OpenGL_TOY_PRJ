#include "Renderer.h"

const char* gui_glsl_version = "#version 330";

Renderer::Renderer(const char* title, int width, int height):screenRes(width,height) {
    // Initialize member of Renderer
    clearColor = glm::vec4(0.29f, 0.66f, 0.85f, 1.0f);

    // Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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
    mainCam->zFar = LANDSIZE*1.5f;
    
    mainCam->target = glm::vec3(0.0f,5.0f,0.0f);
    mainCam->rad = 50.0f;
    mainCam->theta = 45.0f;
    mainCam->phi   = 45.0f;
    mainCam->UpdateRot();
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
    GL_TEXTURE_SKYBOX = skyboxTexture;
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
    ssrPIPE->gBufferBind(defferedPIPE->gPositionMetal, defferedPIPE->gNormalRough);
    ssrPIPE->skyboxBind(skyboxTexture);
}

void Renderer::drawSkybox() {
    glDepthFunc(GL_LEQUAL);
    skyboxShader->use();

    skyboxShader->setVec3("camPos", mainCam->pos);
    skyboxShader->setMat4("projection", mainCam->GetPerspectiveMat());
    skyboxShader->setMat4("view", glm::mat4(glm::mat3(mainCam->GetViewMat())));

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(skyboxShader->ID, "skybox"), 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

HDRPIPE::HDRPIPE(const glm::uvec2& _screenRes, GLuint nextPIPEFBO) :RenderPIPE(), nextFBO(nextPIPEFBO), screenRes(_screenRes) {

    Begin();

    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenRes.x, screenRes.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                             // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    // bloom init
    glGenTextures(1, &extractTexture);
    glBindTexture(GL_TEXTURE_2D, extractTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenTextures(1, &blurredTexture);
    glBindTexture(GL_TEXTURE_2D, blurredTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    extractShader = new ComputeShader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\extract.cs");
    blurShader = new ComputeShader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\blur.cs");


    glGenVertexArrays(1, &quadVAO);
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    std::cout << "HDR SHADER\n";
    hdrShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\postProcess.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\hdrShader.fs");
    std::cout << "HDR SHADER DONE\n";
}

inline void HDRPIPE::Begin() {
    RenderPIPE::Begin();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void HDRPIPE::End() {
    glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);

    extractShader->use();
    glBindImageTexture(0, colorTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(1, extractTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute(screenRes.x / 32, screenRes.y / 18, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    blurShader->use();
    glBindImageTexture(0, extractTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(1, blurredTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute(screenRes.x / 32, screenRes.y / 18, 1);

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    hdrShader->use();
    if (bPrameterChange) {
        hdrShader->setFloat("gamma", GAMMA);
    }

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(hdrShader->ID, "screenTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(hdrShader->ID, "bloomTexture"), 1);
    glBindTexture(GL_TEXTURE_2D, blurredTexture);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

//alpha value of albedo buffer is specular value.

inline DEFFEREDPIPE::DEFFEREDPIPE(const glm::uvec2& screenRes, GLuint nextPIPEFBO) :RenderPIPE(), nextFBO(nextPIPEFBO) {

    Begin();

    glGenTextures(1, &gPositionMetal);
    glBindTexture(GL_TEXTURE_2D, gPositionMetal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionMetal, 0);

    glGenTextures(1, &gNormalRough);
    glBindTexture(GL_TEXTURE_2D, gNormalRough);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalRough, 0);

    glGenTextures(1, &gAlbedoAO);
    glBindTexture(GL_TEXTURE_2D, gAlbedoAO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoAO, 0);

    unsigned int attachments[] = { GL_COLOR_ATTACHMENT0 ,GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenRes.x, screenRes.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                             // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    lightingShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\postProcess.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\deffered.fs");

    gBuffer_position = gPositionMetal;
    gBuffer_Normal = gNormalRough;
    gBuffer_Albedo = gAlbedoAO;


    // frustums
    float& zFar = mainCam->zFar;
    float& zNear = mainCam->zNear;
    const float halfH = zFar * tanf(0.5f * mainCam->fovy);
    const float halfW = halfH * mainCam->aspect;
    const glm::vec3 centerFarPlane = glm::vec3(0.0f, 0.0f, -1.0f) * zFar;
    for (int i = 0; i <= NUM_X_AXIS_TILE; i++) {
        auto planeNormal = glm::normalize(glm::cross(
            centerFarPlane - (halfW - i/(float)NUM_X_AXIS_TILE*2.0f * halfW) * glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)));
        xPlanes[i].x = planeNormal.x;
        xPlanes[i].y = planeNormal.y;
        xPlanes[i].z = planeNormal.z;
    }
    for (int i = 0; i <= NUM_Y_AXIS_TILE; i++) {
        auto planeNormal = glm::normalize(glm::cross(
            glm::vec3(1.0f, 0.0f, 0.0f),
            centerFarPlane - (halfH - i /(float)NUM_Y_AXIS_TILE * 2.0f * halfH) * glm::vec3(0.0f, 1.0f, 0.0f)
        ));
        yPlanes[i].x = planeNormal.x;
        yPlanes[i].y = planeNormal.y;
        yPlanes[i].z = planeNormal.z;
    }
    // Light index
    glGenBuffers(1, &SSBO_lightIndex);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_lightIndex);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(lightIndex), nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, SSBO_lightIndex);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    gl_SSBO_LightIndex = SSBO_lightIndex;

    lightCullShader = new ComputeShader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\lightCulling.cs");
    std::cout << "DEFERRED DONE\n";
}

inline void DEFFEREDPIPE::Begin() {
    RenderPIPE::Begin();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DEFFEREDPIPE::End() {
    glBeginQuery(GL_TIME_ELAPSED, queryBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_SSBO_LightIndex);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &ZERO);
    glMemoryBarrier(GL_ALL_BARRIER_BITS | GL_SHADER_STORAGE_BARRIER_BIT);

    lightCullShader->use();
    lightCullShader->setFloat("zFar", mainCam->zFar);
    lightCullShader->setMat4("view", mainCam->GetViewMat());
    glUniform3fv(glGetUniformLocation(lightCullShader->ID, "xPlanes"), NUM_X_AXIS_TILE+1,(const GLfloat*)xPlanes);
    glUniform3fv(glGetUniformLocation(lightCullShader->ID, "yPlanes"), NUM_Y_AXIS_TILE+1,(const GLfloat*)yPlanes);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_SSBO_LightIndex);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl_SSBO_LightIndex);
    GLuint block_index0 = 
        glGetProgramResourceIndex(lightCullShader->ID, GL_SHADER_STORAGE_BLOCK, "lightIndex");
    GLuint ssbo_binding_point_index = 4;
    glShaderStorageBlockBinding(lightCullShader->ID, block_index0, ssbo_binding_point_index);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_SSBO_FLY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl_SSBO_FLY);
    block_index0 = 
        glGetProgramResourceIndex(lightCullShader->ID, GL_SHADER_STORAGE_BLOCK, "flyInfo");
    ssbo_binding_point_index = 3;
    glShaderStorageBlockBinding(lightCullShader->ID, block_index0, ssbo_binding_point_index);

    glDispatchCompute(NUM_X_AXIS_TILE, NUM_Y_AXIS_TILE, NUM_Z_AXIS_TILE);

    glMemoryBarrier(GL_ALL_BARRIER_BITS | GL_SHADER_STORAGE_BARRIER_BIT);

    lightingShader->use();
    if (bPrameterChange) {
        lightingShader->setFloat("shadowBlurJitter", SHADOW_BLUR_JITTER);
        lightingShader->setFloat("shadowBlurArea", SHADOW_BLUR_AREA);
        lightingShader->setVec3("lightColor", sun->color);
        lightingShader->setFloat("sunStrength", sun->lightStrength);
        lightingShader->setFloat("zFar", mainCam->zFar);
        lightingShader->setBool("bClusterDraw", bClusterDraw);
    }
    lightingShader->setVec3("sunDir", glm::vec3(mainCam->GetViewMat() * glm::vec4(sun->lightDir, 0.0)));
    lightingShader->setVec3("viewPos", mainCam->pos);
    lightingShader->setMat4("view", mainCam->GetViewMat());
    lightingShader->setMat4("inverseViewMat", glm::inverse(mainCam->GetViewMat()));
    lightingShader->setMat4("lightSpaceMat", sun->lightSpaceMat(LANDSIZE));;
    lightingShader->setBool("drawFireflies", bDrawFireflies);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(lightingShader->ID, "gPositionMetal"), 0);
    glBindTexture(GL_TEXTURE_2D, gPositionMetal);
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(lightingShader->ID, "gNormalRough"), 1);
    glBindTexture(GL_TEXTURE_2D, gNormalRough);
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(glGetUniformLocation(lightingShader->ID, "gAlbedoAO"), 2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoAO);
    glActiveTexture(GL_TEXTURE3);
    glUniform1i(glGetUniformLocation(lightingShader->ID, "texture_shadow"), 3);
    glBindTexture(GL_TEXTURE_2D, sun->depthMap);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_SSBO_FLY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl_SSBO_FLY);
    block_index0 = 
        glGetProgramResourceIndex(lightingShader->ID, GL_SHADER_STORAGE_BLOCK, "flyInfo");
    ssbo_binding_point_index = 3;
    glShaderStorageBlockBinding(lightingShader->ID, block_index0, ssbo_binding_point_index);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_SSBO_LightIndex);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl_SSBO_LightIndex);
    block_index0 =
        glGetProgramResourceIndex(lightingShader->ID, GL_SHADER_STORAGE_BLOCK, "lightIndex");
    ssbo_binding_point_index = 4;
    glShaderStorageBlockBinding(lightingShader->ID, block_index0, ssbo_binding_point_index);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glMemoryBarrier(GL_ALL_BARRIER_BITS | GL_FRAMEBUFFER_BARRIER_BIT);

    GLuint64 timeElapsed;
    GLint queryReady=0;
    glEndQuery(GL_TIME_ELAPSED);
    if(bBenchmark) {
        glGetQueryObjectui64v(queryBuffer, GL_QUERY_RESULT, &timeElapsed);
        totalRenderTime += (float)((long double)timeElapsed / (long double)1000000.0);
        totalFrame++;
    }
}

inline SSRPIPE::SSRPIPE(const glm::uvec2& screenRes, GLuint nextPIPEFBO) :RenderPIPE(), nextFBO(nextPIPEFBO) {

    Begin();

    glGenTextures(1, &texture_color);
    glBindTexture(GL_TEXTURE_2D, texture_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenRes.x, screenRes.y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenRes.x, screenRes.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                             // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    ssrShader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\postProcess.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\ssr.fs");

    RES_X = screenRes.x;
    RES_Y = screenRes.y;
}

inline void SSRPIPE::Begin() {
    RenderPIPE::Begin();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SSRPIPE::End() {
    glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ssrShader->use();
    if (bPrameterChange) {
        ssrShader->setFloat("RES_X", RES_X);
        ssrShader->setFloat("RES_Y", RES_Y);
        ssrShader->setFloat("maxDistance", SSR_DISTANCE);
        ssrShader->setFloat("resolution", SSR_RESOLUTION);
        ssrShader->setFloat("thickness", SSR_THICKNESS);
        ssrShader->setInt("bin_steps", SSR_BIN_STEPS);
        ssrShader->setInt("lin_steps", SSR_LIN_STEPS);
        ssrShader->setVec3("lightColor", sun->color);
    }
    ssrShader->setVec3("camPos", mainCam->pos);
    ssrShader->setVec3("camFront", glm::vec3(mainCam->GetViewMat() * glm::vec4(mainCam->front, 0.0)));
    ssrShader->setMat4("Prjmat", mainCam->GetPerspectiveMat());
    ssrShader->setMat4("invViewMat", mainCam->GetInvViewMat());

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(ssrShader->ID, "image_position"), 0);
    glBindTexture(GL_TEXTURE_2D, gBufferImages[0]);
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(ssrShader->ID, "image_normal"), 1);
    glBindTexture(GL_TEXTURE_2D, gBufferImages[1]);
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(glGetUniformLocation(ssrShader->ID, "image_color"), 2);
    glBindTexture(GL_TEXTURE_2D, texture_color);
    glActiveTexture(GL_TEXTURE3);
    glUniform1i(glGetUniformLocation(ssrShader->ID, "texture_skybox"), 3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_skybox);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


