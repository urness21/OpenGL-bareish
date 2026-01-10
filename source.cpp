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
#include "objModel.h"

int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "OpenGL HUD", NULL, NULL);
    if (!window) {
        const char* description;
        int code = glfwGetError(&description);
        std::cout << "Window Creation Failed: " << description << " (Code: " << code << ")" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    lastX = (float)width / 2.0f;
    lastY = (float)height / 2.0f;
    mouse_callback(window, lastX, lastY);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    Shader cubeShader("shaders/default.vert", "shaders/default.frag");
    Shader hudShader("shaders/rectangle.vert", "shaders/rectangle.frag");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    Mesh cubeMesh(Shapes::cubeVertices, sizeof(Shapes::cubeVertices), { 3, 3 }, sizeof(CubeInstance));
    Mesh hudMesh(Shapes::rectangleVertices, sizeof(Shapes::rectangleVertices), { 3 });
    Mesh planeMesh(Shapes::quadVertices, sizeof(Shapes::quadVertices), { 3, 3 });
    objModel myModel("models/projectile.obj");
    objModel pillar("models/pillar.obj");
    objModel floater("models/floater.obj");
    objModel emers("models/emers.obj");
    float lastFrame = 0.0f;
    initGame();
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


        glm::mat4 ground = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1, 0));
        glm::vec3 groundPos = glm::vec3(ground[3]);
        ground = glm::scale(ground, glm::vec3(60.0f, 1.0f, 60.0f));

        // --- 1. PURE LOGIC STEP ---
        if (!isPaused) {
            handleGravity();
     
            for (auto& proj : projectiles) {
                glm::vec3 movement = proj.vel * deltaTime;
                proj.pos += movement;
                proj.rotation += proj.rotVel * deltaTime;
                proj.distanceTraveled += glm::length(movement);
                if (proj.distanceTraveled >= 75.0f) {
                    createSplash(proj.pos, glm::vec3(rand(), rand(), rand()));
                    proj.dmg = 0; // Mark for deletion
                    continue;     // Skip collision check since it's dead
                }
                for (auto& enemy : cubes) {
                    if (enemy.chases) {
                        float dist = glm::distance(proj.pos, enemy.pos);
                        if (dist < (enemy.scale)) {
                            enemy.health -= proj.dmg;
                            proj.dmg = 0; // Mark projectile for deletion
                            createSplash(proj.pos, glm::vec3(0.7f, 0.3f, 0.0f));
                        }
                    }
                }
                for (auto& emerson : emersons) {
                    glm::mat4 modelMatrix = glm::mat4(1.0f);
                    modelMatrix = glm::translate(modelMatrix, emerson.pos);
                    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    float currentAngle = (float)glfwGetTime() * 2.0f;
                    modelMatrix = glm::rotate(modelMatrix, currentAngle, glm::vec3(0.0f, 0.0f, 1.0f));
                    glm::mat4 invModel = glm::inverse(modelMatrix);
                    glm::vec3 localProjPos = glm::vec3(invModel * glm::vec4(proj.pos, 1.0f));
                    bool hit = (localProjPos.x >= emers.minBounds.x && localProjPos.x <= emers.maxBounds.x) &&
                        (localProjPos.y >= emers.minBounds.y && localProjPos.y <= emers.maxBounds.y) &&
                        (localProjPos.z >= emers.minBounds.z && localProjPos.z <= emers.maxBounds.z);

                    if (hit) {
                        emerson.health -= proj.dmg;
                        proj.dmg = 0;
                        createSplash(proj.pos, glm::vec3(0.7f, 0.3f, 0.0f));
                    }
                }
            }
            std::erase_if(projectiles, [](const projectile& p) { return p.dmg <= 0; });
        }
        for (auto& p : splashParticles) {
            p.vel.y += gravity * deltaTime; // Apply gravity to splash too
            p.pos += p.vel * deltaTime;
            p.life -= deltaTime * 1.5f;    // Particles last about 0.6 seconds
            if (p.pos.y < groundy) {
                // Snap to surface so it doesn't get stuck underground
                p.pos.y = groundy;

                // Invert Y velocity and reduce it (0.4f = 40% energy kept)
                p.vel.y = -p.vel.y * 0.4f;

                // Friction: Slow down horizontal movement on impact
                p.vel.x *= 0.8f;
                p.vel.z *= 0.8f;
            }
        }
        // Remove dead particles
        std::erase_if(splashParticles, [](const SplashParticle& p) {
            return p.life <= 0.0f;
            });
        // Cleanup and Spawning
        std::erase_if(cubes, [](const CubeInstance& cube) {
            return (cube.scale <= 0.0f && cube.timeAlive >= 0) || (cube.timeAlive < 0 && cube.health <= 0.0f);
            });

        int colliderCount = 0;
        for (auto& c : cubes) if (c.timeAlive < 0) colliderCount++;
        while (colliderCount < colliders) {
            spawnEnemyAtRadius(15.0f, 30.0f);
            colliderCount++;
        }

        // --- 2. RENDERING STEP ---
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)fbWidth / fbHeight, 0.1f, 100.0f);
        glm::mat4 view = (usingSkyCamera) ?
            glm::lookAt(p.pos + glm::vec3(0.0f, 30.0f, 0.01f), p.pos, glm::vec3(0.0f, 0.0f, -1.0f)) :
            glm::lookAt(p.pos, p.pos + p.front, p.up);

        cubeShader.use();
        cubeShader.setVec3("lightPos", p.pos);
        cubeShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // Pure white light
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);
        cubeShader.setVec3("viewPos", cameraPos);

        // A. DRAW FLOOR
        cubeShader.setInt("isInstanced", 0);
        cubeShader.setMat4("model", ground);
        planeMesh.draw(0, GL_TRIANGLE_STRIP);

        // B. DRAW ENEMIES (Instanced Cubes)
        cubeShader.setInt("isInstanced", 1);
        cubeMesh.updateInstances(cubes.data(), cubes.size() * sizeof(CubeInstance));
        cubeMesh.draw(static_cast<int>(cubes.size()));

        //draw pillars
        cubeShader.setInt("isInstanced", 0); // Single object mode
        for (auto& pill : pillars) {
            glm::mat4 pillarModel = glm::mat4(1.0f);
            pillarModel = glm::translate(pillarModel, pill.pos);
            pillarModel = glm::scale(pillarModel, glm::vec3(1.0f));
            cubeShader.setMat4("model", pillarModel);
            cubeShader.setVec3("playerColor", pill.color);
            pillar.Draw();
        }
        //draw floaters
        glm::mat4 floaterModel = glm::mat4(1.0f);
        floaterModel = glm::translate(floaterModel, glm::vec3(0.0f, 10.0f, 0.0f));
        cubeShader.setMat4("model", floaterModel);
        cubeShader.setVec3("playerColor", glm::vec3(1.0f, 1.0f, 1.0f));
        floater.Draw();
        //draw emerson
        glm::mat4 emersonModel = glm::mat4(1.0f);
        emersonModel = glm::translate(emersonModel, emersons[0].pos);
        emersonModel = glm::rotate(emersonModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        float angle = (float)glfwGetTime() * 2.0f; // Adjust 2.0f for speed
        emersonModel = glm::rotate(emersonModel, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        cubeShader.setMat4("model", emersonModel);
        cubeShader.setVec3("playerColor", glm::vec3(1.0f, 0.0f, 1.0f));
        emers.Draw();
        // --- DEBUG: DRAW ROTATED HITBOX ---
        glm::vec3 size = emers.maxBounds - emers.minBounds;
        glm::vec3 center = (emers.minBounds + emers.maxBounds) / 2.0f;

        glm::mat4 debugModel = glm::mat4(1.0f);
        debugModel = glm::translate(debugModel, emersons[0].pos);

        // Match the Emerson Render Rotations exactly
        debugModel = glm::rotate(debugModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        debugModel = glm::rotate(debugModel, (float)glfwGetTime() * 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));

        // Apply local offset and scale
        debugModel = glm::translate(debugModel, center);
        debugModel = glm::scale(debugModel, size);

        cubeShader.setMat4("model", debugModel);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        cubeMesh.draw();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // --- C. DRAW PROJECTILES (.obj Models) ---
        cubeShader.setInt("isInstanced", 0); // Single object mode
        for (auto& proj : projectiles) {
            glm::mat4 bulletModel = glm::mat4(1.0f);
            bulletModel = glm::translate(bulletModel, proj.pos);
            // 1. Rotation: Face the direction of travel
            if (glm::length(proj.vel) > 0.1f) {
                float angle = atan2(proj.vel.x, proj.vel.z);
                bulletModel = glm::rotate(bulletModel, angle, glm::vec3(0, 1, 0));
            }
            // 2. Rotation: Apply any spinning from proj.rotation
            bulletModel = glm::rotate(bulletModel, proj.rotation.x, glm::vec3(1, 0, 0));
            bulletModel = glm::rotate(bulletModel, proj.rotation.y, glm::vec3(0, 1, 0));
            bulletModel = glm::rotate(bulletModel, proj.rotation.z, glm::vec3(0, 0, 1));
            // 3. Scale: Adjust based on your .obj size
            bulletModel = glm::scale(bulletModel, glm::vec3(0.1f));
            cubeShader.setMat4("model", bulletModel);
            // 4. Color: Pass the struct color to the 'playerColor' uniform
            cubeShader.setVec3("playerColor", proj.color);
            myModel.Draw();
        }
        cubeShader.use();
        cubeShader.setInt("isInstanced", 0);

        for (auto& p : splashParticles) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, p.pos);
            float size = 0.3f * p.life;
            model = glm::scale(model, glm::vec3(size));
            //model = glm::rotate(model, (float)glfwGetTime() * 5.0f, glm::vec3(0, 0, 0));

            cubeShader.setMat4("model", model);
            cubeShader.setVec3("playerColor", p.color);
            cubeMesh.draw();
        }

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
        for (auto& emerson : emersons) {
            if (emerson.health > 0.0f) {
                // 1. Calculate health percentage (assuming 1000 is max health)
                float healthPct = emerson.health / 1000.0f;

                glm::vec3 barPos = emerson.pos + glm::vec3(0.0f, 4.5f, 0.0f); // Adjust height (1.5f) for Emerson's size
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, barPos);

                // Billboard logic (keep this part the same)
                model[0][0] = view[0][0]; model[0][1] = view[1][0]; model[0][2] = view[2][0];
                model[1][0] = view[0][1]; model[1][1] = view[1][1]; model[1][2] = view[2][1];
                model[2][0] = view[0][2]; model[2][1] = view[1][2]; model[2][2] = view[2][2];

                // Red Background (Fixed width of 1.0)
                glm::mat4 bgModel = glm::scale(model, glm::vec3(1.0f, 0.1f, 0.01f));
                cubeShader.setMat4("model", bgModel);
                cubeShader.setVec3("playerColor", glm::vec3(1.0f, 0.0f, 0.0f));
                cubeMesh.draw();

                // Green Foreground (Scaled by percentage)
                // Offset moves the bar to start at the left edge of the red background
                glm::mat4 fgBase = glm::translate(model, glm::vec3(-0.5f * (1.0f - healthPct), 0.0f, 0.01f));
                glm::mat4 fgModel = glm::scale(fgBase, glm::vec3(healthPct, 0.1f, 0.01f));

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
            //tempX = projectiles[0].rotation.x;
            //if (ImGui::SliderFloat("RotationY", &tempX, 0.0f, 359.99f)) {
            //    projectiles[0].rotation.x = tempX;
            //}
            //tempY = projectiles[0].rotation.y;
            //if (ImGui::SliderFloat("RotationY", &tempY, 0.0f, 359.99f)) {
            //    projectiles[0].rotation.y = tempY;
            //}
            //tempZ = projectiles[0].rotation.z;
            //if (ImGui::SliderFloat("RotationZ", &tempZ, 0.0f, 359.99f)) {
            //    projectiles[0].rotation.z = tempZ;
            //}
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
            ImGui::Text("Accuracy: %.1f%", accuracy);
        }
        else {
            ImGui::Text("Accuracy: 0%");
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