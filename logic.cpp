#include "logic.h"
#include "common.h" 
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

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
        float speed = mySpeed * deltaTime;
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
}
void resetPlayer() {
    player& p = players[0];
    p.pitch = 0.0f;
    p.yaw = 0.0f;
    p.pos = glm::vec3(-3.0f, 0.0f, 0.0f);
    p.up = glm::vec3(0.0f, 1.0f, 0.0f);
    p.front = glm::vec3(0.0f, 0.0f, -1.0f);
    p.ammo = 30;
    totalHits = 0;
    totalKills = 0;
    totalClicks = 0;
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

void createCollider(glm::vec3 pos, bool chases, float health) {
    CubeInstance cube;
    cube.pos = pos;
    cube.chases = chases;
    cube.vel = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.scale = 1.0f;
    cube.colorTime = (float)(rand() % 100);
    cube.rotation = glm::vec3(0.0f);
    cube.rotVel = glm::vec3(0.0f);
    cube.timeAlive = -1;
    cube.health = health;
    cubes.push_back(cube);
}
void createUnbreakable(glm::vec3 pos) {
    unbreakable unbreakable;
    unbreakable.pos = pos;
    unbreakable.color = glm::vec3(0.5f, 0.0f, 1.0f);
    unbreakable.rotation = glm::vec3(0.0f);
    unbreakables.push_back(unbreakable);
}
void generateGround() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            createUnbreakable(glm::vec3(i * 1.0f, -3.0f, j * 1.0f));
            j++;
        }
        i++;
    }
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
    createCollider(spawnPos, true, 1.0f);
}

void createSplash(glm::vec3 pos) {

}