#include <iostream>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h" 

#include "Shader.h"
#include "Camera.h"
#include "Model.h"         
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
Camera camera(glm::vec3(0.0f, 8.0f, 15.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;
bool keys[1024] = { false };

// ---- Tiempo ----
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
bool gameStarted = false;

// ---- MODO DE CÁMARA ----
bool firstPersonMode = false;
bool followCameraMode = false;

// ---- ANIMACIÓN DE MUERTE ----
bool isPacmanDying = false;
float deathTimer = 0.0f;
float pacScale = 1.0f;

bool frightenedMode = false;
float frightenedTimer = 0.0f;
float frightenedDuration = 8.0f;

// ---- Estado Pac-Man ----
glm::vec3 pacStart = glm::vec3(-0.0f, 0.3f, 2.8f);
glm::vec3 pacPos = pacStart;
float     pacYaw = 0.0f;
float     pacPitch = 0.0f;
float     moveSpeed = 5.0f;
float     rotSpeed = 90.0f;

// ---- Boca ----
float mouthAngle = 5.0f;
float mouthTarget = 35.0f;
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
    glm::vec3(-2.8f, 0.4f, -7.5f),
    glm::vec3(3.2f, 0.4f, -7.5f),
    glm::vec3(1.2f, 0.4f, -7.5f),
    glm::vec3(-0.8f, 0.4f, -7.5f)
};

glm::vec3 ghostDirection[4];
glm::vec3 ghostLastDir[4];

float ghostSpeed = 2.0f;
float floorLimit = 7.0f;

// ---- MATRIZ ORIGINAL ----
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

// Cubo (Solo 6 valores: XYZ, Normales XYZ)
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

// Piso (8 valores: XYZ, Normales XYZ, UV TexCoords)
float floorVertices[] = {
    // Posiciones          // Normales       // UVs
    -15.0f, 0.0f, -15.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     15.0f, 0.0f, -15.0f,  0.0f, 1.0f, 0.0f,  10.0f, 0.0f,
     15.0f, 0.0f,  15.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
     15.0f, 0.0f,  15.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
    -15.0f, 0.0f,  15.0f,  0.0f, 1.0f, 0.0f,  0.0f, 10.0f,
    -15.0f, 0.0f, -15.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f
};

void SetLights(Shader& shader, glm::vec3 currentCamPos, glm::vec3 currentCamFront) {
    glUniform3f(glGetUniformLocation(shader.Program, "dirLight.direction"), -0.3f, -1.0f, -0.5f);
    glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"), 0.7f, 0.7f, 0.6f);
    glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"), 0.4f, 0.4f, 0.4f);

    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.1f, 0.08f, 0.0f);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 0.4f, 0.9f, 0.4f);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 0.8f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.032f);

    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.0f, 0.0f, 0.05f);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].diffuse"), 0.3f, 0.4f, 1.0f);
    glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].specular"), 0.5f, 0.5f, 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.032f);

    // Linterna apuntando hacia donde mira la cámara
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.position"), currentCamPos.x, currentCamPos.y, currentCamPos.z);
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.direction"), currentCamFront.x, currentCamFront.y, currentCamFront.z);
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.1f);
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
    glUniform3f(glGetUniformLocation(shader.Program, "spotLight.specular"), 0.1f, 0.1f, 0.5f);
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.quadratic"), 0.032f);
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
    glUniform1f(glGetUniformLocation(shader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(17.5f)));
}

std::vector<glm::vec3> pellets;
std::vector<bool> pelletEaten;

