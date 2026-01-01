#include <glad/glad.h>
#include <vector>
#include <numeric>

class Mesh {
public:
    unsigned int VAO, VBO, iVBO;
    int vertexCount;
    bool isInstanced;

    // --- STANDARD CONSTRUCTOR (e.g., for HUD) ---
    Mesh(float* vertices, size_t dataSize, std::vector<int> layout) {
        isInstanced = false;
        iVBO = 0;
        setupBaseMesh(vertices, dataSize, layout);
    }

    // --- INSTANCED CONSTRUCTOR (e.g., for Enemy Cubes) ---
    // instanceSize: The size of your CubeInstance struct (e.g., sizeof(CubeInstance))
    Mesh(float* vertices, size_t dataSize, std::vector<int> layout, size_t instanceSize) {
        isInstanced = true;
        setupBaseMesh(vertices, dataSize, layout);

        // Create and bind the Instance VBO
        glGenBuffers(1, &iVBO);
        glBindBuffer(GL_ARRAY_BUFFER, iVBO);

        // We initialize with NULL because we update this every frame in the loop
        glBufferData(GL_ARRAY_BUFFER, 1000 * instanceSize, NULL, GL_DYNAMIC_DRAW);

        // Attributes 2-5 are your Instance Data (Position, Scale, Color, Rotation)
        // These are hardcoded here to match your CubeInstance struct:
        // Attrib 2: Position (vec3)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, instanceSize, (void*)0);
        glVertexAttribDivisor(2, 1); // Only move to next data after full cube is drawn

        // Attrib 3: Scale (float)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, instanceSize, (void*)12);
        glVertexAttribDivisor(3, 1);

        // Attrib 4: Color (vec3)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, instanceSize, (void*)16);
        glVertexAttribDivisor(4, 1);

        // Attrib 5: Rotation (vec3)
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, instanceSize, (void*)28);
        glVertexAttribDivisor(5, 1);

        glBindVertexArray(0);
    }

    // Update the enemy data on the GPU every frame
    void updateInstances(void* data, size_t size) {
        if (!isInstanced) return;
        glBindBuffer(GL_ARRAY_BUFFER, iVBO);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }

    void draw(int count = 0, GLenum mode = GL_TRIANGLES) {
        glBindVertexArray(VAO);
        if (isInstanced && count > 0) {
            glDrawArraysInstanced(mode, 0, vertexCount, count);
        }
        else {
            glDrawArrays(mode, 0, vertexCount);
        }
    }

private:
    void setupBaseMesh(float* vertices, size_t dataSize, std::vector<int> layout) {
        int floatsPerVertex = std::accumulate(layout.begin(), layout.end(), 0);
        vertexCount = (int)(dataSize / (floatsPerVertex * sizeof(float)));

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, dataSize, vertices, GL_STATIC_DRAW);

        int stride = floatsPerVertex * sizeof(float);
        size_t offset = 0;
        for (int i = 0; i < layout.size(); i++) {
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, layout[i], GL_FLOAT, GL_FALSE, stride, (void*)offset);
            offset += layout[i] * sizeof(float);
        }
    }
};