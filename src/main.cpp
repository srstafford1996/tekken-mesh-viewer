#include <iostream>

#include <GL/glew.h>
#include <SDL3/SDL.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Engine/camera.hpp"
#include "Engine/model.hpp"
#include "Engine/shader.hpp"

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

int main()
{

    /******************** START WINDOW INITIALIZATION  ********************/
    if ( SDL_Init( SDL_INIT_VIDEO) < 0 )
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

    GLint maxTextureUints;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUints);
    std::cout << "OpenGL Max Textures: " << maxTextureUints << std::endl;

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(glewError) << std::endl;
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    /******************** END WINDOW INITIALIZATION  ********************/

    // View and projection matrices
    Camera camera = Camera(-90.0f, 0.0f, glm::vec3(0.0f, 50.0f, 150.0f));    
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 1000.0f);

    // Initialize shader program
    ShaderProgram meshShader = ShaderProgram("shaders/mesh.vert", "shaders/mesh.frag");
    meshShader.use();
    meshShader.setMat4("projectionMatrix", projectionMatrix);

    // Time management
    Uint64 currFrame = 0;
    Uint64 lastFrame = 0;
    float deltaTime = 0;

    // Window context
    bool isRunning = true;
    SDL_Event e;

    // Test Model
    Model hwoModel;
    hwoModel.AddMesh(std::string("assets/Game/Character/Item/Meshes/hwo/Face/hwo_fac/Meshes/SK_CH_hwo_fac.psk"));
    hwoModel.AddMesh(std::string("assets/Game/Character/Item/Meshes/hwo/Hair/hwo_har_1p/Meshes/SK_CH_hwo_har_1p.psk"));
    hwoModel.AddMesh(std::string("assets/Game/Character/Item/Meshes/hwo/Lower/hwo_bdl_taekwondo/Meshes/SK_CH_hwo_bdl_taekwondo.psk"));
    hwoModel.AddMesh(std::string("assets/Game/Character/Item/Meshes/hwo/Upper/hwo_bdu_1p/Meshes/SK_CH_hwo_bdu_1p.psk"));

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // Main loop start

    std::cout << "Starting loop\n";
    while (isRunning)
    {
        currFrame = SDL_GetTicks();
        if (lastFrame)
        {
            deltaTime = (currFrame - lastFrame) / 1000.0f;
        }
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
        glm::mat4 viewMatrix = camera.getView();
        meshShader.setMat4("viewMatrix", viewMatrix);
        hwoModel.Draw(meshShader);
        SDL_GL_SwapWindow(window);
    }
}