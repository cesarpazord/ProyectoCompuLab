#pragma once
#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class PacmanMesh {
public:
    GLuint VAO = 0, VBO = 0, EBO = 0;
    int indexCount = 0;

    void build(float mouthAngle, int stacks = 30, int slices = 60) {

        // ✅ 1. LIMITAR ÁNGULO (evita romper la malla)
        mouthAngle = glm::clamp(mouthAngle, 1.0f, 60.0f);
        float halfMouth = glm::radians(mouthAngle);

        std::vector<float> verts;
        std::vector<unsigned int> idx;

        auto addVertex = [&](float theta, float phi) {
            float x = sin(theta) * cos(phi);
            float y = cos(theta);
            float z = sin(theta) * sin(phi);

            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);

            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);
            };

        int rows = stacks + 1;
        int cols = slices + 1;

        // ---- VÉRTICES ----
        for (int i = 0; i < rows; i++) {
            float theta = (float)i / stacks * (float)M_PI;

            for (int j = 0; j < cols; j++) {
                float phi = (float)j / slices * 2.0f * (float)M_PI - (float)M_PI;
                addVertex(theta, phi);
            }
        }

        // ---- ÍNDICES (QUITANDO LA BOCA) ----
        for (int i = 0; i < stacks; i++) {
            for (int j = 0; j < slices; j++) {

                int a = i * cols + j;
                int b = a + 1;
                int c = (i + 1) * cols + j;
                int d = c + 1;

                float phi = ((float)j + 0.5f) / slices * 2.0f * (float)M_PI - (float)M_PI;

                float dPhi = phi - (float)M_PI / 2.0f;

                while (dPhi > (float)M_PI) dPhi -= 2.0f * (float)M_PI;
                while (dPhi < -(float)M_PI) dPhi += 2.0f * (float)M_PI;

                bool inMouth = fabs(dPhi) < halfMouth;

                if (!inMouth) {
                    idx.push_back(a); idx.push_back(c); idx.push_back(b);
                    idx.push_back(b); idx.push_back(c); idx.push_back(d);
                }
            }
        }

        // ---- DISCOS DE LA BOCA ----
        buildMouthDisk(verts, idx, halfMouth, +1.0f);
        buildMouthDisk(verts, idx, halfMouth, -1.0f);

        // ✅ 2. VALIDACIÓN (evita crash)
        if (verts.empty() || idx.empty()) return;

        indexCount = (int)idx.size();

        // ---- GPU ----
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
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw() {
        if (VAO == 0 || indexCount == 0) return; // ✅ protección extra
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }

private:

    void buildMouthDisk(std::vector<float>& verts,
        std::vector<unsigned int>& idx,
        float halfMouth, float sign)
    {
        // ✅ NORMAL SEGURA (evita NaN)
        glm::vec3 norm = glm::normalize(glm::vec3(0.0f, -sign, 0.2f));

        int centerIdx = (int)(verts.size() / 6);

        verts.push_back(0.0f); verts.push_back(0.0f); verts.push_back(0.0f);
        verts.push_back(norm.x); verts.push_back(norm.y); verts.push_back(norm.z);

        int diskSlices = 40;
        int borderStart = (int)(verts.size() / 6);

        for (int j = 0; j <= diskSlices; j++) {

            float phi = ((float)M_PI / 2.0f - halfMouth) +
                (2.0f * halfMouth) * ((float)j / diskSlices);

            float px = cos(phi);
            float pz = sin(phi);
            float py = sign * sin(halfMouth) * (1.0f - fabs(cos(phi)));

            verts.push_back(px); verts.push_back(py); verts.push_back(pz);
            verts.push_back(norm.x); verts.push_back(norm.y); verts.push_back(norm.z);
        }

        for (int j = 0; j < diskSlices; j++) {
            idx.push_back(centerIdx);

            if (sign > 0) {
                idx.push_back(borderStart + j);
                idx.push_back(borderStart + j + 1);
            }
            else {
                idx.push_back(borderStart + j + 1);
                idx.push_back(borderStart + j);
            }
        }
    }
};