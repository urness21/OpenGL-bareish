#include "common.h"
#include "logic.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "Shader.h"
#include "Shapes.h"
#include "Mesh.h"

int main() {
    srand(static_cast<unsigned int>(time(NULL)));
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

    // Initialize Shaders
    Shader cubeShader("default.vert", "default.frag");
    Shader hudShader("rectangle.vert", "rectangle.frag");

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    // --- Initialize Meshes ---
    // 3D instanced Mesh for cubes, Players, and Healthbars
    Mesh cubeMesh(Shapes::cubeVertices, sizeof(Shapes::cubeVertices), { 3, 3 }, sizeof(CubeInstance));
    // 2D Mesh for HUD
    Mesh hudMesh(Shapes::rectangleVertices, sizeof(Shapes::rectangleVertices), { 3 });

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
            // Logic
            for (auto& cube : cubes) {
                cube.rotation += cube.rotVel * deltaTime;
                cube.colorTime += deltaTime * 5.0f;
                if (cube.chases == true) {
                    cube.color = glm::vec3(0.0f, 0.5f, 0.5f);
                    glm::vec3 direction = p.pos - cube.pos;
                    if (glm::length(direction) > 0.1f) {
                        direction = glm::normalize(direction);
                        cube.pos += direction * enemySpeed * deltaTime;
                        cube.rotation.y = atan2(direction.x, direction.z);
                    }
                }
                else {
                    if (cube.health == 0.0f) {
                        cube.color = glm::vec3(sin(cube.colorTime) * 0.5 + 0.5, sin(cube.colorTime) * 0.5, sin(cube.colorTime) * 0.5 + 0.5);
                        cube.pos += cube.vel * deltaTime * 5.0f;
                        cube.scale -= 0.2f * deltaTime;
                        cube.timeAlive += 1;
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
                        cube.color = glm::vec3(sin(cube.colorTime) * 0.5 + 0.5, sin(4.0f * cube.colorTime) * 0.5, sin(2.0f * cube.colorTime) * 0.5 + 0.5);
                        cube.pos += cube.vel * deltaTime * 5.0f;
                    }
                }
            }
        }

        std::erase_if(cubes, [](const CubeInstance& cube) {
            bool isDeadBullet = (cube.scale <= 0.0f && cube.timeAlive >= 0);
            bool isDeadCollider = (cube.timeAlive < 0 && cube.health <= 0.0f);
            return isDeadBullet || isDeadCollider;
            });

        int colliderCount = 0;
        for (auto& c : cubes) if (c.timeAlive < 0) colliderCount++;
        while (colliderCount < colliders) {
            spawnEnemyAtRadius(15.0f, 30.0f);
            colliderCount++;
        }

        // --- Rendering ---
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)fbWidth / fbHeight, 0.1f, 100.0f);
        glm::mat4 view = (usingSkyCamera) ?
            glm::lookAt(p.pos + glm::vec3(0.0f, 30.0f, 0.01f), p.pos, glm::vec3(0.0f, 0.0f, -1.0f)) :
            glm::lookAt(p.pos, p.pos + p.front, p.up);

        // 1. Draw HUD
        glDisable(GL_DEPTH_TEST);
        hudShader.use();
        hudMesh.draw(0, GL_TRIANGLE_STRIP);
        glEnable(GL_DEPTH_TEST);

        // 2. Draw Enemy Swarm (Instanced)
        cubeShader.use();
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);
        cubeShader.setVec3("viewPos", cameraPos);
        cubeShader.setVec3("lightPos", glm::vec3(0.0f, 5.0f, 5.0f));
        cubeShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        cubeShader.setInt("isInstanced", 1);

        cubeMesh.updateInstances(cubes.data(), cubes.size() * sizeof(CubeInstance));
        cubeMesh.draw(static_cast<int>(cubes.size()));

        // 3. Draw Health Bars (Billboards)
        cubeShader.setInt("isInstanced", 0);
        for (auto& cube : cubes) {
            if (cube.health > 0.0f) {
                glm::vec3 barPos = cube.pos + glm::vec3(0.0f, cube.scale + 0.2f, 0.0f);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, barPos);
                model[0][0] = view[0][0]; model[0][1] = view[1][0]; model[0][2] = view[2][0];
                model[1][0] = view[0][1]; model[1][1] = view[1][1]; model[1][2] = view[2][1];
                model[2][0] = view[0][2]; model[2][1] = view[1][2]; model[2][2] = view[2][2];

                // Red Background
                glm::mat4 bgModel = glm::scale(model, glm::vec3(1.0f, 0.1f, 0.01f));
                cubeShader.setMat4("model", bgModel);
                cubeShader.setVec3("playerColor", glm::vec3(1.0f, 0.0f, 0.0f));
                cubeMesh.draw();

                // Green Foreground
                glm::mat4 fgBase = glm::translate(model, glm::vec3(-0.5f * (1.0f - cube.health), 0.0f, 0.01f));
                glm::mat4 fgModel = glm::scale(fgBase, glm::vec3(cube.health, 0.1f, 0.01f));
                cubeShader.setMat4("model", fgModel);
                cubeShader.setVec3("playerColor", glm::vec3(0.0f, 1.0f, 0.0f));
                cubeMesh.draw();
            }
        }

        // 4. Draw Player Cube
        if (usingSkyCamera) {
            cubeShader.setInt("isInstanced", 0);
            cubeShader.setVec3("playerColor", p.color);
            glm::mat4 pModel = glm::mat4(1.0f);
            pModel = glm::translate(pModel, p.pos);
            pModel = glm::rotate(pModel, glm::radians(-p.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            pModel = glm::rotate(pModel, glm::radians(p.pitch), glm::vec3(0.0f, 0.0f, 1.0f));
            pModel = glm::scale(pModel, glm::vec3(0.8f));
            cubeShader.setMat4("model", pModel);
            cubeMesh.draw();
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