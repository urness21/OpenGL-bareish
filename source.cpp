#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <ctime>

// ImGui Includes
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
};

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window);
void createCube();
void createCollider();

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int cubeCount = 2;
static int totalClicks = 0;
std::vector<CubeInstance> cubes;

// Camera
float deltaTime = 0.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float sensitivity = 0.1f;

// --- Shaders ---
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 iPos;
layout (location = 3) in float iScale;
layout (location = 4) in vec3 iColor;
layout (location = 5) in vec3 iRotation;

out vec3 FragPos;
out vec3 Normal;
out vec3 InstanceColor;

uniform mat4 projection;
uniform mat4 view;

mat4 getRotationMatrix(vec3 rot) {
    float cx = cos(rot.x); float sx = sin(rot.x);
    float cy = cos(rot.y); float sy = sin(rot.y);
    float cz = cos(rot.z); float sz = sin(rot.z);
    mat4 rx = mat4(1, 0, 0, 0, 0, cx, sx, 0, 0, -sx, cx, 0, 0, 0, 0, 1);
    mat4 ry = mat4(cy, 0, -sy, 0, 0, 1, 0, 0, sy, 0, cy, 0, 0, 0, 0, 1);
    mat4 rz = mat4(cz, sz, 0, 0, -sz, cz, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    return rz * ry * rx;
}

void main() {
    mat4 rotationMatrix = getRotationMatrix(iRotation);
    vec3 scaledPos = aPos * iScale;
    vec4 rotatedPos = rotationMatrix * vec4(scaledPos, 1.0);
    Normal = normalize(vec3(rotationMatrix * vec4(aNormal, 0.0)));
    FragPos = rotatedPos.xyz + iPos; 
    InstanceColor = iColor;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec3 InstanceColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main() {
    vec3 ambient = 0.15 * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.5 * spec * lightColor;
    FragColor = vec4((ambient + diffuse + specular) * InstanceColor, 1.0);
}
)glsl";

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    // Initialize Cubes
    createCollider();

    // GLFW & OpenGL Setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL HUD", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);

    // --- ImGui Setup ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    // Shaders
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

    // Buffer Setup
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
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        for (auto& cube : cubes) {
            cube.rotation += cube.rotVel * deltaTime;
            cube.colorTime += deltaTime * 1.5f;
            if (cube.timeAlive < 0) {
                cube.color = glm::vec3(sin(cube.colorTime)*0.5+0.5, 1.0, sin(cube.colorTime*4)*0.5+0.5);
                continue;
            }
            cube.color = glm::vec3(sin(cube.colorTime) * 0.5 + 0.5, sin(cube.colorTime) * 0.5, sin(cube.colorTime) * 0.5 + 0.5);
            cube.pos += cube.vel * deltaTime * 5.0f;
            cube.scale -= 0.2f * deltaTime;
            cube.timeAlive += 1;
        }

        // Remove cubes that are invisible OR too old
        std::erase_if(cubes, [](const CubeInstance& cube) {
            return cube.scale <= 0.0f;
            });

        glBindBuffer(GL_ARRAY_BUFFER, iVBO);
        glBufferData(GL_ARRAY_BUFFER, cubes.size() * sizeof(CubeInstance), cubes.data(), GL_DYNAMIC_DRAW);

        // Rendering
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 5.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, static_cast<GLsizei>(cubes.size()));
        // --- ImGui Frame ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // HUD: Top Right
        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 10, 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowBgAlpha(0.3f);
        ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "COORDINATES");
        ImGui::Separator();
        ImGui::Text("X: %.3f", cameraPos.x);
        ImGui::Text("Y: %.3f", cameraPos.y);
        ImGui::Text("Z: %.3f", cameraPos.z);
        ImGui::Text("FPS: %.1f", io.Framerate);
		ImGui::Text("Cubes: %d", cubes.size());
		ImGui::Text("Time: %.2f s", currentFrame);
        ImGui::Text("Clicks: %d)", totalClicks);
		ImGui::Separator();
		ImGui::Text("Yaw: %.2f", yaw);
		ImGui::Text("Pitch: %.2f", pitch);

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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    float speed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    // --- Detect Mouse Held Down ---
    // Check if the left button is currently pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        // Use ImGui check to ensure we aren't clicking through UI
        if (!ImGui::GetIO().WantCaptureMouse) {
            // This code runs every frame the button is held
           // createCube();
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            totalClicks++;
            createCube();
            std::cout << "World Click at: " << xpos << ", " << ypos << std::endl;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = (float)xposIn; float ypos = (float)yposIn;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = (xpos - lastX) * sensitivity;
    float yoffset = (lastY - ypos) * sensitivity;
    lastX = xpos; lastY = ypos;
    yaw += xoffset; pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f; if (pitch < -89.0f) pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void createCube() {
    CubeInstance cube;
    cube.pos = cameraPos + (cameraFront * 2.0f);
    // Give it a random velocity so it flies away
    glm::vec3 spread = glm::vec3(
        ((rand() % 100) / 100.0f) - 0.5f,
        ((rand() % 100) / 100.0f) - 0.5f,
        ((rand() % 100) / 100.0f) - 0.5f
    );
    cube.vel = (cameraFront * 5.0f) + (spread * 1.0f);
	cube.scale = 0.3f;
    cube.colorTime = (float)(rand() % 100);
    cube.rotation = glm::vec3(0.0f);
    cube.rotVel = glm::vec3((rand() % 100) / 50.0f);
    cube.timeAlive = 0;

    cubes.push_back(cube);
}
void createCollider() {
    CubeInstance cube;
    cube.pos = cameraPos + (cameraFront * 25.0f);
    // Give it a random velocity so it flies away
    glm::vec3 spread = glm::vec3(
        ((rand() % 100) / 100.0f) - 0.5f,
        ((rand() % 100) / 100.0f) - 0.5f,
        ((rand() % 100) / 100.0f) - 0.5f
    );
    cube.vel = cameraFront + 0.0f;
    cube.scale = 1.0f;
    cube.colorTime = (float)(rand() % 100);
    cube.rotation = glm::vec3(0.0f);
    cube.rotVel = glm::vec3((rand() % 100) / 50.0f);
    cube.timeAlive = -1;

    cubes.push_back(cube);
}