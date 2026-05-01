#pragma once
#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class GhostMesh {
public:
    GLuint VAO = 0, VBO = 0, EBO = 0;
    int indexCount = 0;

    void build(int slices = 32, int stacks = 20) {
        std::vector<float> verts;
        std::vector<unsigned int> idx;

        const float radius = 1.0f;
        const float bodyHeight = 1.0f;
        const float height = bodyHeight + radius;
        const int waves = 4;
        const float waveAmp = 0.2f;

        int cols = slices + 1;

        // =====================================================
        // CUERPO COMPLETO (cilindro + cúpula suave)
        // =====================================================
        for (int i = 0; i <= stacks; i++) {
    float v = (float)i / stacks;

    float y, r;

    // 🔥 separar claramente cilindro y media esfera
    if (v < 0.5f) {
        // ---- CUERPO (cilindro) ----
        float t = v / 0.5f;
        y = bodyHeight * (1.0f - t);
        r = radius;
    }
    else {
        // ---- CABEZA (MEDIA ESFERA PERFECTA) ----
        float t = (v - 0.5f) / 0.5f;

        float theta = t * (M_PI / 2.0f); // 0 → 90°

        r = radius * cos(theta);
        y = bodyHeight + radius * sin(theta);
    }
            for (int j = 0; j <= slices; j++) {
                float phi = (float)j / slices * 2.0f * M_PI;

                float x = r * cos(phi);
                float z = r * sin(phi);

                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);

                // normales suaves
                verts.push_back(x);
                verts.push_back(0.5f);
                verts.push_back(z);
            }
        }

        // Índices del cuerpo
        for (int i = 0; i < stacks; i++) {
            for (int j = 0; j < slices; j++) {
                int a = i * cols + j;
                int b = a + 1;
                int c = (i + 1) * cols + j;
                int d = c + 1;

                idx.push_back(a); idx.push_back(c); idx.push_back(b);
                idx.push_back(b); idx.push_back(c); idx.push_back(d);
            }
        }

        // =====================================================
        // BASE ONDULADA (DIENTES CLÁSICOS)
        // =====================================================
        int baseCenter = verts.size() / 6;

        // centro
        verts.push_back(0);
        verts.push_back(0);
        verts.push_back(0);

        verts.push_back(0);
        verts.push_back(-1);
        verts.push_back(0);

        int start = verts.size() / 6;

        for (int j = 0; j <= slices; j++) {
            float phi = (float)j / slices * 2.0f * M_PI;

            float wave = sin(phi * waves) * waveAmp;

            float x = radius * cos(phi);
            float z = radius * sin(phi);
            float y = 0.0f + wave;

            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);

            verts.push_back(0);
            verts.push_back(-1);
            verts.push_back(0);
        }

        for (int j = 0; j < slices; j++) {
            idx.push_back(baseCenter);
            idx.push_back(start + j + 1);
            idx.push_back(start + j);
        }

        indexCount = idx.size();

        // =====================================================
        // GPU
        // =====================================================
        if (VAO) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
}; 