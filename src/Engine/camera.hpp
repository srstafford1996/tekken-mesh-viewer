#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <SDL3/SDL.h>

class Camera
{
private:
    float yaw;
    float pitch;

    glm::vec3 front;
    void updateDirection();
public:
    glm::vec3 direction;
    glm::vec3 position;

    Camera(float initYaw, float initPitch, glm::vec3 initPos);

    glm::mat4 getView();

    void processMouseInput();
    void processKeyboardInput(SDL_Scancode scancode, float deltaTime);
};