#include "Engine.h"

#include "Parameter.h"

Engine::Engine(const char* title):renderer(title) {
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
    objs.push_back(new TerrainObject(0.5f,100.0,ptr_shader, ptr_grass_shader));
    //objs.push_back(new TerrainObject(0, 0, 10.0f));

    // PARAMETER INIT
    TESS_LEVEL = 24;
}

void Engine::loop() {
    while (!glfwWindowShouldClose(window)) {
        engine_input_handler(window);

        // Renderer draw
        renderer.startFrameRender();
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
                clock_t clk = clock();
                clk = ((clk % 10800));
                float deg = glm::radians(((float)clk) * 0.033f);
                terrain.lightDir = glm::normalize(glm::vec3(std::sin(deg)* std::sin(deg), std::cos(deg), std::sin(deg)*std::cos(deg)));
                terrain.draw();
            }
        }
        renderer.endFrameRender();

        glfwPollEvents();
    }
    glfwTerminate();
}

void engine_resoultion_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    mainCam->aspect = ((float)width / (float)height);
    std::cout << "<" << width << ", " << height << ">" << std::endl;
}


clock_t lastPPress = 0;
clock_t lastADDPress = 0;
clock_t lastSUBPress = 0;

float speed = 0.01;
float rotSpeed = 0.005f;
clock_t prevTime = 0;
void engine_input_handler(GLFWwindow* window) {
    clock_t now = clock();
    clock_t dt = now - prevTime;
    prevTime = now;

    //mainCam->pos += dt * speed * glm::vec3(mainCam->front.x, 0.0, mainCam->front.z);

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

    bool isIPressed = glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS;
    if (isIPressed && (now - lastADDPress) > 50) {
        GLuint tmp = TESS_LEVEL + 2;
        if (tmp <= 64) TESS_LEVEL = tmp;
        std::cout << "Current Level : " << TESS_LEVEL << " ---- MAXIMUM LEVEL : " << 64 << "\n";
        lastADDPress = now;
    }
    else if (isIPressed && (now - lastADDPress) <= 50) {
        lastADDPress = now;
    }

    bool isKPressed = glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;
    if (isKPressed && (now - lastSUBPress) > 50) {
        GLuint tmp = TESS_LEVEL - 2;
        if (tmp > 0) TESS_LEVEL = tmp;
        std::cout << "Current Level : " << TESS_LEVEL << " ---- MAXIMUM LEVEL : " << 64 << "\n";
        lastSUBPress = now;
    }
    else if (isKPressed && (now - lastSUBPress) <= 50) {
        lastSUBPress = now;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        mainCam->pos += dt * speed * mainCam->front;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        mainCam->pos -= dt * speed * mainCam->front;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        mainCam->pos += dt * speed * mainCam->right;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        mainCam->pos -= dt * speed * mainCam->right;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), -dt * rotSpeed, mainCam->right);
        glm::vec4 _newFront = rotMat * glm::vec4(mainCam->front, 1.0f);
        glm::vec3 newFront = glm::vec3(_newFront[0], _newFront[1], _newFront[2]);
        mainCam->updateRot(newFront, mainCam->right, false);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), dt * rotSpeed, mainCam->right);
        glm::vec4 _newFront = rotMat * glm::vec4(mainCam->front, 1.0f);
        glm::vec3 newFront = glm::vec3(_newFront[0], _newFront[1], _newFront[2]);
        mainCam->updateRot(newFront, mainCam->right, false);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), dt * rotSpeed, mainCam->front);
        glm::vec4 _newUP = rotMat * glm::vec4(mainCam->up, 1.0f);
        glm::vec3 newUp = glm::vec3(_newUP[0], _newUP[1], _newUP[2]);
        mainCam->updateRot(mainCam->front, newUp, true);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), -dt * rotSpeed, mainCam->front);
        glm::vec4 _newUP = rotMat * glm::vec4(mainCam->up, 1.0f);
        glm::vec3 newUp = glm::vec3(_newUP[0], _newUP[1], _newUP[2]);
        mainCam->updateRot(mainCam->front, newUp, true);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), dt * rotSpeed, mainCam->up);
        glm::vec4 _newFront = rotMat * glm::vec4(mainCam->front, 1.0f);
        glm::vec3 newFront = glm::vec3(_newFront[0], _newFront[1], _newFront[2]);
        mainCam->updateRot(newFront, mainCam->up);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), -dt * rotSpeed, mainCam->up);
        glm::vec4 _newFront = rotMat * glm::vec4(mainCam->front, 1.0f);
        glm::vec3 newFront = glm::vec3(_newFront[0], _newFront[1], _newFront[2]);
        mainCam->updateRot(newFront, mainCam->up);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        mainCam->pos += glm::vec3(0.0f, dt * speed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        mainCam->pos += glm::vec3(0.0f, -dt * speed, 0.0f);
    }
}