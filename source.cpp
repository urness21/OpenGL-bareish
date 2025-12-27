#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// --- Data Structure ---
struct CubeInstance {
    glm::vec3 pos;
    float scale;
    glm::vec3 color;
    glm::vec3 rotation;
    glm::vec3 vel;
    glm::vec3 rotVel;
    float colorTime;
    int timeAlive;
    float health;
};

struct player {
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
    float yaw;
    float pitch;
    glm::vec3 color;
    int ammo;
    float lastShotTime;
    float health;
};

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window);
void createCube();
void createCollider(glm::vec3);
void switchCamera();
void resetPlayer();
void spawnEnemyAtRadius(float minRadius, float maxRadius);
void resetAll();
// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
static int totalClicks = 0;
std::vector<CubeInstance> cubes;
std::vector<player> players;
int colliders = 3;
// Camera
float deltaTime = 0.0f;
glm::vec3 cameraPos = glm::vec3(-3.0f, 0.0f, 0.0f);
glm::vec3 skyCamera = glm::vec3(0.0f, 30.0f, 0.0f);
bool usingSkyCamera = false;
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool firstMouse = true;
double lcxpos, lcypos;
float yaw = 0.0f;
float pitch = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;
float sensitivity = 0.1f;
bool isPaused = true;

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

GLFWwindow* window;
GLFWmonitor* primaryMonitor;
int width, height;

int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    std::string vertShaderStr = loadShaderSource("default.vert");
    std::string fragShaderStr = loadShaderSource("default.frag");
    const char* vertexShaderSource = vertShaderStr.c_str();
    const char* fragmentShaderSource = fragShaderStr.c_str();

    // Initialize Cubes
    createCollider(glm::vec3(0.0f, 0.0f, 0.0f));
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
                // Logic for Stationary Colliders (Now moving toward player)
                if (cube.timeAlive < 0) {
                    cube.color = glm::vec3(sin(cube.colorTime) * 0.5 + 0.5, 1.0, sin(cube.colorTime * 4) * 0.5 + 0.5);
                    // 1. Calculate direction vector (Target - Current)
                    glm::vec3 direction = p.pos - cube.pos;
                    // 2. Normalize the direction so the speed is consistent
                    if (glm::length(direction) > 0.1f) { // Prevent jittering when on top of player
                        direction = glm::normalize(direction);
                        float enemySpeed = 3.0f;
                        cube.pos += direction * enemySpeed * deltaTime;
                        // 4. Face the player (Optional: Update rotation to look at player)
                        cube.rotation.y = atan2(direction.x, direction.z);
                    }
                }
                else {
                    // Logic for moving cubes (bullets)
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
                            }
                        }
                    }
                }
            }
        }
        // Remove cubes that are invisible OR too old OR dead colliders
        std::erase_if(cubes, [](const CubeInstance& cube) {
            bool isDeadBullet = (cube.scale <= 0.0f && cube.timeAlive >= 0);
            bool isDeadCollider = (cube.timeAlive < 0 && cube.health <= 0.0f);

            if (isDeadCollider) {
                // Generate a random position within a 10x10 area
                float rx = (rand() % 200 / 10.0f) - 10.0f;
                float ry = (rand() % 100 / 10.0f) - 5.0f;
                float rz = (rand() % 200 / 10.0f) - 10.0f;

                // We can't call push_back inside erase_if (it breaks the iterator)
                // So we use a flag or a simple trick: spawn it in the next frame
                // For now, let's just trigger a createCollider call after erase_if
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
            view = glm::lookAt(skyCamera, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
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

        // --- ImGui ---
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
        // Enemy Kill Counter
        ImGui::Text("Enemies Defeated: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%d", 0);
        // Calculate Kill Streak or Rank based on placeholder logic
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

void processInput(GLFWwindow* window) {
    player& p = players[0];
    static bool pPressed = false;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (!pPressed) {
            isPaused = !isPaused;
            if (isPaused) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true; // Reset mouse delta to prevent snapping
            }
            pPressed = true;
        }
    }
    else {
        pPressed = false;
    }
    if (!isPaused) {
        float speed = 5.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) p.pos += speed * p.front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) p.pos -= speed * p.front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) p.pos -= glm::normalize(glm::cross(p.front, p.up)) * speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) p.pos += glm::normalize(glm::cross(p.front, p.up)) * speed;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) p.pos -= speed * p.up;
        if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) resetPlayer();
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) p.pos += speed * p.up;
        static bool rPressed = false;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            if (!rPressed) {
                resetAll();
                rPressed = true;
            }
        }
        else {
            rPressed = false;
        }

        static bool mPressed = false;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            if (!mPressed) {
                switchCamera();
                mPressed = true;
            }
        }
        else {
            mPressed = false;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            float currentTime = (float)glfwGetTime();
            float cooldown = 0.05f;
            if (!ImGui::GetIO().WantCaptureMouse) {
                if (currentTime - p.lastShotTime >= cooldown) {
                    glfwGetCursorPos(window, &lcxpos, &lcypos);
                    totalClicks++;
                    createCube();
                    p.lastShotTime = currentTime;
                }
            }
        }
    }
}

