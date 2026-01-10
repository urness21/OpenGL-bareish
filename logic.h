#ifndef LOGIC_H
#define LOGIC_H
#include "common.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window);
void createCollider(glm::vec3, bool, float);
void switchCamera();
void resetPlayer();
void spawnEnemyAtRadius(float minRadius, float maxRadius);
void resetAll();
void createSplash(glm::vec3 pos, glm::vec3 color);
void createUnbreakable(glm::vec3 pos);
void shoot();
void initGame();
void handleGravity();
#endif