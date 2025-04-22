#include <GL/glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <SDL3/SDL.h>

class SK_Camera
{
private:
    float yaw;
    float pitch;

    glm::vec3 front;
    void updateDirection();
public:
    glm::vec3 direction;
    glm::vec3 position;

    SK_Camera(float initYaw, float initPitch, glm::vec3 initPos);

    glm::mat4 getView();

    void processMouseInput();
    void processKeyboardInput(SDL_Scancode scancode, float deltaTime);
};