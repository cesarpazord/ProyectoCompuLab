#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "PacmanMesh.h"
#include "GhostMesh.h"

// ---- Prototipos ----
void KeyCallback(GLFWwindow*, int, int, int, int);
void MouseCallback(GLFWwindow*, double, double);
void ScrollCallback(GLFWwindow*, double, double);
void DoMovement();
void AnimateMouth();

bool isIntersection(glm::vec3 pos) {
    float gridSize = 1.0f;

    float x = round(pos.x / gridSize) * gridSize;
    float z = round(pos.z / gridSize) * gridSize;

    return (fabs(pos.x - x) < 0.1f && fabs(pos.z - z) < 0.1f);
}

// ---- Dimensiones ----
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// ---- Camara ----
Camera camera(glm::vec3(0.0f, 1.5f, 8.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;
bool keys[1024] = { false };

// ---- Tiempo ----
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
bool gameStarted = false;

////////
bool frightenedMode = false;
float frightenedTimer = 0.0f;
float frightenedDuration = 8.0f; // segundos como el juego real 


// ---- Estado Pac-Man ----
glm::vec3 pacStart = glm::vec3(0.0f, 0.5f, 3.0f);
glm::vec3 pacPos = pacStart; 
float     pacYaw = 0.0f;    // rotacion horizontal (grados)
float     pacPitch = 0.0f;    // rotacion vertical   (grados)
float     moveSpeed = 3.0f;
float     rotSpeed = 90.0f;   // grados/seg con flechas

// ---- Boca ----
float mouthAngle = 5.0f;    // apertura actual (grados)
float mouthTarget = 35.0f;   // apertura maxima
bool  mouthOpening = true;
bool  animMouth = true;

// ---- Luces ----
glm::vec3 pointLightPositions[] = {
    glm::vec3(3.0f,  4.0f,  2.0f),
    glm::vec3(-3.0f,  3.0f, -2.0f),
    glm::vec3(0.0f,  5.0f,  0.0f),
    glm::vec3(0.0f,  0.0f,  0.0f)
};

// ---- Fantasmas ----
glm::vec3 ghostPos[4] = {
    glm::vec3(-1.0f, 0.4f, -1.0f),
    glm::vec3(1.0f, 0.4f, -1.0f),
    glm::vec3(-1.0f, 0.4f,  1.0f),
    glm::vec3(1.0f, 0.4f,  1.0f)
};

// dirección actual
glm::vec3 ghostDirection[4];

// dirección anterior (para no regresar)
glm::vec3 ghostLastDir[4];

float ghostSpeed = 2.0f;
float floorLimit = 7.0f;

// Camino tipo grid (mismo que usas para bolitas)
const int GRID_SIZE = 21;
const float CELL_SIZE = 1.4f;

int maze[GRID_SIZE][GRID_SIZE] = {
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1},
{1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1},
{1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
{1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,1},
{1,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1},
{1,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,1},
{1,0,1,0,1,1,1,1,0,1,0,1,0,1,1,1,1,0,1,0,1},
{1,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1},

// 🔥 zona de fantasmas (sin pellets)
{1,1,1,1,1,0,1,1,1,1,2,1,1,1,1,0,1,1,1,1,1},

{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1},
{1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1},
{1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
{1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,1},
{1,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1},
{1,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,1},
{1,0,1,0,1,1,1,1,0,1,0,1,0,1,1,1,1,0,1,0,1},
{1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


// Cubo de vértices para representar las luces (posición + normal)
float cubeVertices[] = {
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

// ---- Plano del suelo (quad) ----
float floorVertices[] = {
    // pos              normal
    -15.0f,0.0f,-15.0f,  0,1,0,
     15.0f,0.0f,-15.0f,  0,1,0,
     15.0f,0.0f, 15.0f,  0,1,0,
     15.0f,0.0f, 15.0f,  0,1,0,
    -15.0f,0.0f, 15.0f,  0,1,0,
    -15.0f,0.0f,-15.0f,  0,1,0,
};

// ============================================================
//  HELPER: envía todas las luces al shader
// ============================================================
void SetLights(Shader& shader) {
    // Direccional (sol)
    glUniform3f(glGetUniformLocation(shader.Program, "dirLight.direction"), -0.3f, -1.0f, -0.5f);
    glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"), 0.25f, 0.25f, 0.2f);
    glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"), 0.7f, 0.7f, 0.6f);
    glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"), 0.4f, 0.4f, 0.4f);

    // Punto 0 - cálido
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.1f, 0.08f, 0.0f);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 1.0f, 0.9f, 0.4f);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 0.8f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.032f);

    // Punto 1 - frio/azul
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.0f, 0.0f, 0.05f);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].diffuse"), 0.3f, 0.4f, 1.0f);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].specular"), 0.5f, 0.5f, 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.032f);

    // Puntos 2 y 3 - apagados (usar si se quieren más)
    for (int i = 2; i < 4; i++) {
        std::string pl = "pointLights[" + std::to_string(i) + "]";
        glUniform3f(glGetUniformLocation(shader.Program, (pl + ".position").c_str()), 0, 0, 0);
        glUniform3f(glGetUniformLocation(shader.Program, (pl + ".ambient").c_str()), 0, 0, 0);
        glUniform3f(glGetUniformLocation(shader.Program, (pl + ".diffuse").c_str()), 0, 0, 0);
        glUniform3f(glGetUniformLocation(shader.Program, (pl + ".specular").c_str()), 0, 0, 0);
        glUniform1f(glGetUniformLocation(shader.Program, (pl + ".constant").c_str()), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, (pl + ".linear").c_str()), 0.0f);
        glUniform1f(glGetUniformLocation(shader.Program, (pl + ".quadratic").c_str()), 0.0f);
    }

    // Spotlight (linterna de la cámara, azulada)
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.1f);
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.specular"), 0.1f, 0.1f, 0.5f);
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.quadratic"), 0.032f);
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(17.5f)));
}

