#include "common.h"

// Initialize the vectors
std::vector<CubeInstance> cubes;
std::vector<player> players;
std::vector<unbreakable> unbreakables;

// Initialize the settings and state
float deltaTime = 0.0f;
bool firstMouse = true;
bool isPaused = true;
float lastX, lastY;
int width, height;
double lcxpos, lcypos;
int totalClicks, totalHits, totalKills;
int colliders = 20;
float enemySpeed = 3.0f;
float mySpeed = 50.0f;

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