void InitPellets() {
    float y = 0.15f;
    float offset = (GRID_SIZE * CELL_SIZE) / 2.0f;
    pellets.clear();

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (maze[i][j] != 1) {
                float worldX = j * CELL_SIZE - offset;
                float worldZ = i * CELL_SIZE - offset;
                pellets.push_back(glm::vec3(worldX, y, worldZ));
            }
        }
    }
    pelletEaten.resize(pellets.size(), false);
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_image_free(data);
    }
    else {
        std::cout << "Error al cargar textura en: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

int main()
{
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

    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

    Model mapaLaberinto((char*)"Models/Paredes.obj");

    // VAO DEL CUBO: Solo tiene 6 floats de stride (X,Y,Z, NX,NY,NZ)
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

    // VAO DEL PISO: Tiene 8 floats de stride (X,Y,Z, NX,NY,NZ, U,V)
    GLuint floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    PacmanMesh pacman;
    pacman.build(mouthAngle);

    GhostMesh ghost;
    ghost.build(64, 50);

    InitPellets();

    for (int i = 0; i < 4; i++) {
        ghostDirection[i] = glm::vec3(1, 0, 0);
        ghostLastDir[i] = glm::vec3(1, 0, 0);
    }

    glm::vec3 ghostColors[] = {
        glm::vec3(1.0f, 0.2f, 0.2f),
        glm::vec3(0.2f, 1.0f, 1.0f),
        glm::vec3(1.0f, 0.4f, 1.0f),
        glm::vec3(1.0f, 0.6f, 0.2f)
    };

    glm::mat4 projection = glm::perspective(
        glm::radians(camera.GetZoom()),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f, 100.0f
    );

    // Cargar la textura
    unsigned int textureID = loadTexture("Models/Pared.png");

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

        // ============================================================
        //  COLISIONES Y ANIMACIÓN DE MUERTE
        // ============================================================
        for (int i = 0; i < 4; i++) {
            float distancia = glm::length(pacPos - ghostPos[i]);
            float radioColision = 0.9f;

            if (distancia < radioColision && !isPacmanDying) {
                if (frightenedMode) {
                    ghostPos[i] = glm::vec3(-1.0f, 0.4f, -1.0f);
                }
                else {
                    isPacmanDying = true;
                    deathTimer = 0.0f;
                    gameStarted = false;
                }
            }
        }

        if (isPacmanDying) {
            deathTimer += deltaTime;
            pacYaw += 720.0f * deltaTime;
            pacPitch += 180.0f * deltaTime;
            pacScale = 1.0f - (deathTimer / 1.5f);
            if (pacScale < 0.0f) pacScale = 0.0f;

            if (deathTimer > 1.5f) {
                isPacmanDying = false;
                pacScale = 1.0f;
                pacPos = glm::vec3(-0.0f, 0.3f, 2.8f);
                pacYaw = 0.0f;
                pacPitch = 0.0f;
                firstPersonMode = false;

                ghostPos[0] = glm::vec3(-2.8f, 0.4f, -7.5f);
                ghostPos[1] = glm::vec3(3.2f, 0.4f, -7.5f);
                ghostPos[2] = glm::vec3(1.2f, 0.4f, -7.5f);
                ghostPos[3] = glm::vec3(-0.8f, 0.4f, -7.5f);
            }
        }

        // ---- MOVIMIENTO FANTASMAS ----
        if (gameStarted) {
            for (int i = 0; i < 4; i++) {

                float offset = (GRID_SIZE * CELL_SIZE) / 2.0f;
                int gi = (ghostPos[i].z + offset) / CELL_SIZE;
                int gj = (ghostPos[i].x + offset) / CELL_SIZE;
                int pi = (pacPos.z + offset) / CELL_SIZE;
                int pj = (pacPos.x + offset) / CELL_SIZE;

                std::vector<glm::vec3> options;
                std::vector<glm::vec3> dirs = { {1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1} };

                for (auto& d : dirs) {
                    int ni = gi + (int)d.z;
                    int nj = gj + (int)d.x;

                    if (ni >= 0 && ni < GRID_SIZE && nj >= 0 && nj < GRID_SIZE && maze[ni][nj] == 0) {
                        if (d.x == -ghostLastDir[i].x && d.z == -ghostLastDir[i].z) continue;
                        options.push_back(d);
                    }
                }

                if (options.empty()) {
                    options.push_back(-ghostLastDir[i]);
                }

                float centerZ = gi * CELL_SIZE - offset;
                float centerX = gj * CELL_SIZE - offset;
                bool enNodo = (fabs(ghostPos[i].x - centerX) < 0.15f && fabs(ghostPos[i].z - centerZ) < 0.15f);

                glm::vec3 testPos = ghostPos[i] + ghostDirection[i] * ghostSpeed * deltaTime;
                int testI = (testPos.z + offset) / CELL_SIZE;
                int testJ = (testPos.x + offset) / CELL_SIZE;
                if (maze[testI][testJ] != 0) {
                    enNodo = true;
                }

                if (enNodo) {
                    glm::vec3 bestDir = options[0];
                    float bestDist = 9999.0f;

                    for (auto& d : options) {
                        int ni = gi + (int)d.z;
                        int nj = gj + (int)d.x;
                        float dist = abs(ni - pi) + abs(nj - pj);

                        if (i == 0) {
                            if (dist < bestDist) { bestDist = dist; bestDir = d; }
                        }
                        else if (i == 1) {
                            bestDir = options[rand() % options.size()];
                        }
                        else if (i == 2) {
                            if (abs(d.x) > 0) bestDir = d;
                        }
                        else {
                            if (rand() % 2 == 0) bestDir = d;
                        }
                    }
                    ghostLastDir[i] = ghostDirection[i];
                    ghostDirection[i] = bestDir;
                }

                glm::vec3 nextPos = ghostPos[i] + ghostDirection[i] * ghostSpeed * deltaTime;
                int ngi = (nextPos.z + offset) / CELL_SIZE;
                int ngj = (nextPos.x + offset) / CELL_SIZE;

                if (ngi >= 0 && ngi < GRID_SIZE && ngj >= 0 && ngj < GRID_SIZE && maze[ngi][ngj] == 0) {
                    ghostPos[i] = nextPos;

                    float lerpSpeed = 15.0f * deltaTime;
                    if (ghostDirection[i].x != 0.0f) {
                        float idealZ = round((ghostPos[i].z + offset) / CELL_SIZE) * CELL_SIZE - offset;
                        ghostPos[i].z = glm::mix(ghostPos[i].z, idealZ, lerpSpeed);
                    }
                    else if (ghostDirection[i].z != 0.0f) {
                        float idealX = round((ghostPos[i].x + offset) / CELL_SIZE) * CELL_SIZE - offset;
                        ghostPos[i].x = glm::mix(ghostPos[i].x, idealX, lerpSpeed);
                    }
                }
            }
        }

        static float lastMouth = -1.0f;
        if (fabs(mouthAngle - lastMouth) > 0.3f) {
            pacman.build(mouthAngle);
            lastMouth = mouthAngle;
        }

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ================================================================
        //  SISTEMA DE CÁMARA
        // ================================================================
        glm::mat4 view;
        glm::vec3 currentCamPos;
        glm::vec3 currentCamFront;

        if (firstPersonMode) {

            currentCamPos = pacPos + glm::vec3(0.0f, 1.0f * pacScale, 0.0f);

            currentCamFront.x = -cos(glm::radians(pacYaw)) * cos(glm::radians(pacPitch));
            currentCamFront.y = sin(glm::radians(pacPitch));
            currentCamFront.z = sin(glm::radians(pacYaw)) * cos(glm::radians(pacPitch));
            currentCamFront = glm::normalize(currentCamFront);

            view = glm::lookAt(
                currentCamPos,
                currentCamPos + currentCamFront,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        }
        else if (followCameraMode) {

            glm::vec3 front;

            front.x = -cos(glm::radians(pacYaw));
            front.y = 0.0f;
            front.z = sin(glm::radians(pacYaw));
            front = glm::normalize(front);

            currentCamPos = pacPos - front * 4.0f + glm::vec3(0.0f, 3.0f, 0.0f);

            currentCamFront = glm::normalize(pacPos - currentCamPos);

            view = glm::lookAt(
                currentCamPos,
                pacPos,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        }
        else {

            view = camera.GetViewMatrix();
            currentCamPos = camera.GetPosition();
            currentCamFront = camera.GetFront();
        }

        glm::mat4 model = glm::mat4(1.0f);

        lightingShader.Use();

        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), currentCamPos.x, currentCamPos.y, currentCamPos.z);
        SetLights(lightingShader, currentCamPos, currentCamFront);

        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

        // ============================================================
        // ---- DIBUJAR LABERINTO Y PISO CON TEXTURAS ----
        // ============================================================
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.1f, 0.1f, 0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.2f, 0.2f, 0.8f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

        // ACTIVAR TEXTURA
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.useTexture"), 1);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "texture_diffuse1"), 0);

        // Render Paredes
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.2f, -4.2f));
        model = glm::scale(model, glm::vec3(1.25f, 0.8f, 1.15f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mapaLaberinto.Draw(lightingShader);

        // Render Piso
        glBindVertexArray(floorVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.2f, -4.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // ============================================================
        // ---- DIBUJAR PERSONAJES Y PELLETS (SIN TEXTURAS) ----
        // ============================================================
        glUniform1i(glGetUniformLocation(lightingShader.Program, "material.useTexture"), 0); // Desactivar textura

        // ---- PELLETS ----
        for (size_t i = 0; i < pellets.size(); i++) {
            if (pelletEaten[i]) continue;

            float offset = (GRID_SIZE * CELL_SIZE) / 2.0f;
            int gi = round((pellets[i].z + offset) / CELL_SIZE);
            int gj = round((pellets[i].x + offset) / CELL_SIZE);

            bool isPowerPellet = false;
            if ((gi == 1 && gj == 1) || (gi == 15 && gj == 15) || (gi == 3 && gj == 15) ||
                (gi == 11 && gj == 1) || (gi == 7 && gj == 10)) {
                isPowerPellet = true;
            }

            float scale = isPowerPellet ? 0.42f : 0.16f;

            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.5f, 0.45f, 0.1f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 1.0f, 0.9f, 0.2f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.4f, 0.4f, 0.2f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.color"), 1.0f, 0.9f, 0.2f);

            glm::mat4 pelletModel = glm::mat4(1.0f);
            pelletModel = glm::translate(pelletModel, pellets[i]);
            pelletModel = glm::scale(pelletModel, glm::vec3(scale));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pelletModel));

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ---- PAC-MAN ----
        if (!firstPersonMode) {
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 8.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.color"), 1.0f, 0.85f, 0.0f);

            model = glm::mat4(1.0f);
            model = glm::translate(model, pacPos + glm::vec3(0.0f, 0.6f * pacScale, 0.0f));

            model = glm::rotate(model, glm::radians(pacYaw), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(pacPitch), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            model = glm::scale(model, glm::vec3(pacScale));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            pacman.draw();

            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.color"), 0.0f, 0.0f, 0.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 2.0f);

            glm::mat4 mouthModel = model;
            mouthModel = glm::scale(mouthModel, glm::vec3(0.95f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mouthModel));
            pacman.draw();

            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.color"), 0.02f, 0.02f, 0.02f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 128.0f);

            glm::mat4 eyeModel = model;
            eyeModel = glm::translate(eyeModel, glm::vec3(0.6f, 0.7f, 0.6f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.15f, 0.22f, 0.15f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            eyeModel = model;
            eyeModel = glm::translate(eyeModel, glm::vec3(0.6f, -0.7f, 0.6f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.15f, 0.22f, 0.15f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }

        // ---- FANTASMAS ----
        for (int i = 0; i < 4; i++) {
            glm::vec3 color;
            if (frightenedMode) {
                color = glm::vec3(0.1f, 0.1f, 1.0f);
                if (frightenedTimer < 2.0f && ((int)(frightenedTimer * 10) % 2 == 0)) {
                    color = glm::vec3(1.0f, 1.0f, 1.0f);
                }
            }
            else {
                color = ghostColors[i];
            }

            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.color"), color.r, color.g, color.b);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

            glm::mat4 ghostModel = glm::mat4(1.0f);
            float shakeX = 0.0f, shakeZ = 0.0f, shakeY = 0.0f;

            if (frightenedMode) {
                shakeX = sin(glfwGetTime() * 8.0f + i) * 0.01f;
                shakeZ = cos(glfwGetTime() * 8.0f + i) * 0.01f;
                shakeY = sin(glfwGetTime() * 10.0f + i) * 0.05f;
            }

            ghostModel = glm::translate(ghostModel, ghostPos[i] + glm::vec3(shakeX, 0.4f + shakeY, shakeZ));
            ghostModel = glm::scale(ghostModel, glm::vec3(0.7f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ghostModel));
            ghost.draw();

            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 64.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.color"), 1.0f, 1.0f, 1.0f);

            glm::mat4 eyeModel = ghostModel;
            eyeModel = glm::translate(eyeModel, glm::vec3(0.3f, 0.5f, 0.9f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.22f, 0.28f, 0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            eyeModel = ghostModel;
            eyeModel = glm::translate(eyeModel, glm::vec3(-0.3f, 0.5f, 0.9f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.22f, 0.28f, 0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glUniform3f(glGetUniformLocation(lightingShader.Program, "material.color"), 0.0f, 0.0f, 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

            eyeModel = ghostModel;
            eyeModel = glm::translate(eyeModel, glm::vec3(0.3f, 0.5f, 1.0f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.08f, 0.12f, 0.08f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);

            eyeModel = ghostModel;
            eyeModel = glm::translate(eyeModel, glm::vec3(-0.3f, 0.5f, 1.0f));
            eyeModel = glm::scale(eyeModel, glm::vec3(0.08f, 0.12f, 0.08f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

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

        glfwSwapBuffers(window);
    }

    pacman.cleanup();
    ghost.cleanup();
    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (isPacmanDying) return;

    float speed = moveSpeed * deltaTime;

    if (firstPersonMode) {
        glm::vec3 front;
        front.x = -cos(glm::radians(pacYaw));
        front.y = 0.0f;
        front.z = sin(glm::radians(pacYaw));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

        if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) { pacPos += front * speed; gameStarted = true; }
        if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) { pacPos -= front * speed; gameStarted = true; }
        if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) { pacPos -= right * speed; gameStarted = true; }
        if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) { pacPos += right * speed; gameStarted = true; }
    }
    else {
        if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
        if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);

        if (keys[GLFW_KEY_UP]) { pacPos.z -= speed; pacYaw = 270.0f; gameStarted = true; }
        if (keys[GLFW_KEY_DOWN]) { pacPos.z += speed; pacYaw = 90.0f; gameStarted = true; }
        if (keys[GLFW_KEY_LEFT]) { pacPos.x -= speed; pacYaw = 0.0f; gameStarted = true; }
        if (keys[GLFW_KEY_RIGHT]) { pacPos.x += speed; pacYaw = 180.0f; gameStarted = true; }

        if (keys[GLFW_KEY_UP] && keys[GLFW_KEY_RIGHT]) pacYaw = 225.0f;
        if (keys[GLFW_KEY_UP] && keys[GLFW_KEY_LEFT]) pacYaw = 315.0f;
        if (keys[GLFW_KEY_DOWN] && keys[GLFW_KEY_RIGHT]) pacYaw = 135.0f;
        if (keys[GLFW_KEY_DOWN] && keys[GLFW_KEY_LEFT]) pacYaw = 45.0f;
    }

    if (keys[GLFW_KEY_R]) {
        pacPos = glm::vec3(-0.7f, 0.5f, 6.3f);
        pacYaw = 0.0f;
        pacPitch = 0.0f;
    }

    // =====================================================
// COLISION CON PELLETS
// =====================================================
    for (size_t i = 0; i < pellets.size(); i++) {

        if (pelletEaten[i]) continue;

        float dist = glm::length(pacPos - pellets[i]);

        if (dist < 0.5f) {

            pelletEaten[i] = true;

            float offset = (GRID_SIZE * CELL_SIZE) / 2.0f;

            int gi = round((pellets[i].z + offset) / CELL_SIZE);
            int gj = round((pellets[i].x + offset) / CELL_SIZE);

            // POWER PELLETS
            if ((gi == 1 && gj == 1) ||
                (gi == 15 && gj == 15) ||
                (gi == 3 && gj == 15) ||
                (gi == 11 && gj == 1) ||
                (gi == 7 && gj == 10)) {

                frightenedMode = true;
                frightenedTimer = frightenedDuration;
            }
        }
    }
}

void AnimateMouth()
{
    if (!animMouth || isPacmanDying) {
        if (!animMouth) mouthAngle = 5.0f;
        return;
    }

    float speed = 80.0f * deltaTime;

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

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        if (action == GLFW_RELEASE) keys[key] = false;
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        animMouth = !animMouth;

    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        std::cout << "Integrantes: Carlos Ivan Hernandez Cruz, Julio Cesar Paz Orduñez, Ambar Atl Flores Garcia" << std::endl;
        glfwSetWindowTitle(window, "Pac-Man 3D | Integrantes listados en consola");
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        glfwSetWindowTitle(window, "Pac-Man 3D");
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS && !isPacmanDying) {

        firstPersonMode = !firstPersonMode;

        if (firstPersonMode)
            followCameraMode = false;

        if (firstPersonMode)
            std::cout << "Camara Primera Persona: ACTIVADA" << std::endl;
        else
            std::cout << "Camara Libre Espectador: ACTIVADA" << std::endl;
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {

        firstPersonMode = false;
        followCameraMode = false;

        std::cout << "Camara Libre ACTIVADA" << std::endl;
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS && !isPacmanDying) {

        followCameraMode = !followCameraMode;

        if (followCameraMode) {
            firstPersonMode = false;
            std::cout << "Camara Seguimiento: ACTIVADA" << std::endl;
        }
        else {
            std::cout << "Camara Seguimiento: DESACTIVADA" << std::endl;
        }
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    if (isPacmanDying) return;

    if (firstPersonMode) {
        float sensitivity = 0.1f;
        pacYaw += xOffset * sensitivity;
        pacPitch += yOffset * sensitivity;

        if (pacPitch > 89.0f) pacPitch = 89.0f;
        if (pacPitch < -89.0f) pacPitch = -89.0f;
    }
    else {
        camera.ProcessMouseMovement(xOffset, yOffset);
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}