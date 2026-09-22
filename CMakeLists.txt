cmake_minimum_required(VERSION 3.20)
project(AutomatedConveyorFactory VERSION 1.0 LANGUAGES C CXX)

# CMake 4 removed pre-3.5 policy compatibility; GLM 0.9.9.8 still declares
# an older minimum even though it configures correctly with modern CMake.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.3.9.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

FetchContent_Declare(glm
    URL https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

FetchContent_Declare(glad
    URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.6.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    SOURCE_SUBDIR cmake
)

FetchContent_MakeAvailable(glfw glm glad)
glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

add_executable(conveyor_factory
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Geometry.cpp
    src/FactoryScene.cpp
)

target_include_directories(conveyor_factory PRIVATE include)
target_link_libraries(conveyor_factory PRIVATE glfw glad_gl_core_33 glm::glm)

if(MSVC)
    target_compile_options(conveyor_factory PRIVATE /W4 /permissive-)
else()
    target_compile_options(conveyor_factory PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET conveyor_factory POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:conveyor_factory>/shaders
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT conveyor_factory)
