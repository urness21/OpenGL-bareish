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
        // 1. Calculate a "flat" forward vector so looking up doesn't make you fly
        glm::vec3 flatFront = glm::normalize(glm::vec3(p.front.x, 0.0f, p.front.z));
        glm::vec3 right = glm::normalize(glm::cross(p.front, p.up));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) p.pos += speed * flatFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) p.pos -= speed * flatFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) p.pos -= speed * right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) p.pos += speed * right;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) p.pos -= speed * p.up;
        if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) resetPlayer();
        float groundLevel = -1.0f + players[0].height; // ground y + player height
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && p.pos.y <= groundLevel + 0.01f) {
            p.vel.y = 7.0f; // Give an upward "kick"
        }
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
                    shoot();
                    p.lastShotTime = currentTime;
                    p.ammo -= 1;
                    if (p.ammo <= 0) {
                        p.ammo = 30;
                    }
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
    p.height = 1.0f;
    p.yaw = 0.0f;
    p.pos = glm::vec3(-3.0f, 3.0f, 0.0f);
    p.vel = glm::vec3(0.0f, 0.0f, 0.0f);
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
void initGame() {
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

    glm::vec3 positions[] = {
        glm::vec3(20.0f, 0.0f,  20.0f),
        glm::vec3(-20.0f, 0.0f,  20.0f),
        glm::vec3(20.0f, 0.0f, -20.0f),
        glm::vec3(-20.0f, 0.0f, -20.0f)
    };
    for (int i = 0; i < 4; i++) {
        pillar p;
        p.pos = positions[i]*1.5f;
        p.color = glm::vec3(0.8f, 0.2f, 0.2f);
        pillars.push_back(p);
    }

    emers emerson;
    emerson.pos = glm::vec3(12.0f, 3.0f, 0.0f);
    emerson.health = 1000.0f;
    emerson.vel = glm::vec3(0.0f, 0.0f, 0.0f);
    emerson.height = 3.0f;
    emersons.push_back(emerson);

    resetAll();
}
void shoot() {
    projectile projectile;
    projectile.pos = cameraPos + (cameraFront * 1.0f);
    projectile.color = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 spread = glm::vec3(
        ((rand() % 100) / 100.0f) - 0.5f,
        ((rand() % 100) / 100.0f) - 0.5f,
        ((rand() % 100) / 100.0f) - 0.5f
    );
    projectile.vel = (cameraFront * 100.2f);// +(spread * 0.5f);
    projectile.rotation = glm::vec3(3.20f, 0.0f, 0.0f);
    projectile.rotVel = glm::vec3(0.0f, 0.0f, 254.993f);
    projectile.dmg = 0.5f;
    projectile.distanceTraveled = 0.0f;
    projectiles.push_back(projectile);
}

void createCollider(glm::vec3 pos, bool chases, float health) {
    CubeInstance cube;
    cube.color = glm::vec3(0.0f, 0.5f, 0.3f);
    cube.pos = pos;
    cube.chases = chases;
    cube.vel = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.scale = 1.0f;
    cube.colorTime = (float)(rand() % 100);
    cube.rotation = glm::vec3(0.0f);
    cube.rotVel = glm::vec3(0.0f);
    cube.timeAlive = -1;
    cube.health = health;
    cube.height = 1.0f;
    cubes.push_back(cube);
}
void createUnbreakable(glm::vec3 pos) {
    unbreakable unbreakable;
    unbreakable.pos = pos;
    unbreakable.color = glm::vec3(0.5f, 0.0f, 1.0f);
    unbreakable.rotation = glm::vec3(0.0f);
    unbreakables.push_back(unbreakable);
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

void createSplash(glm::vec3 pos, glm::vec3 color) {
    int particleCount = 20;
    for (int i = 0; i < particleCount; i++) {
        SplashParticle p;
        p.pos = pos;

        // 1. Horizontal angle (0 to 360 degrees)
        float phi = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;

        // 2. Vertical angle (0 to 180 degrees) - This allows DOWNWARD movement
        float theta = ((float)rand() / RAND_MAX) * 3.14159f;

        // 3. Speed of the burst
        float strength = ((float)rand() / RAND_MAX) * 20.0f + 2.0f;

        // Map angles to X, Y, Z coordinates
        p.vel.x = sin(theta) * cos(phi) * strength;
        p.vel.y = cos(theta) * strength; // Positive is up, Negative is down
        p.vel.z = sin(theta) * sin(phi) * strength;

        p.life = 1.0f;
        p.color = color;

        splashParticles.push_back(p);
    }
}

void handleGravity() {
    // 1. Apply constant Gravity to velocity
    for (auto& p : players) {
        p.vel.y += gravity * deltaTime;
        // 2. Apply vertical velocity to position
        p.pos.y += p.vel.y * deltaTime;
        if (p.pos.y < groundy + p.height) {
            p.pos.y = groundy + p.height;
            p.vel.y = 0.0f;
        }
    }
    for (auto& c : cubes) {
        c.vel.y += gravity * deltaTime;
        c.pos.y += c.vel.y * deltaTime;
        if (c.pos.y < groundy + c.height) {
            c.pos.y = groundy + c.height;
            c.vel.y = 0.0f;
        }
    }
    for (auto& c : emersons) {
        c.vel.y += gravity * deltaTime;
        c.pos.y += c.vel.y * deltaTime;
        if (c.pos.y < groundy + c.height) {
            c.pos.y = groundy + c.height;
            c.vel.y = 0.0f;
        }
    }
    for (auto& c : projectiles) {
        c.vel.y += gravity * deltaTime;
        c.pos.y += c.vel.y * deltaTime;
        if (c.pos.y < groundy) {
            c.pos.y = groundy;
            c.vel.y = 0.0f;
            c.dmg = 0;
            createSplash(c.pos, glm::vec3(0.7f, 0.9f, 1.0f));
        }
    }
}