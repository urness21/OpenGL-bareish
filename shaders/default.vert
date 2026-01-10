#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
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

// Helper function to build a rotation matrix from Euler angles
mat4 getRotationMatrix(vec3 angles) {
    float x = angles.x; float y = angles.y; float z = angles.z;
    mat4 rx = mat4(1,0,0,0, 0,cos(x),-sin(x),0, 0,sin(x),cos(x),0, 0,0,0,1);
    mat4 ry = mat4(cos(y),0,sin(y),0, 0,1,0,0, -sin(y),0,cos(y),0, 0,0,0,1);
    mat4 rz = mat4(cos(z),-sin(z),0,0, sin(z),cos(z),0,0, 0,0,1,0, 0,0,0,1);
    return rz * ry * rx;
}

void main() {
    vec3 worldPos;
    if(isInstanced) {
        // 1. Scale the vertex
        vec3 scaledPos = aPos * iScale; 
        
        // 2. Rotate the vertex before moving it to world space
        mat4 rotation = getRotationMatrix(iRot);
        vec4 rotatedPos = rotation * vec4(scaledPos, 1.0);
        
        // 3. Move to instance position
        worldPos = rotatedPos.xyz + iPos; 
        
        Color = iColor;
        // Rotate the normal as well so lighting stays correct
        Normal = mat3(rotation) * aNormal; 
    } else {
        worldPos = vec3(model * vec4(aPos, 1.0));
        Color = playerColor;
        Normal = mat3(transpose(inverse(model))) * aNormal;
    }

    FragPos = worldPos;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}