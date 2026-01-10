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
    float height;
};
struct projectile {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 vel;
    glm::vec3 rotation;
    glm::vec3 rotVel;
    float dmg;
    float distanceTraveled;
};
struct SplashParticle {
    glm::vec3 pos;
    glm::vec3 vel;
    float life;
    glm::vec3 color;
};
struct unbreakable {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 rotation;
};

struct player {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 front;
    glm::vec3 up;
    float height;
    float yaw;
    float pitch;
    glm::vec3 color;
    int ammo;
    float lastShotTime;
    float health;
};
struct emers {
    glm::vec3 pos;
    glm::vec3 vel;
    float height;
    float health;
};
struct pillar {
    glm::vec3 pos;
    glm::vec3 color;
};
// --- Global Data Declarations (The "Announcements") ---
// initialized in global.cpp
extern std::vector<CubeInstance> cubes;
extern std::vector<projectile> projectiles;
extern std::vector<player> players;
extern std::vector<unbreakable> unbreakables;
extern std::vector<pillar> pillars;
extern std::vector<emers> emersons; 
extern std::vector<SplashParticle> splashParticles;

extern int height;
extern int width;

extern float gravity;
extern float deltaTime;
extern bool isPaused;
extern bool firstMouse;
extern int totalClicks;
extern int totalHits;
extern int totalKills;
extern int colliders;
extern float lastX;
extern float lastY;
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

//temp
extern float tempX;
extern float tempY;
extern float tempZ;

//world
extern float groundy;

#endif