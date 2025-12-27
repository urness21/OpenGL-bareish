#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
// Instanced attributes (from the cubes vector)
layout (location = 2) in vec3 iPos;
layout (location = 3) in float iScale;
layout (location = 4) in vec3 iColor;
layout (location = 5) in vec3 iRot;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform bool isInstanced;
uniform vec3 playerColor;

out vec3 Normal;
out vec3 FragPos;
out vec3 Color;

void main() {
    vec3 worldPos;
    if(isInstanced) {
        vec3 scaledPos = aPos * iScale;
        worldPos = scaledPos + iPos;
        Color = iColor;
        Normal = aNormal;
    } else {
        // --- Player Cube Path ---
        worldPos = vec3(model * vec4(aPos, 1.0));
        Color = playerColor;
        Normal = mat3(transpose(inverse(model))) * aNormal;
    }

    FragPos = worldPos;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}