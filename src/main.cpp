#include <GL/glew.h>
#include <SDL3/SDL.h>

#include <iostream>

#include "pskreader.hpp"
#include "model.hpp"
#include "camera.hpp"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

int main()
{
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0)
    {
        std::cout << "Error initializing SDL: " << SDL_GetError();
        return -1;
    }
    
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window *window = SDL_CreateWindow(
        "Tekken Model Viewer", WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);
    SDL_SetWindowRelativeMouseMode(window, true);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(glewError) << std::endl;
        return -1;
    }
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    ShaderProgram meshShader("shaders/mesh.vert", "shaders/mesh.frag");
    meshShader.use();

    Model charModel = Model();

    // Camera and projection stuff
    SK_Camera camera(-90.0f, 0.0f, glm::vec3(0.0f, 50.0f, 150.0f)); 


    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 1000.0f);


    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.3f));

    glm::vec3 center = (glm::vec3(-22.2836f, -0.997706f, -49.3001f) + 
                    glm::vec3(22.2837f, 113.718f, 18.0586f)) * 0.5f;
    //model = glm::translate(model, -center);

    meshShader.setMat4("modelMatrix", model);
    meshShader.setMat4("projectionMatrix", projection);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    Uint64 currFrame = 0;
    Uint64 lastFrame = 0;
    float deltaTime = 0;

    bool isRunning = true;
    SDL_Event e;

    while (isRunning)
    {
        currFrame = SDL_GetTicks();
        if (lastFrame) deltaTime = (currFrame - lastFrame) / 1000.0f;
        lastFrame = currFrame; 

        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_EVENT_QUIT:
                    isRunning = false;
                    break;
                
                case SDL_EVENT_KEY_DOWN:
                    camera.processKeyboardInput(e.key.scancode, deltaTime);
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    camera.processMouseInput();
                    break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        glm::mat4 view = camera.getView();

        
        charModel.Draw(meshShader, model, view, projection);
        SDL_GL_SwapWindow(window);
    }
    return 0;
}
