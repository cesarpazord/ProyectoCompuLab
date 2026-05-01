#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;
    bool constrainPitch = true;

    Camera(glm::vec3 position = glm::vec3(0, 0, 3), glm::vec3 up = glm::vec3(0, 1, 0),
        float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0, 0, -1)), MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    float GetZoom() { return Zoom; }
    glm::vec3 GetPosition() { return Position; }
    glm::vec3 GetFront() { return Front; }

    void ProcessKeyboard(Camera_Movement dir, float dt) {
        float velocity = MovementSpeed * dt;
        if (dir == FORWARD)  Position += Front * velocity;
        if (dir == BACKWARD) Position -= Front * velocity;
        if (dir == LEFT)     Position -= Right * velocity;
        if (dir == RIGHT)    Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        Yaw += xoffset;
        Pitch += yoffset;
        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f)  Zoom = 1.0f;
        if (Zoom > 90.0f) Zoom = 90.0f;
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
