cmake_minimum_required(VERSION 3.5...4.0)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version for all subprojects")

project(SFMLOpenGLExample LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

############################
### Library dependencies ###
############################

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	1.0.3
)
FetchContent_MakeAvailable(glm)

add_library(common INTERFACE)
target_compile_features(common INTERFACE cxx_std_20)
target_compile_options(common INTERFACE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall -DGL_SILENCE_DEPRECATION>
  $<$<CXX_COMPILER_ID:MSVC>: /W4>
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Fix CoreGraphics linking on macOS
if(APPLE)
    find_library(CORE_GRAPHICS_LIB CoreGraphics REQUIRED)

    target_link_libraries(common INTERFACE
        ${CORE_GRAPHICS_LIB}
    )
endif()

###############################
### Sources and executables ###
###############################

# Tappa 01
SET(fn01 "Tappa01")
SET(fn02 "Tappa02")
SET(fn03 "Tappa03")
SET(fn04 "Tappa04")
SET(fn05 "Tappa05")
SET(fn06 "Tappa06")
SET(fn07 "Tappa07")
SET(fn08 "Tappa08")
SET(fn09 "Tappa09")
SET(fn10 "Tappa10")

add_executable(${fn01}
    Tappa01/tappa01.cpp
)
add_executable(${fn02}
    Tappa02/tappa02.cpp
)
add_executable(${fn03}
    Tappa03/tappa03.cpp
)
add_executable(${fn04}
    Tappa04/tappa04.cpp
)
add_executable(${fn05}
    Tappa05/tappa05.cpp
)
add_executable(${fn06}
    Tappa06/tappa06.cpp
)
add_executable(${fn07}
    Tappa07/tappa07.cpp
)
add_executable(${fn08}
    Tappa08/tappa08.cpp
)
add_executable(${fn09}
    Tappa09/tappa09.cpp
)
add_executable(${fn10}
    Tappa10/tappa10.cpp
)


target_link_libraries(${fn01}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)

target_link_libraries(${fn02}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)
target_link_libraries(${fn03}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)
target_link_libraries(${fn04}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)
target_link_libraries(${fn05}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)
target_link_libraries(${fn06}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)
target_link_libraries(${fn07}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)
target_link_libraries(${fn08}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)
target_link_libraries(${fn09}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)
target_link_libraries(${fn10}
    PRIVATE
    common
    SFML::Window
    OpenGL::GL
    glm::glm-header-only
    SFML::Graphics
)