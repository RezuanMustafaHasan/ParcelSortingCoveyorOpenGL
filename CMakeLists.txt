cmake_minimum_required(VERSION 3.20)
project(AutomatedConveyorFactory VERSION 1.0 LANGUAGES C CXX)

# CMake 4 removed pre-3.5 policy compatibility; GLM 0.9.9.8 still declares
# an older minimum even though it configures correctly with modern CMake.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# GLAD generates its C loader with this same Python interpreter.
find_package(Python 3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND "${Python_EXECUTABLE}" -c "import jinja2"
    RESULT_VARIABLE JINJA2_IMPORT_RESULT OUTPUT_QUIET ERROR_QUIET)
if(NOT JINJA2_IMPORT_RESULT EQUAL 0)
    message(FATAL_ERROR
        "GLAD requires Jinja2. Install it with: ${Python_EXECUTABLE} -m pip install Jinja2==3.1.6"
    )
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.3.9.tar.gz
    URL_HASH SHA256=a7e7faef424fcb5f83d8faecf9d697a338da7f7a906fc1afbc0e1879ef31bd53
)

FetchContent_Declare(glm
    URL https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.tar.gz
    URL_HASH SHA256=7d508ab72cb5d43227a3711420f06ff99b0a0cb63ee2f93631b162bfe1fe9592
)

FetchContent_Declare(glad
    URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.6.tar.gz
    URL_HASH SHA256=61a70234dc7da467cceb07fcdd6dec1213d6143a1b6b19ccc5d7b64cc247ea47
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
    src/FactorySimulation.cpp
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

include(CTest)
if(BUILD_TESTING)
    add_executable(factory_simulation_tests tests/FactorySimulationTests.cpp src/FactorySimulation.cpp)
    target_include_directories(factory_simulation_tests PRIVATE include)
    target_link_libraries(factory_simulation_tests PRIVATE glm::glm)
    add_test(NAME factory_simulation COMMAND factory_simulation_tests)
endif()
