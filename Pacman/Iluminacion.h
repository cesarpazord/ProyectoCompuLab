#ifndef ILUMINACION_H
#define ILUMINACION_H

#include <glm/glm.hpp>

struct Luz {
    glm::vec3 posicion;
    glm::vec3 color;

    float ambient;
    float diffuse;
    float specular;

    Luz() {
        posicion = glm::vec3(1.2f, 1.0f, 2.0f);
        color = glm::vec3(1.0f);

        ambient = 0.2f;
        diffuse = 0.5f;
        specular = 1.0f;
    }
};

#endif