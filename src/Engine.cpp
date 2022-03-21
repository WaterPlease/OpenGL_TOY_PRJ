#include "Engine.h"

#include "Parameter.h"

Engine::Engine(const char* title, int width, int height):renderer(title, width, height) {
    window = renderer.getWindow();

    // callback for resolution changed.
    glfwSetFramebufferSizeCallback(window, engine_resoultion_callback);

    // Debug area
    /*
    objs.push_back(new SimpleObject());
    SimpleObject& obj = *(SimpleObject*)(objs.back());

    obj.setModel(new Model("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\model\\Intergalactic_Spaceship-(Wavefront).obj"));

    obj.setShader(new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\default.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\default.fs",nullptr,nullptr));
    */

    // add sun
    sun = new Sun(glm::vec3(-2.0f, 4.0f, -1.0f), 1024, width, height);

    // Load Tessellation shader
    auto ptr_shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.fs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.tcs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain.tes",nullptr);

    auto ptr_grass_shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\grass.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\grass.fs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\grass.tcs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\grass.tes",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\grass.gs");
    auto ptr_water_shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.fs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.tcs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.tes",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\water.gs");
    auto ptr_shadow_shader = new Shader("C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.vs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.fs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.tcs",
        "C:\\Users\\kwonh\\Desktop\\study\\Graphics\\OpenGL_TOY_PRJ\\shader\\terrain_shadow.tes", nullptr);
    objs.push_back(new TerrainObject(0.5f,100.0,ptr_shader, ptr_shadow_shader, ptr_grass_shader,ptr_water_shader));
    //objs.push_back(new TerrainObject(0, 0, 10.0f));
}

