#ifndef COMMON_H
#define COMMON_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

struct CubeInstance {
    glm::vec3 pos;
    float scale;
    glm::vec3 color;
    glm::vec3 rotation;
    glm::vec3 vel;
    glm::vec3 rotVel;
    float colorTime;
    int timeAlive;
    bool chases;
    float health;
};

struct unbreakable {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 rotation;
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

// --- Global Data Declarations (The "Announcements") ---
extern std::vector<CubeInstance> cubes;
extern std::vector<player> players;
extern std::vector<unbreakable> unbreakables;

extern float deltaTime;
extern bool isPaused;
extern bool firstMouse;
extern int totalClicks;
extern int totalHits;
extern int totalKills;
extern int colliders;
extern float lastX;
extern float lastY;
extern int width, height;
extern double lcxpos, lcypos;

extern float enemySpeed;
extern float mySpeed;

//camera
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;
extern glm::vec3 skyCamera;
extern bool usingSkyCamera;
extern float yaw;
extern float pitch;
extern float sensitivity;

//gl
extern GLFWwindow* window;
extern GLFWmonitor* primaryMonitor;
extern unsigned int unbreakVAO, unbreakVBO;

#endif