// ============================================================
//  PELLETS (CAMINOS TIPO PAC-MAN)
// ============================================================
std::vector<glm::vec3> pellets;
std::vector<bool> pelletEaten;

void InitPellets() {

    float y = 0.15f;
    float step = 1.0f;

    float min = -13.0f;
    float max = 13.0f;

    // =================================================
    // BORDES (como el mapa clásico)
    // =================================================
    for (float x = min; x <= max; x += step) {
        pellets.push_back(glm::vec3(x, y, min));
        pellets.push_back(glm::vec3(x, y, max));
    }

    for (float z = min; z <= max; z += step) {
        pellets.push_back(glm::vec3(min, y, z));
        pellets.push_back(glm::vec3(max, y, z));
    }

    // =================================================
    // CORREDORES HORIZONTALES
    // =================================================
    float rows[] = { -10, -5, 0, 5, 10 };

    for (float z : rows) {
        for (float x = min + 2; x <= max - 2; x += step) {
            pellets.push_back(glm::vec3(x, y, z));
        }
    }

    // =================================================
    // CORREDORES VERTICALES
    // =================================================
    float cols[] = { -10, -5, 0, 5, 10 };

    for (float x : cols) {
        for (float z = min + 2; z <= max - 2; z += step) {
            pellets.push_back(glm::vec3(x, y, z));
        }
    }

    // =================================================
    // POWER PELLETS (las grandes)
    // =================================================
    pellets.push_back(glm::vec3(-12, y, -12));
    pellets.push_back(glm::vec3(12, y, -12));
    pellets.push_back(glm::vec3(-12, y, 12));
    pellets.push_back(glm::vec3(12, y, 12));

    pelletEaten.resize(pellets.size(), false);
}
// ============================================================
//  MAIN
// ============================================================
int main()
{
    // ---- Init GLFW ----
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Pac-Man 3D", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // ---- Shaders ----
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

    // ---- Geometría del cubo de luz ----
    GLuint cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // ---- Geometría del suelo ----
    GLuint floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // ---- Pac-Man mesh procedural ----
    PacmanMesh pacman;
    pacman.build(mouthAngle);

    // ---- Fantasmas ----
    GhostMesh ghost;
    ghost.build(64, 50);

    InitPellets(); 
    // posiciones de los 4 fantasmas
    
    for (int i = 0; i < 4; i++) {
        ghostDirection[i] = glm::vec3(1, 0, 0);
        ghostLastDir[i] = glm::vec3(1, 0, 0);
    }

    // colores tipo clásico
    glm::vec3 ghostColors[] = {
        glm::vec3(1.0f, 0.2f, 0.2f),  // rojo
        glm::vec3(0.2f, 1.0f, 1.0f),  // cyan
        glm::vec3(1.0f, 0.4f, 1.0f),  // rosa
        glm::vec3(1.0f, 0.6f, 0.2f)   // naranja
    };

    // ---- Proyeccion ----
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.GetZoom()),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f, 100.0f
    );

    // ============================================================
    //  GAME LOOP
    // ============================================================
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (frightenedMode) {
            frightenedTimer -= deltaTime;

            if (frightenedTimer <= 0.0f) {
                frightenedMode = false;
            }
        }

        glfwPollEvents();
        DoMovement();
        AnimateMouth();

        // ---- Movimiento automático de fantasmas ----
        if (gameStarted) {
            for (int i = 0; i < 4; i++) {

                // 🔹 convertir a grid REAL
                int gi = (ghostPos[i].z + (GRID_SIZE * CELL_SIZE) / 2) / CELL_SIZE;
                int gj = (ghostPos[i].x + (GRID_SIZE * CELL_SIZE) / 2) / CELL_SIZE;

                int pi = (pacPos.z + (GRID_SIZE * CELL_SIZE) / 2) / CELL_SIZE;
                int pj = (pacPos.x + (GRID_SIZE * CELL_SIZE) / 2) / CELL_SIZE;

                std::vector<glm::vec3> options;

                std::vector<glm::vec3> dirs = {
                    {1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}
                };

                // 🔹 detectar caminos válidos
                for (auto& d : dirs) {

                    int ni = gi + (int)d.z;
                    int nj = gj + (int)d.x;

                    if (ni >= 0 && ni < GRID_SIZE &&
                        nj >= 0 && nj < GRID_SIZE &&
                        maze[ni][nj] == 0)
                    {
                        // 🚫 evitar regresar atrás (solo una vez)
                        if (d.x == -ghostLastDir[i].x && d.z == -ghostLastDir[i].z)
                            continue;

                        options.push_back(d);
                    }
                }

                // 🔹 detectar intersección REAL (usando CELL_SIZE)
                bool enNodo =
                    fabs(fmod(ghostPos[i].x, CELL_SIZE)) < 0.2f &&
                    fabs(fmod(ghostPos[i].z, CELL_SIZE)) < 0.2f;

                if (!options.empty() && enNodo)
                {
                    glm::vec3 bestDir = options[0];
                    float bestDist = 9999.0f;

                    for (auto& d : options) {

                        int ni = gi + (int)d.z;
                        int nj = gj + (int)d.x;

                        float dist = abs(ni - pi) + abs(nj - pj);

                        if (i == 0) { // rojo (persigue)
                            if (dist < bestDist) {
                                bestDist = dist;
                                bestDir = d;
                            }
                        }
                        else if (i == 1) { // azul (random)
                            bestDir = options[rand() % options.size()];
                        }
                        else if (i == 2) { // rosa (horizontal)
                            if (abs(d.x) > 0) bestDir = d;
                        }
                        else { // naranja (semi-random)
                            if (rand() % 2 == 0)
                                bestDir = d;
                        }
                    }

                    ghostLastDir[i] = ghostDirection[i];
                    ghostDirection[i] = bestDir;
                }

                // 🔹 movimiento validado
                glm::vec3 nextPos = ghostPos[i] + ghostDirection[i] * ghostSpeed * deltaTime;

                int ngi = (nextPos.z + (GRID_SIZE * CELL_SIZE) / 2) / CELL_SIZE;
                int ngj = (nextPos.x + (GRID_SIZE * CELL_SIZE) / 2) / CELL_SIZE;

                if (ngi >= 0 && ngi < GRID_SIZE &&
                    ngj >= 0 && ngj < GRID_SIZE &&
                    maze[ngi][ngj] == 0)
                {
                    ghostPos[i] = nextPos;
                }
            }
        }



        // Rebuild mesh solo si la boca cambio significativamente
        static float lastMouth = -1.0f;
        if (fabs(mouthAngle - lastMouth) > 0.3f) {
            pacman.build(mouthAngle);
            lastMouth = mouthAngle;
        }

        // ---- Clear ----
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---- Matrices comunes ----
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // ================================================================
        //  RENDER CON LIGHTING SHADER
        // ================================================================
        lightingShader.Use();

        // Cámara y luces
        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        SetLights(lightingShader);

        // Matrices de transformacion
        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

        // ---- SUELO ----
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.01f, 0.1f, 0.1f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.1f, 0.1f, 0.15f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 8.0f);

        model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // ---- PELLETS ----
        for (size_t i = 0; i < pellets.size(); i++) {

            if (pelletEaten[i]) continue;

            int gi = (pellets[i].z + (GRID_SIZE * CELL_SIZE) / 2) / CELL_SIZE;
            int gj = (pellets[i].x + (GRID_SIZE * CELL_SIZE) / 2) / CELL_SIZE;

            // 🚫 NO dibujar pellets en muros ni zona de fantasmas
            if (maze[gi][gj] != 0) continue;

            float dist = glm::length(pacPos - pellets[i]);
            if (dist < 0.5f) {
                pelletEaten[i] = true;

                // 🔥 detectar si es power pellet (las grandes)
                if (i % 12 == 0) {
                    frightenedMode = true;
                    frightenedTimer = frightenedDuration;
                }

                continue;
            }

            float scale = 0.12f;  // pequeñas
            if (i % 12 == 0) scale = 0.38f; // grandes

            // Color
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.5f, 0.45f, 0.1f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 1.0f, 0.9f, 0.2f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.4f, 0.4f, 0.2f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

            glm::mat4 pelletModel = glm::mat4(1.0f);
            pelletModel = glm::translate(pelletModel, pellets[i]);
            pelletModel = glm::scale(pelletModel, glm::vec3(scale));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pelletModel));

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ---- PAC-MAN ----
        // Material amarillo brillante
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.4f, 0.35f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 1.0f, 0.85f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.2f, 0.2f, 0.2f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 8.0f);

        model = glm::mat4(1.0f);
        // Posicion del pac-man
        model = glm::translate(model, pacPos + glm::vec3(0.0f, 0.6f, 0.0f));
        // Rotacion por teclado (yaw horizontal)
        model = glm::rotate(model, glm::radians(pacYaw), glm::vec3(0.0f, 2.0f, 0.0f));
        // Rotacion vertical (pitch)
        model = glm::rotate(model, glm::radians(pacPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        // Boca apunta hacia +X: primero -90 en X (boca al frente), luego -90 en Y (gira a +X)
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // Escala uniforme
        model = glm::scale(model, glm::vec3(1.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        pacman.draw();

        // ---- FANTASMAS ----
        for (int i = 0; i < 4; i++) {

            glm::vec3 color;

            // 🔵 modo asustado
            if (frightenedMode) {
                color = glm::vec3(0.1f, 0.1f, 1.0f);

                // ⚡ PARPADEO
                if (frightenedTimer < 2.0f) {
                    if ((int)(frightenedTimer * 10) % 2 == 0)
                        color = glm::vec3(1.0f, 1.0f, 1.0f);
                }
            }
            else {
                color = ghostColors[i];
            }

            // MATERIAL
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"),
                color.r * 0.3f,
                color.g * 0.3f,
                color.b * 0.3f);

            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"),
                color.r,
                color.g,
                color.b);

            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"),
                0.9f, 0.9f, 0.9f);

            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

            // MODEL 
            glm::mat4 ghostModel = glm::mat4(1.0f);

            // SHAKE
            float shakeX = 0.0f;
            float shakeZ = 0.0f;
            float shakeY = 0.0f;

            if (frightenedMode) {
                shakeX = sin(glfwGetTime() * 8.0f + i) * 0.01f;
                shakeZ = cos(glfwGetTime() * 8.0f + i) * 0.01f;
                shakeY = sin(glfwGetTime() * 10.0f + i) * 0.05f;
            }

            // 🔁 TRANSLATE
            ghostModel = glm::translate(
                ghostModel,
                ghostPos[i] + glm::vec3(shakeX, 0.4f + shakeY, shakeZ)
            );
            //Tamaño fantasmas 
            ghostModel = glm::scale(ghostModel, glm::vec3(0.7f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ghostModel));

            ghost.draw();
      

            // ---- OJOS DEL FANTASMA ----

// blanco del ojo
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.8f, 0.8f, 0.8f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 1.0f, 1.0f, 1.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.9f, 0.9f, 0.9f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 64.0f);

            // OJO IZQUIERDO
            glm::mat4 eyeModel = ghostModel;
            eyeModel = glm::translate(eyeModel, glm::vec3(0.3f, 0.5f, 0.9f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.22f, 0.28f, 0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // OJO DERECHO
            eyeModel = ghostModel;
            eyeModel = glm::translate(eyeModel, glm::vec3(-0.3f, 0.5f, 0.9f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.22f, 0.28f, 0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // ---- PUPILAS (NEGRAS estilo Pac-Man) ----
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.0f, 0.0f, 0.2f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.0f, 0.0f, 1.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.5f, 0.5f, 0.2f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

            // pupila izquierda
            eyeModel = ghostModel;
            eyeModel = glm::translate(eyeModel, glm::vec3(0.3f, 0.5f, 1.0f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.08f, 0.12f, 0.08f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // pupila derecha
            eyeModel = ghostModel;
            eyeModel = glm::translate(eyeModel, glm::vec3(-0.3f, 0.5f, 1.0f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.08f, 0.12f, 0.08f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // ---- INTERIOR NEGRO DE LA BOCA ----
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);

        glm::mat4 mouthModel = model;
        mouthModel = glm::scale(mouthModel, glm::vec3(0.95f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mouthModel));
        pacman.draw();

        // ---- OJO de Pac-Man ----
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.02f, 0.02f, 0.02f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.8f, 0.8f, 0.8f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 128.0f);

        // Ojo lado derecho
        glm::mat4 eyeModel = model;
        eyeModel = glm::translate(eyeModel, glm::vec3(0.6f, 0.7f, 0.6f));
        eyeModel = glm::scale(eyeModel, glm::vec3(0.15f, 0.22f, 0.15f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Ojo lado izquierdo

        eyeModel = model;
        eyeModel = glm::translate(eyeModel, glm::vec3(0.6f, -0.7f, 0.6f));
        eyeModel = glm::scale(eyeModel, glm::vec3(0.15f, 0.22f, 0.15f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // ================================================================
        //  RENDER LAMP SHADER (cubos de luces)
        // ================================================================
        lampShader.Use();
        GLint lModelLoc = glGetUniformLocation(lampShader.Program, "model");
        GLint lViewLoc = glGetUniformLocation(lampShader.Program, "view");
        GLint lProjLoc = glGetUniformLocation(lampShader.Program, "projection");
        glUniformMatrix4fv(lViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(lProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(cubeVAO);
        for (int i = 0; i < 2; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.15f));
            glUniformMatrix4fv(lModelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // ---- Swap ----
        glfwSwapBuffers(window);
    }

    pacman.cleanup();
    ghost.cleanup(); 
    glfwTerminate();
    return 0;
}

// ============================================================
//  MOVIMIENTO
// ============================================================
void DoMovement()
{
    // Camara libre con WASD
    if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);

    float speed = moveSpeed * deltaTime;

    // Flechas: mueven Pac-Man en el plano XZ y rotan la boca
    // hacia donde va — igual que el Pac-Man clásico
    if (keys[GLFW_KEY_LEFT]) {
        pacPos.x -= speed;
        pacYaw = 0.0f;
        gameStarted = true;
    }
    if (keys[GLFW_KEY_RIGHT]) {
        pacPos.x += speed;
        pacYaw = 180.0f;
        gameStarted = true;
        
    }
        if (keys[GLFW_KEY_DOWN]) {
            pacPos.z += speed;
            pacYaw = 90.0f;
            gameStarted = true;
    }
        if (keys[GLFW_KEY_UP]) {
            pacPos.z -= speed;
            pacYaw = 270.0f;
            gameStarted = true;
        
    }

    // Reset
    if (keys[GLFW_KEY_R]) {
        pacPos = glm::vec3(0.0f, 0.5f, 0.0f);
        pacYaw = 0.0f;
    }
}

// ============================================================
//  ANIMACION DE LA BOCA
// ============================================================
void AnimateMouth()
{
    if (!animMouth) {
        mouthAngle = 5.0f;
        return;
    }

    float speed = 80.0f * deltaTime;  // grados/seg

    if (mouthOpening) {
        mouthAngle += speed;
        if (mouthAngle >= mouthTarget) {
            mouthAngle = mouthTarget;
            mouthOpening = false;
        }
    }
    else {
        mouthAngle -= speed;
        if (mouthAngle <= 2.0f) {
            mouthAngle = 2.0f;
            mouthOpening = true;
        }
    }
}

glm::vec3 GridToWorld(int i, int j) {
    float offset = (GRID_SIZE * CELL_SIZE) / 2.0f;
    return glm::vec3(
        j * CELL_SIZE - offset,
        0.4f,
        i * CELL_SIZE - offset
    );
}

// ============================================================
//  CALLBACKS
// ============================================================
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        if (action == GLFW_RELEASE) keys[key] = false;
    }

    // Toggle boca
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        animMouth = !animMouth;
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) {
        lastX = (float)xPos;
        lastY = (float)yPos;
        firstMouse = false;
    }
    float xOffset = (float)xPos - lastX;
    float yOffset = lastY - (float)yPos;
    lastX = (float)xPos;
    lastY = (float)yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}