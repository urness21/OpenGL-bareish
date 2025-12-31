#include "common.h"
#include "logic.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>


std::string loadShaderSource(const char* filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }
    std::stringstream sstr;
    sstr << fileStream.rdbuf();
    content = sstr.str();
    fileStream.close();
    return content;
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    std::string vertShaderStr = loadShaderSource("default.vert");
    std::string fragShaderStr = loadShaderSource("default.frag");
    const char* vertexShaderSource = vertShaderStr.c_str();
    const char* fragmentShaderSource = fragShaderStr.c_str();
    generateGround();
    player playerone;
    playerone.pos = glm::vec3(-3.0f, 0.0f, 0.0f);
    playerone.front = glm::vec3(0.0f, 0.0f, -1.0f);
    playerone.up = glm::vec3(0.0f, 1.0f, 0.0f);
    playerone.yaw = 0.0f;
    playerone.pitch = 0.0f;
    playerone.color = glm::vec3(1.0f, 1.0f, 1.0f);
    playerone.ammo = 30;
    playerone.health = 1.0f;
    players.push_back(playerone);
    cameraPos = playerone.pos;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    window = glfwCreateWindow(mode->width, mode->height, "OpenGL HUD", primaryMonitor, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwGetWindowSize(window, &width, &height);
    lastX = (float)width / 2.0f;
    lastY = (float)height / 2.0f;
    mouse_callback(window, lastX, lastY);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    float vertices[] = {
        -0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f, 0.5f,-0.5f, 0,0,-1,
         0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f,-0.5f,-0.5f, 0,0,-1,
        -0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f, 0.5f, 0.5f, 0,0, 1,
         0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f,-0.5f, 0.5f, 0,0, 1,
        -0.5f, 0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f,-0.5f,-1,0, 0,
        -0.5f,-0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f, 0.5f,-1,0, 0,
         0.5f, 0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f,-0.5f, 1,0, 0,
         0.5f,-0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f, 0.5f, 1,0, 0,
        -0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f, 0.5f, 0,-1,0,
         0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f,-0.5f, 0,-1,0,
        -0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f, 0.5f, 0, 1,0,
         0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f,-0.5f, 0, 1,0
    };
    unsigned int VBO, VAO, iVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &iVBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, iVBO);
    glBufferData(GL_ARRAY_BUFFER, cubes.size() * sizeof(CubeInstance), &cubes[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), (void*)0); glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(3); glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), (void*)offsetof(CubeInstance, scale)); glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(4); glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), (void*)offsetof(CubeInstance, color)); glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(5); glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), (void*)offsetof(CubeInstance, rotation)); glVertexAttribDivisor(5, 1);
    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        player& p = players[0];
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        cameraPos = p.pos;
        cameraFront = p.front;
        yaw = p.yaw;
        pitch = p.pitch;
        processInput(window);
        if (!isPaused) {
            // Physics & Cube Logic
            for (auto& cube : cubes) {
                cube.rotation += cube.rotVel * deltaTime;
                cube.colorTime += deltaTime * 5.0f;
                if (cube.chases==true) {
                    //cube.color = glm::vec3(sin(cube.colorTime) * 0.5 + 0.5, 1.0, sin(cube.colorTime * 4) * 0.5 + 0.5);
                    cube.color = glm::vec3(0.0f, 0.5f, 0.5f);
                    // Calculate direction vector (Target - Current)
                    glm::vec3 direction = p.pos - cube.pos;
                    if (glm::length(direction) > 0.1f) {
                        direction = glm::normalize(direction);
                        cube.pos += direction * enemySpeed * deltaTime;
                        cube.rotation.y = atan2(direction.x, direction.z);
                    }
                }
                else {
                    // Logic for moving cubes (bullets)
                    if (cube.health == 0.0f) {
                        cube.color = glm::vec3(sin(cube.colorTime) * 0.5 + 0.5, sin(cube.colorTime) * 0.5, sin(cube.colorTime) * 0.5 + 0.5);
                        cube.pos += cube.vel * deltaTime * 5.0f;
                        cube.scale -= 0.2f * deltaTime;
                        cube.timeAlive += 1;
                        // Collision Detection (Bullet vs Moving Collider)
                        for (auto& otherCube : cubes) {
                            if (otherCube.timeAlive < 0) {
                                float dist = glm::distance(cube.pos, otherCube.pos);
                                float radiusSum = (cube.scale / 2.0f) + (otherCube.scale / 2.0f);
                                if (dist < radiusSum) {
                                    otherCube.health -= 0.1f;
                                    cube.scale = 0.0f;
                                    totalHits++;
                                    if (otherCube.health <= 0.0f) {
                                        totalKills++;
                                    }
                                }
                            }
                        }
                    }
                    else {
                        cube.color = glm::vec3(sin(cube.colorTime) * 0.5 + 0.5, sin(4.0f*cube.colorTime) * 0.5, sin(2.0f*cube.colorTime) * 0.5 + 0.5);
                        cube.pos += cube.vel * deltaTime * 5.0f;
                    }

                }
            }
        }
        std::erase_if(cubes, [](const CubeInstance& cube) {
            bool isDeadBullet = (cube.scale <= 0.0f && cube.timeAlive >= 0);
            bool isDeadCollider = (cube.timeAlive < 0 && cube.health <= 0.0f);
            if (isDeadCollider) {
                // Generate a random position within a 10x10 area
                float rx = (rand() % 200 / 10.0f) - 10.0f;
                float ry = (rand() % 100 / 10.0f) - 5.0f;
                float rz = (rand() % 200 / 10.0f) - 10.0f;
                return true;
            }
            return isDeadBullet || isDeadCollider;
            });
        // Maintain minimum collider targets at all times
        int colliderCount = 0;
        for (auto& c : cubes) if (c.timeAlive < 0) colliderCount++;
        while (colliderCount < colliders) {
            spawnEnemyAtRadius(15.0f, 30.0f);
            colliderCount++;
        }
        glBindBuffer(GL_ARRAY_BUFFER, iVBO);
        glBufferData(GL_ARRAY_BUFFER, cubes.size() * sizeof(CubeInstance), cubes.data(), GL_DYNAMIC_DRAW);
        // Rendering
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)fbWidth / fbHeight, 0.1f, 100.0f);
        glm::mat4 view;
        if (usingSkyCamera) {
            view = glm::lookAt(p.pos + glm::vec3(0.0f, 30.0f, 0.01f), p.pos, glm::vec3(0.0f, 0.0f, -1.0f));
        }
        else {
            view = glm::lookAt(p.pos, p.pos + p.front, p.up);
        }

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 5.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        // 1. Draw Instanced Cubes
        glUniform1i(glGetUniformLocation(shaderProgram, "isInstanced"), 1);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, static_cast<GLsizei>(cubes.size()));
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, static_cast<GLsizei>(unbreakables.size()));

        // 2. Draw Health Bars (Billboards) for Colliders
        glUniform1i(glGetUniformLocation(shaderProgram, "isInstanced"), 0);
        for (auto& cube : cubes) {
            if (cube.health > 0.0f) { //if it has health, draw a healthbar
                glm::vec3 barPos = cube.pos + glm::vec3(0.0f, cube.scale + 0.2f, 0.0f);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, barPos);

                // Billboard Trick: Copy camera rotation
                model[0][0] = view[0][0]; model[0][1] = view[1][0]; model[0][2] = view[2][0];
                model[1][0] = view[0][1]; model[1][1] = view[1][1]; model[1][2] = view[2][1];
                model[2][0] = view[0][2]; model[2][1] = view[1][2]; model[2][2] = view[2][2];

                // Draw Background (Red)
                glm::mat4 bgModel = glm::scale(model, glm::vec3(1.0f, 0.1f, 0.01f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(bgModel));
                glUniform3f(glGetUniformLocation(shaderProgram, "playerColor"), 1.0f, 0.0f, 0.0f);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                // Draw Foreground (Green)
                if (cube.health <= 0.0f) { 
                    continue; }
                model = glm::translate(model, glm::vec3(-0.5f * (1.0f - cube.health), 0.0f, 0.01f));
                glm::mat4 fgModel = glm::scale(model, glm::vec3(cube.health, 0.1f, 0.01f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fgModel));
                glUniform3f(glGetUniformLocation(shaderProgram, "playerColor"), 0.0f, 1.0f, 0.0f);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // 3. Draw Player Cube
        if (usingSkyCamera) {
            glUniform1i(glGetUniformLocation(shaderProgram, "isInstanced"), 0);
            glUniform3f(glGetUniformLocation(shaderProgram, "playerColor"), p.color.r, p.color.g, p.color.b);
            glm::mat4 pModel = glm::mat4(1.0f);
            pModel = glm::translate(pModel, p.pos);
            pModel = glm::rotate(pModel, glm::radians(-p.yaw), glm::vec3(0, 1, 0));
            pModel = glm::rotate(pModel, glm::radians(p.pitch), glm::vec3(0, 0, 1));
            pModel = glm::scale(pModel, glm::vec3(0.8f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(pModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        ImVec2 hudPos = ImVec2((float)winWidth - 10.0f, (float)winHeight - 10.0f);
        ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.3f);
        ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "COORDINATES");
        ImGui::Separator();
        ImGui::Text("X: %.3f", cameraPos.x);
        ImGui::Text("Y: %.3f", cameraPos.y);
        ImGui::Text("Z: %.3f", cameraPos.z);
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Cubes: %d", cubes.size());
        ImGui::Text("Players: %d", players.size());
        ImGui::Text("Time: %.2f s", currentFrame);
        ImGui::Text("Clicks: %d", totalClicks);
        ImGui::Separator();
        ImGui::Text("Yaw: %.2f", yaw);
        ImGui::Text("Pitch: %.2f", pitch);
        ImGui::Text("Camera: %s", usingSkyCamera ? "Sky" : "PoV");
        ImGui::Text("last click: %.1f , %.1f", (float)lcxpos, (float)lcypos);
        ImGui::End();
        if (!usingSkyCamera) {
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Crosshair", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
            ImGui::TextColored(ImVec4(1, 1, 1, 0.8f), "+");
            ImGui::End();
        }
        if (isPaused) {
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("PAUSE MENU", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::Text("Paused");
            ImGui::Separator();
            if (ImGui::Button("Resume", ImVec2(200, 0))) {
                isPaused = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            static float tempSense = sensitivity;
            if (ImGui::SliderFloat("Sensitivity", &tempSense, 0.01f, 1.0f)) {
                sensitivity = tempSense;
            }
            static int tempColliders = colliders;
            if (ImGui::SliderInt("Colliders", &tempColliders, 0, 50)) {
                colliders = tempColliders;
            }
            if (ImGui::Button("Reset Player", ImVec2(200, 0))) {
                resetPlayer();
            }
            ImGui::Separator();
            if (ImGui::Button("Exit to Desktop", ImVec2(200, 0))) {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::End();
        }
        // --- Stats Window (Top Left) ---
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.35f); // Slightly transparent
        ImGui::Begin("Game Stats", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "BATTLE LOG");
        ImGui::Separator();
        ImGui::Text("Enemies Defeated: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%d", totalKills);
        ImGui::Text("Shots Fired: %d", totalClicks);
        ImGui::Text("Shots Hit: %d", totalHits);
        if (totalClicks > 0) {
            float accuracy = ((float)totalHits / (float)totalClicks) * 100.0f;
            ImGui::Text("Accuracy: %.1f%%", accuracy);
        }
        else {
            ImGui::Text("Accuracy: 0%%");
        }
        const char* rank = "Novice";
        ImGui::Text("Current Rank: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", rank);
        ImGui::Separator();
        ImGui::Text("Ammo: %d / 30", p.ammo);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}