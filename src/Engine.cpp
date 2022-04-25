#include "Engine.h"

#include "Parameter.h"

Engine::Engine(const char* title, int width, int height):renderer(title, width, height) {
    window = renderer.getWindow();

    glGenQueries(1, &queryBuffer);

    // callback for resolution changed.
    glfwSetFramebufferSizeCallback(window, engine_resoultion_callback);

    objs.push_back(new FireFliesObject());
    objs.push_back(new SimpleObject("lightHouse"));
    SimpleObject& obj = *(SimpleObject*)(objs.back());
    obj.setModel(new Model(pathMng.getAbsPath("model\\lighthouse\\uploads_files_2841766_Lighthouse.obj")));
    std::cout << "Texture loaded\n";
    for (Texture texture : obj.getModel()->textures_loaded) {
        std::cout << "Texture : " << texture.type << "\n";
        std::cout << "      at \"" << texture.path << "\"\n";
    }
    obj.setShader(new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\default.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\default.fs"));
    obj.setShadowShader(new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\default_shadow.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.fs"));
    obj.pos = glm::vec3(5.2f, -2.6f, 10.5f);
    obj.scale = 0.5f;
    obj.make_shadow = true;

    ComputeShader::printWorkgrouInfo();

    // add sun
    sun = new Sun(glm::vec3(-2.0f, 4.0f, -1.0f), 4096, width, height);

    // Load Tessellation shader
    auto ptr_shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.fs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.tcs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.tes",nullptr);

    auto ptr_grass_shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\grass.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\grass.fs");
    auto ptr_water_shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.fs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.tcs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.tes",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.gs");
    auto ptr_shadow_shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.fs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.tcs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.tes", nullptr);
    objs.push_back(new TerrainObject(0.065231*8.0f, ptr_shader, ptr_shadow_shader, ptr_grass_shader,ptr_water_shader));

    pathMng.PrintCWD();
}

void Engine::Loop() {
    std::cout << "RENDER START\n";
    mainCam->UpdateMat();
    while (!glfwWindowShouldClose(window)) {
        // Time update
        timeMng.updateTime();

        // GUI PRE-RENDER
        ImGuiIO& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard || io.WantCaptureMouse || bBenchmark)) {
            engine_input_handler(window);
        }
        if (!bBenchmark && bDrawGUI) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            {
                ImGui::Begin("Graphic setting");                          // Create a window called "Hello, world!" and append into it.

                if (ImGui::SliderInt("Ground tessLvl", &TESS_GROUND_LEVEL, 1, 64) ||
                    ImGui::SliderInt("Grass tessLvl", &INSTANCE_GRASS_LEVEL, 32, 1024) ||
                    ImGui::SliderInt("Water tessLvl", &TESS_WATER_LEVEL, 1, 64)) {
                    bPrameterChange = true;
                }
                if (ImGui::SliderFloat("shadowBlur jit", &SHADOW_BLUR_JITTER, 0.0f, 5.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("shadowBlur area", &SHADOW_BLUR_AREA, 0.1f, 10.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("gamma", &GAMMA, 0.0f, 3.0f, "%.3f", 1.0f)) {
                    bPrameterChange = true;
                }
                ImGui::Text("SSR setting");
                if (ImGui::InputFloat("ssr_maxDistance", &SSR_DISTANCE) ||
                    ImGui::InputFloat("ssr_resolution", &SSR_RESOLUTION) ||
                    ImGui::InputInt("ssr_lin_steps", &SSR_LIN_STEPS) ||
                    ImGui::InputInt("ssr_bin_steps", &SSR_BIN_STEPS) ||
                    ImGui::InputFloat("ssr_thickness", &SSR_THICKNESS)) {
                    bPrameterChange = true;
                }
                ImGui::ColorEdit3("clear color", (float*)&(renderer.clearColor[0])); // Edit 3 floats representing a color
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }
            {
                ImGui::Begin("Terrain setting");
                if (ImGui::SliderFloat("uvFactorRock", &LAND_ROCK_UV_FACTOR, 1.0f, 1000.f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("uvFactorGrass", &LAND_GRASS_UV_FACTOR, 1.0f, 1000.f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("uvFactorWater", &WATER_UV_FACTOR, 1.0f, 1000.f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("max_height", &LAND_HEIGHT, 0.0f, 100.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("waterSize", &WATER_SIZE, 1.0f, 8.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("waterLevel", &WATER_LEVEL, -0.5f, LAND_HEIGHT, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("water waveLength", &WATER_WAVE_LENGTH, 1.0f, 100.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("water steepness", &WATER_WAVE_STEEPENSS, 0.1f, 1.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("water transparency", &WATER_TRANSPARENCY, 0.1f, 10.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("water time speed", &WATER_TIME_FACTOR, 0.0f, 1.0f) ||
                    ImGui::SliderFloat("grass waveLength", &GRASS_WAVE_LENGTH, 1.0f, 100.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("grass steepness", &GRASS_WAVE_STEEPNESS, 0.0f, 0.5f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("grass probability", &GRASS_DENSITY, 0.0f, 1.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("grass size", &GRASS_SIZE, 0.0f, 5.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat("grass gen crit", &GRASS_SLOPE_CRITERION, 0.7f, 1.0f, "%.3f", 1.0f) ||
                    ImGui::SliderFloat3("sun dir", &(sun->lightDir[0]), -1.0f, 1.0f, "%.3f", 1.0f)) {
                    bPrameterChange = true;

                }
                ImGui::End();
            }
            {
                ImGui::Begin("Control setting");
                ImGui::SliderFloat("move speed", &CAM_MOVE_SPEED, 1.0f, 100.0f, "%.3f", 1.0f);
                ImGui::SliderFloat("rotation speed", &CAM_ROT_SPEED, 0.01f, 1.0f, "%.3f", 1.0f);
                if (ImGui::SliderFloat("time speed", &TIME_SPEED, -3.0f, 3.0f, "%.3f", 1.0f)) {
                    timeMng.setSpeed((double)TIME_SPEED);
                }
                if (ImGui::ColorEdit3("Sun color", &(sun->color[0])) ||
                    ImGui::InputFloat("Sun strength", &sun->lightStrength)) {
                    bPrameterChange = true;
                }
                ImGui::End();
            }
            {
                ImGui::Begin("Shadow Map");
                ImGui::Image((ImTextureID)(sun->depthMap), ImVec2(400, 400), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::End();
            }
            {
                ImGui::Begin("Render path");
                ImGui::Text("Before lighting pass");
                ImGui::Image((ImTextureID)(renderer.defferedPIPE->gAlbedoAO), ImVec2(480, 270), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::Text("Before SSR PIPE");
                //ImGui::Image((ImTextureID)(renderer.ssrPIPE->texture_color), ImVec2(480, 270), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::Text("HDR Bloom filter - blurred");
                ImGui::Image((ImTextureID)(renderer.hdrPIPE->blurredTexture), ImVec2(480, 270), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::Text("Before tone mapping");
                ImGui::Image((ImTextureID)(renderer.hdrPIPE->colorTexture), ImVec2(480, 270), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::End();
            }
            {
                ImGui::Begin("Obj transform");
                for (auto iter = objs.begin(); iter != objs.end(); iter++) {
                    BaseObject* ptr_obj = *iter;
                    if (ptr_obj->getClassID() == ObjClass::Simple) {
                        SimpleObject& obj = *(SimpleObject*)ptr_obj;
                        ImGui::Text(obj.name.c_str());
                        if (ImGui::SliderFloat3("Pos", &obj.pos[0], -LANDSIZE / 2.0f, LANDSIZE / 2.0f, "%.3f", 1.0f) ||
                            ImGui::InputFloat("scale", &obj.scale)) {
                            bPrameterChange = true;
                        }
                    }
                }
                ImGui::Text("Firefly");
                if (ImGui::Checkbox("Draw fireflies ?", &bDrawFireflies))
                    bPrameterChange = true;
                ImGui::End();
            }
            {
                ImGui::Begin("BenchMark");
                ImGui::Text("Bench Time : %.2f sec", benchTime);
                ImGui::Text("Total Frame : %d", totalFrame);
                ImGui::Text("Mean FPS : %.3f fps/sec", (float)totalFrame/benchTime);
                ImGui::Text("Total Render Time : %.2f ms", totalRenderTime/(float)totalFrame);
                ImGui::End();
            }
        }
        
        if (bBenchmark) {
            float benchTimeElapsed = timeMng.getRealTime() - benchStart;
            mainCam->target = glm::vec3(0.0, 0.0, 0.0);
            mainCam->theta = 45.0f;
            mainCam->phi = 360.0f * benchTimeElapsed / benchTime;
            mainCam->UpdateRot();
            mainCam->UpdateMat();
            bBenchmark = !(benchTimeElapsed > benchTime);
        }

        // Generate shadeow map
        sun->Begin();
        for (auto iter = objs.begin(); iter != objs.end(); iter++) {
            BaseObject* ptr_obj = *iter;
            if (ptr_obj->getClassID() == ObjClass::Simple) {
                SimpleObject& simpleObj = *(SimpleObject*)ptr_obj;
                if (simpleObj.make_shadow)
                    simpleObj.shadowDraw();
            }
            else if (ptr_obj->getClassID() == ObjClass::Terrain) {
                TerrainObject& terrain = *(TerrainObject*)ptr_obj;
                terrain.shadowDraw();
            }
        }
        sun->End();

        // Renderer draw
        renderer.StartFrameRender();

        //std::cout << "STG 0\n";
        if (bWireMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        // object rendering
        for (auto iter = objs.begin(); iter != objs.end(); iter++) {
            BaseObject* ptr_obj = *iter;
            if (ptr_obj->getClassID() == ObjClass::Simple) {
                SimpleObject& obj = *(SimpleObject*)ptr_obj;
                obj.draw();
            }
            else if (ptr_obj->getClassID() == ObjClass::Terrain) {
                TerrainObject& terrain = *(TerrainObject*)ptr_obj;
                //float clk = (float)clock() * 0.0001f;
                //glm::vec3 res(cos(clk), 0.2f, -sin(clk));;
                //sunLightDir = glm::normalize(res);
                //sunLightDir = glm::normalize(sunLightDir+glm::vec3(0.0f,0.5f,0.0f));
                terrain.draw();
            }else if (ptr_obj->getClassID() == ObjClass::FireFlies) {
                FireFliesObject& obj = *(FireFliesObject*)ptr_obj;
                if(bDrawFireflies)
                    obj.draw();
            }
        }
        if (bWireMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        //std::cout << "STG 1\n";

        // skybox rendering
        renderer.drawSkybox();
        //std::cout << "STG 2\n";
        // post process
        renderer.postProcess();
        //std::cout << "STG 3\n";
        // GUI render on opengl screen
        if (!bBenchmark && bDrawGUI) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        renderer.endFrameRender();
        //std::cout << "STG 4\n";
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void engine_resoultion_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    mainCam->aspect = ((float)width / (float)height);
    std::cout << "<" << width << ", " << height << ">" << std::endl;
}


void engine_input_handler(GLFWwindow* window) {
    static bool   mouse_dragged = false;
    static double mouse_xpos, mouse_ypos;
    static double prev_xpos = 0.0, prev_ypos = 0.0;

    static double lastPPress = 0.0;
    double now = timeMng.getRealTime();
    float dt = timeMng.getRealDelta();

    //Keyboard handle
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    bool isWPressed = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (isWPressed && (now- lastPPress)>0.05) {
        if (bWireMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        bWireMode = !bWireMode;
        lastPPress = now;
    }else if(isWPressed &&(now - lastPPress) <= 0.05) {
        lastPPress = now;
    }

    isWPressed = glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS;
    if (isWPressed && (now - lastPPress) > 0.05) {
        bDrawGUI = !bDrawGUI;
        lastPPress = now;
    }
    else if (isWPressed && (now - lastPPress) <= 0.05) {
        lastPPress = now;
    }

    isWPressed = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;
    if (isWPressed && (now - lastPPress) > 6.0f) {
        bBenchmark = true;
        lastPPress = now;
        benchStart = timeMng.getRealTime();
        totalFrame = 0;
        totalRenderTime = 0.0;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 xyProjection = mainCam->front - glm::vec3(0.0f, mainCam->front.y, 0.0f);
        if (glm::length(xyProjection) > 1e-3) {
            mainCam->target += dt * CAM_MOVE_SPEED * glm::normalize(mainCam->front - glm::vec3(0.0f, mainCam->front.y, 0.0f));
            mainCam->UpdatePos();
            mainCam->UpdateMat();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm::vec3 xyProjection = mainCam->front - glm::vec3(0.0f, mainCam->front.y, 0.0f);
        if (glm::length(xyProjection) > 1e-3) {
            mainCam->target -= dt * CAM_MOVE_SPEED * glm::normalize(mainCam->front - glm::vec3(0.0f, mainCam->front.y, 0.0f));
            mainCam->UpdatePos();
            mainCam->UpdateMat();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        mainCam->target += dt * CAM_MOVE_SPEED * mainCam->right;
        mainCam->UpdatePos();
        mainCam->UpdateMat();
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        mainCam->target -= dt * CAM_MOVE_SPEED * mainCam->right;
        mainCam->UpdatePos();
        mainCam->UpdateMat();
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        mainCam->target += glm::vec3(0.0f, dt * CAM_MOVE_SPEED, 0.0f);
        mainCam->UpdatePos();
        mainCam->UpdateMat();
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        mainCam->target += glm::vec3(0.0f, -dt * CAM_MOVE_SPEED, 0.0f);
        mainCam->UpdatePos();
        mainCam->UpdateMat();
    }


    // Mouse handle
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)==GLFW_PRESS) {
        if (!mouse_dragged) {
            glfwGetCursorPos(window, &prev_xpos, &prev_ypos);
            mouse_dragged = true;
        }
        glfwGetCursorPos(window, &mouse_xpos, &mouse_ypos);
        mainCam->phi -= CAM_ROT_SPEED * (float)(mouse_xpos - prev_xpos);
        mainCam->theta -= CAM_ROT_SPEED * (float)(mouse_ypos - prev_ypos);
        prev_xpos = mouse_xpos;
        prev_ypos = mouse_ypos;
        mainCam->UpdateRot();
        mainCam->UpdateMat();
    }
    else {
        mouse_dragged = false;
    }
}