void switchCamera() {
    usingSkyCamera = !usingSkyCamera;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (isPaused) return;
    player& p = players[0];
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = (xpos - lastX) * sensitivity;
    float yoffset = (lastY - ypos) * sensitivity;
    lastX = xpos;
    lastY = ypos;
    p.yaw += xoffset;
    if (p.yaw > 360.0f) p.yaw -= 360.0f;
    if (p.yaw < 0.0f) p.yaw += 360.0f;
    p.pitch += yoffset;
    if (p.pitch > 89.0f) p.pitch = 89.0f;
    if (p.pitch < -89.0f) p.pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(p.yaw)) * cos(glm::radians(p.pitch));
    front.y = sin(glm::radians(p.pitch));
    front.z = sin(glm::radians(p.yaw)) * cos(glm::radians(p.pitch));
    p.front = glm::normalize(front);
}
void resetAll() {
    cubes.clear();
    resetPlayer();
    for (int i = 0; i < colliders; i++) {
        spawnEnemyAtRadius(15, 30);
    }
    totalClicks = 0;
}
void resetPlayer() {
    player& p = players[0];
    p.pitch = 0.0f;
    p.yaw = 0.0f;
    p.pos = glm::vec3(-3.0f, 0.0f, 0.0f);
    p.up = glm::vec3(0.0f, 1.0f, 0.0f);
    p.front = glm::vec3(0.0f, 0.0f, -1.0f);
    p.ammo = 30;
    glfwGetWindowSize(window, &width, &height);
    lastX = (float)width / 2.0f;
    lastY = (float)height / 2.0f;
    mouse_callback(window, lastX, lastY);
    glfwSetCursorPos(window, lastX, lastY);
}

void createCube() {
    CubeInstance cube;
    cube.pos = cameraPos + (cameraFront * 1.0f);
    glm::vec3 spread = glm::vec3(
        ((rand() % 100) / 100.0f) - 0.5f,
        ((rand() % 100) / 100.0f) - 0.5f,
        ((rand() % 100) / 100.0f) - 0.5f
    );
    cube.vel = (cameraFront * 8.0f) + (spread * 1.0f);
    cube.scale = 0.3f;
    cube.colorTime = (float)(rand() % 100);
    cube.rotation = glm::vec3(0.0f);
    cube.rotVel = glm::vec3((rand() % 100) / 50.0f);
    cube.timeAlive = 0;
    cube.health = 0.0f;
    cubes.push_back(cube);
}

void createCollider(glm::vec3 pos) {
    CubeInstance cube;
    cube.pos = pos;
    cube.vel = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.scale = 1.0f;
    cube.colorTime = (float)(rand() % 100);
    cube.rotation = glm::vec3(0.0f);
    cube.rotVel = glm::vec3(0.0f);
    cube.timeAlive = -1;
    cube.health = 1.0f;
    cubes.push_back(cube);
}

void spawnEnemyAtRadius(float minRadius, float maxRadius) {
    player& p = players[0];
    // 1. Get a random angle in radians (0 to 360 degrees)
    float angle = (float)(rand() % 360) * (3.14159f / 180.0f);
    // 2. Get a random radius between min and max
    float radius = minRadius + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxRadius - minRadius)));
    // 3. Convert Polar coordinates to Cartesian (X, Z)
    float xOffset = cos(angle) * radius;
    float zOffset = sin(angle) * radius;
    // 4. Position it relative to the player
    glm::vec3 spawnPos = p.pos + glm::vec3(xOffset, 0.0f, zOffset);
    createCollider(spawnPos);
}