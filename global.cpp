#include "common.h"

// Initialize the vectors
std::vector<CubeInstance> cubes;
std::vector<projectile> projectiles;
std::vector<player> players;
std::vector<unbreakable> unbreakables;
std::vector<pillar> pillars;
std::vector<emers> emersons;
std::vector<SplashParticle> splashParticles;

// window
int height = 800;
int width = 1280;

// Initialize the settings and state
float deltaTime = 0.0f;
bool firstMouse = true;
bool isPaused = true;
float lastX, lastY;
double lcxpos, lcypos;
int totalClicks, totalHits, totalKills;
int colliders = 3;
float enemySpeed = 3.0f;
float mySpeed = 5.0f;
float gravity = -18.0f;

// Initialize camera and input
glm::vec3 cameraPos = glm::vec3(-3.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 skyCamera = glm::vec3(0.0f, 30.0f, 0.0f);
bool usingSkyCamera = false;
float yaw = 0.0f;
float pitch = 0.0f;
float sensitivity = 0.1f;

// OpenGL handles
GLFWwindow* window = nullptr;
GLFWmonitor* primaryMonitor;
unsigned int unbreakVAO = 0;
unsigned int unbreakVBO = 0;

//temp
float tempRotationX = 0.0f;
float tempRotationY = 0.0f;
float tempRotationZ = 0.0f;

//world
float groundy = -1.0f;