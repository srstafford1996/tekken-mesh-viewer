#include <iostream>

#include <glm.hpp>
#include <gtc/type_ptr.hpp> // glm::lookAt

#include "camera.hpp"

#define CAMERA_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define CAMERA_SPEED 200.0f

#define SENSITIVITY 0.1f

Camera::Camera(float initYaw, float initPitch, glm::vec3 initPos)
{
    yaw = initYaw;
    pitch = initPitch;
    position = initPos;
    
    updateDirection();
}

void Camera::updateDirection()
{
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);
}

glm::mat4 Camera::getView()
{
    return glm::lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::processKeyboardInput(SDL_Scancode scancode, float deltaTime)
{
    if (scancode == SDL_SCANCODE_W) position += (CAMERA_SPEED * deltaTime) * front;
    if (scancode == SDL_SCANCODE_S) position -= (CAMERA_SPEED * deltaTime) * front;
    if (scancode == SDL_SCANCODE_A) position -= glm::normalize(glm::cross(front, CAMERA_UP)) * (CAMERA_SPEED * deltaTime);
    if (scancode == SDL_SCANCODE_D) position += glm::normalize(glm::cross(front, CAMERA_UP)) * (CAMERA_SPEED * deltaTime);
}

void Camera::processMouseInput()
{
    float deltaX, deltaY;
    SDL_GetRelativeMouseState(&deltaX, &deltaY);

    deltaX *= SENSITIVITY;
    deltaY *= -SENSITIVITY;

    yaw += deltaX;
    pitch += deltaY;

    if (pitch > 89.0f)
        pitch = 89.0f;
    else if (pitch < -89.0f)
        pitch = -89.0f;

    updateDirection();
}