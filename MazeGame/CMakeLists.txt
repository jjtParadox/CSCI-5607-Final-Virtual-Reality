cmake_minimum_required(VERSION 3.12)
project(MazeGame)
include(ExternalProject)

set(PLATFORM 86)

ExternalProject_Add(openvr
        GIT_REPOSITORY
            https://github.com/ValveSoftware/openvr.git
        GIT_SHALLOW TRUE
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "")

ExternalProject_Get_Property(openvr SOURCE_DIR)
SET(OPENVR_LIBRARY ${SOURCE_DIR}/lib/win32/openvr_api.lib)
SET(OPENVR_INCLUDE_DIR ${SOURCE_DIR}/headers)

ExternalProject_add(glm
        URL https://github.com/g-truc/glm/releases/download/0.9.8.5/glm-0.9.8.5.7z
        GIT_SHALLOW TRUE
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "")

ExternalProject_Get_Property(glm SOURCE_DIR)
SET(GLM_INCLUDE_DIR ${SOURCE_DIR}/glm)

ExternalProject_add(SDL2
        URL https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "")

ExternalProject_Get_Property(SDL2 SOURCE_DIR)
SET(SDL2_INCLUDE_DIR ${SOURCE_DIR}/include)
SET(SDL2_LIBRARIES ${SOURCE_DIR}/lib/x${PLATFORM}/${CMAKE_SHARED_MODULE_PREFIX}SDL2.lib)
LIST(APPEND SDL2_LIBRARIES ${SOURCE_DIR}/lib/x${PLATFORM}/${CMAKE_SHARED_MODULE_PREFIX}SDL2main.lib)

find_package(OpenGL REQUIRED)

file(GLOB SOURCE_FILES
        MazeGame/*.cpp
        MazeGame/*.c
        MazeGame/*.h)

include_directories(. ${SDL2_INCLUDE_DIR} ${GLM_INCLUDE_DIR} MazeGame/thirdparty)

add_executable(MazeGame MazeGame ${SOURCE_FILES})
target_link_libraries(MazeGame
        ${OPENGL_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${OPENVR_LIBRARY}
        ${CMAKE_DL_LIBS})

add_dependencies(MazeGame openvr glm SDL2)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MazeGame)
set_target_properties(MazeGame PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MazeGame")