void Engine::loop() {
    while (!glfwWindowShouldClose(window)) {
        ImGuiIO& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard || io.WantCaptureMouse)) {
            engine_input_handler(window);
        }
        // Generate shadeow map
        sun->begin();
        for (auto iter = objs.begin(); iter != objs.end(); iter++) {
            BaseObject* ptr_obj = *iter;
            if (ptr_obj->getClassID() == ObjClass::Simple) {
                continue;
            }
            else if (ptr_obj->getClassID() == ObjClass::Terrain) {
                TerrainObject& terrain = *(TerrainObject*)ptr_obj;
                terrain.shadowDraw();
            }
        }
        sun->end();

        // Renderer draw
        renderer.startFrameRender();

        // GUI PRE-RENDER
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Graphic setting");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

            if (ImGui::SliderInt("Ground tessLvl", &GROUND_TESS_LEVEL, 1, 64)) {            // Edit 1 float using a slider from 0.0f to 1.0f
                parameter_changed = true;
            }
            if (ImGui::SliderInt("Grass tessLvl", &GRASS_TESS_LEVEL, 1, 64)) {            // Edit 1 float using a slider from 0.0f to 1.0f
                parameter_changed = true;
            }
            if (ImGui::InputFloat("uvFactorRock", &uvFactorRock)   ||
                ImGui::InputFloat("uvFactorGrass", &uvFactorGrass) ||
                ImGui::InputFloat("uvFactorWater", &uvFactorWater)) {
                parameter_changed = true;
            }
            if (ImGui::SliderFloat("max_height",&max_height,0.0f,100.0f,"%.3f",1.0f) ||
                ImGui::SliderFloat("waterLevel", &waterLevel, -0.5f, max_height, "%.3f", 1.0f)) {
                parameter_changed = true;
            }
            if (ImGui::SliderFloat("water waveLength", &water_waveLength, 1.0f, 100.0f, "%.3f", 1.0f) ||
                ImGui::SliderFloat("water steepness", &water_steepness, 0.1f, 1.0f, "%.3f", 1.0f)) {
                parameter_changed = true;
            }
            if (ImGui::SliderFloat("grass waveLength", &grass_waveLength, 1.0f, 100.0f, "%.3f", 1.0f) ||
                ImGui::SliderFloat("grass steepness", &grass_steepness, 0.0f, 0.5f, "%.3f", 1.0f)) {
                parameter_changed = true;
            }
            ImGui::SliderFloat("waterLambda", &waterLambda, 0.0f, 1.0f);

            ImGui::ColorEdit3("clear color", (float*)&(renderer.clearColor[0])); // Edit 3 floats representing a color
            if (ImGui::SliderFloat3("sun dir", &(sun->lightDir[0]), -1.0f, 1.0f, "%.3f", 1.0f)) {
                //sun->lightDir = glm::normalize(sun->lightDir);
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        {
            ImGui::Begin("Control setting");
            ImGui::SliderFloat("move speed", &speed, 0.01f, 0.1f, "%.3f", 1.0f);
            ImGui::SliderFloat("rotation speed", &rotSpeed, 0.01f, 1.0f, "%.3f", 1.0f);
            ImGui::End();
        }
        {
            ImGui::Begin("Shadow Map");
            ImGui::Image((ImTextureID)(sun->depthMap), ImVec2(400, 400),ImVec2(0,1),ImVec2(1,0));
            ImGui::End();
        }


        // object rendering
        for (auto iter = objs.begin(); iter != objs.end(); iter++) {
            BaseObject* ptr_obj = *iter;
            if (ptr_obj->getClassID() == ObjClass::Simple) {
                SimpleObject& obj = *(SimpleObject*)ptr_obj;
                obj.getShader()->use();

                // view/projection transformations
                glm::mat4 projection = mainCam->getPerspectiveMat();
                glm::mat4 view = mainCam->getViewMat();
                obj.getShader()->setMat4("projection", projection);
                obj.getShader()->setMat4("view", view);

                // render the loaded model
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
                model = glm::rotate(model, glm::radians((float)((clock() / 10) % 360)), glm::vec3(0.0, 1.0, 0.0));
                model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
                obj.getShader()->setMat4("model", model);
                renderer.draw(*ptr_obj);
            }
            else if (ptr_obj->getClassID() == ObjClass::Terrain) {
                TerrainObject& terrain = *(TerrainObject*)ptr_obj;
                //float clk = (float)clock() * 0.0001f;
                //glm::vec3 res(cos(clk), 0.2f, -sin(clk));;
                //sunLightDir = glm::normalize(res);
                //sunLightDir = glm::normalize(sunLightDir+glm::vec3(0.0f,0.5f,0.0f));
                terrain.draw();
            }
        }

        // GUI render on opengl screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        renderer.endFrameRender();

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
    static clock_t lastPPress = 0;
    static clock_t prevTime = 0;

    clock_t now = clock();
    clock_t dt = now - prevTime;
    prevTime = now;

    //Keyboard handle
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    bool isWPressed = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (isWPressed && (now- lastPPress)>50) {
        if (wireMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        wireMode = !wireMode;
        lastPPress = now;
    }else if(isWPressed &&(now - lastPPress) <= 50) {
        lastPPress = now;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm::vec3 xyProjection = mainCam->front - glm::vec3(0.0f, mainCam->front.y, 0.0f);
        if (glm::length(xyProjection) > 1e-3) {
            mainCam->target += dt * speed * glm::normalize(mainCam->front - glm::vec3(0.0f, mainCam->front.y, 0.0f));
            mainCam->updatePos();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm::vec3 xyProjection = mainCam->front - glm::vec3(0.0f, mainCam->front.y, 0.0f);
        if (glm::length(xyProjection) > 1e-3) {
            mainCam->target -= dt * speed * glm::normalize(mainCam->front - glm::vec3(0.0f, mainCam->front.y, 0.0f));
            mainCam->updatePos();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        mainCam->target += dt * speed * mainCam->right;
        mainCam->updatePos();
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        mainCam->target -= dt * speed * mainCam->right;
        mainCam->updatePos();
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        mainCam->target += glm::vec3(0.0f, dt * speed, 0.0f);
        mainCam->updatePos();
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        mainCam->target += glm::vec3(0.0f, -dt * speed, 0.0f);
        mainCam->updatePos();
    }


    // Mouse handle
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)==GLFW_PRESS) {
        if (!mouse_dragged) {
            glfwGetCursorPos(window, &prev_xpos, &prev_ypos);
            mouse_dragged = true;
        }
        glfwGetCursorPos(window, &mouse_xpos, &mouse_ypos);
        mainCam->phi -= rotSpeed * (float)(mouse_xpos - prev_xpos);
        mainCam->theta -= rotSpeed * (float)(mouse_ypos - prev_ypos);
        prev_xpos = mouse_xpos;
        prev_ypos = mouse_ypos;
        mainCam->updateRot();
    }
    else {
        mouse_dragged = false;
    }
}