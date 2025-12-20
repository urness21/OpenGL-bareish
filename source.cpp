#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <ctime>
void mouse_button_callback(GLFWwindow*, int, int, int);
void mouse_callback(GLFWwindow*, double, double);

// --- Data Structure ---
struct CubeInstance {
    glm::vec3 pos;
    float scale;
    glm::vec3 color;
    glm::vec3 rotation;
    glm::vec3 vel;
    glm::vec3 rotVel;
    float colorTime;
    float baseScale;
    float scaleTime;
    float pulseSpeed;
    int timeAlive;
};

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int cubeCount = 20000;
std::vector<CubeInstance> cubes;
//camera
float deltaTime = 0.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool firstMouse = true;
float yaw = -90.0f; // Initialized to -90 degrees to point toward -Z
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float sensitivity = 0.1f;

// --- 1. Vertex Shader (Cleaned up Raw String) ---
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
    // Normals must be rotated but NOT translated
    Normal = normalize(vec3(rotationMatrix * vec4(aNormal, 0.0)));
    
    FragPos = rotatedPos.xyz + iPos; 
    InstanceColor = iColor;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)glsl";

// --- 2. Fragment Shader (Cleaned up Raw String) ---
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
    // Ambient
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular (Blinn-Phong)
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * InstanceColor;
    FragColor = vec4(result, 1.0);
}
)glsl";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    for (int i = 0; i < cubeCount; i++) {
        CubeInstance cube;
        cube.pos = glm::vec3((rand() % 200 - 100) / 10.0f, (rand() % 200 - 100) / 10.0f, (rand() % 500) / -10.0f - 10.0f);
        cube.vel = glm::vec3(0.0f, 0.0f, (rand() % 50 + 20) / 10.0f);
        cube.baseScale = (rand() % 100 + 30) / 800.0f;
        cube.scale = cube.baseScale;
        cube.scaleTime = (float)(rand() % 100);
        cube.pulseSpeed = (rand() % 30) / 10.0f + 1.0f;
        cube.colorTime = (float)(rand() % 100);
        cube.rotation = glm::vec3((rand() % 360) * 0.0174f, (rand() % 360) * 0.0174f, (rand() % 360) * 0.0174f);
        cube.rotVel = glm::vec3(((rand() % 100) / 50.0f - 1.0f), ((rand() % 100) / 50.0f - 1.0f), ((rand() % 100) / 50.0f - 1.0f)) * 0.5f;
        cube.timeAlive = 0;
        cubes.push_back(cube);
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Lighting Cubes", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    // Tell GLFW to hide the cursor and lock it to the center of the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);

    // Compile logic (Simplified for space)
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

    // Cube Data: Pos (3) and Normal (3)
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
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

    // Instance attributes
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
   //         if(cube.timeAlive>=1000){
   // //            cube.pos = glm::vec3((rand() % 200 - 100) / 10.0f, (rand() % 200 - 100) / 10.0f, -50.0f);
   // //            cube.vel = glm::vec3(0.0f, 0.0f, (rand() % 50 + 20) / 10.0f);
   // //            cube.baseScale = (rand() % 100 + 30) / 800.0f;
   // //            cube.scale = cube.baseScale;
   // //            cube.scaleTime = (float)(rand() % 100);
   // //            cube.pulseSpeed = (rand() % 30) / 10.0f + 1.0f;
   // //            cube.colorTime = (float)(rand() % 100);
   // //            cube.rotation = glm::vec3((rand() % 360) * 0.0174f, (rand() % 360) * 0.0174f, (rand() % 360) * 0.0174f);
			//	//cube.rotVel = glm::vec3(((rand() % 100) / 50.0f - 1.0f), ((rand() % 100) / 50.0f - 1.0f), ((rand() % 100) / 50.0f - 1.0f)) * 0.5f;
   //             //continue; //hides the cube (fake delete)
			//}
            cube.pos += cube.vel * deltaTime * 5.0f;
            if (cube.pos.z > cameraPos.z + 2.0f) cube.pos.z = -50.0f;
            cube.rotation += cube.rotVel * deltaTime * 2.0f;
            cube.colorTime += deltaTime * 1.5f;
            cube.color = glm::vec3(std::sin(cube.colorTime) * 0.5f + 0.5f, std::sin(cube.colorTime + 2.0f) * 0.5f + 0.5f, std::sin(cube.colorTime + 4.0f) * 0.5f + 0.5f);
            cube.scaleTime += deltaTime * cube.pulseSpeed;
            //cube.scale = cube.baseScale * (std::sin(cube.scaleTime) * 0.3f + 1.0f);
            cube.scale = cube.baseScale * (deltaTime * 0.3f + 1.0f);
			cube.timeAlive += 1;
        }



        glBindBuffer(GL_ARRAY_BUFFER, iVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, cubes.size() * sizeof(CubeInstance), &cubes[0]);

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 5.0f, 5.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, cubeCount);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    // Movement Polling (Continuous)
    float cameraSpeed = 5.0f * deltaTime; // Adjust 5.0f to change speed
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * glm::vec3(0, 0, -1);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos += cameraSpeed * glm::vec3(0, 0, 1);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= cameraSpeed * glm::vec3(1, 0, 0);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += cameraSpeed * glm::vec3(1, 0, 0);

    // Single Action Polling
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        // Trigger a jump or a burst of speed
    }

}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed: y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constraint: Prevent the camera from flipping over at the top/bottom
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Convert Euler angles to a 3D vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << "Left Click at: " << xpos << ", " << ypos << std::endl;

        // Note: To "click" a 3D cube, you would need to implement "Raycasting"
    }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}