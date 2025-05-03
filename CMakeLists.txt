# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Project name
project(tekken-mesh-viewer VERSION 1.0)


# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL3_DIR "C:\\SDL\\SDL3")
find_package(SDL3 REQUIRED HINTS ${SDL3_DIR})

find_package(OpenGL REQUIRED)

add_subdirectory(extern/glew/build/cmake)
add_subdirectory(extern/glm/)

add_definitions(-D_AMD64_)
# Manually specify source files
set(SOURCE_FILES
    src/main.cpp

    src/Engine/camera.cpp
    src/Engine/model.cpp
    src/Engine/shader.cpp

    src/Engine/UAM/mesh.cpp
    src/Engine/UAM/material.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    SDL3::SDL3
    glew_s
    OpenGL::GL 
    glm::glm
) 

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/extern/glew/include
    ${CMAKE_SOURCE_DIR}/extern/glm
)

# Build the exe in the base project folder
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)
# Get DLL locations
get_target_property(SDL3_DLL_PATH SDL3::SDL3 IMPORTED_LOCATION)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "${CMAKE_SOURCE_DIR}/bin/shaders"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_DLL_PATH}"
        "${CMAKE_SOURCE_DIR}/bin"
    COMMENT "Copying shaders and SDL3 DLLs to output directory"
    
)