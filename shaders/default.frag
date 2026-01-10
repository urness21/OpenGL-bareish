#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 Color; // This comes from the Vertex Shader (used for instancing)

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 playerColor; // The uniform you set in C++ with cubeShader.setVec3
uniform bool isInstanced; // The toggle you set in C++

void main() {
    // 1. Pick the base color
    vec3 baseColor;
    if (isInstanced) {
        baseColor = Color; // Use the per-instance color from the buffer
    } else {
        baseColor = playerColor; // Use the manual color set for the projectile
    }

    // 2. Ambient light (minimum visibility)
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
  	
    // 3. Diffuse light (directional surface shading)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // 4. Final Result
    // If baseColor is black (0,0,0), the result will always be black!
    vec3 result = (ambient + diffuse) * baseColor;
    FragColor = vec4(result, 1.0);
}