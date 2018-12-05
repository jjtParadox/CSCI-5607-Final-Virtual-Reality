// MazeGame, Jackson Kruger, 2018
// Credit to Stephen J. Guy, 2018 for the foundations

#include <OpenVR/openvr.h>
#include "bounding_box.h"
#include "camera.h"
#include "map.h"
#include "map_loader.h"
#include "player.h"
#include "shader_manager.h"
#include "texture_manager.h"
const char* INSTRUCTIONS =
    "***************\n"
    "This is a game made by Jackson Kruger for CSCI 5607 at the University of Minnesota.\n"
    "\n"
    "Controls:\n"
    "WASD - Player movement\n"
    "Space - Player jump\n"
    "Left ctrl - Player crouch\n"
    "g - Drop key\n"
    "Esc - Quit\n"
    "F11 - Fullscreen\n"
    "***************\n";

const char* USAGE =
    "Usage:\n"
    "-w \'width\'x\'height\'\n"
    "   Example: -m 800x600\n"
    "-m map\n"
    "   This map must be in the root of the directory the game's being run from.\n"
    "   Example: -m map1.txt\n";

#include "glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <cstdio>
#include <string>

#include "map.h"
#include "model_manager.h"

using namespace std;

unsigned int screenWidth = 1536;
unsigned int screenHeight = 864;
float timePassed = 0;

bool fullscreen = false;

// srand(time(NULL));
float rand01() {
    return rand() / (float)RAND_MAX;
}

void drawGeometry(int shaderProgram, int model1_start, int model1_numVerts, int model2_start, int model2_numVerts);

vr::IVRSystem* m_pHMD;

struct FramebufferDesc {
    GLuint m_nDepthBufferId;
    GLuint m_nRenderTextureId;
    GLuint m_nRenderFramebufferId;
    GLuint m_nResolveTextureId;
    GLuint m_nResolveFramebufferId;
};
FramebufferDesc leftEyeDesc;
FramebufferDesc rightEyeDesc;

glm::mat4 m_mat4ProjectionLeft, m_mat4ProjectionRight;
glm::mat4 m_mat4eyePosLeft, m_mat4eyePosRight;

bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc& framebufferDesc) {
    glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

    glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

    glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

    glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

    glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
    glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

bool SetupStereoRenderTargets() {
    if (!m_pHMD) return false;

    m_pHMD->GetRecommendedRenderTargetSize(&screenWidth, &screenHeight);

    CreateFrameBuffer(screenWidth, screenHeight, leftEyeDesc);
    CreateFrameBuffer(screenWidth, screenHeight, rightEyeDesc);

    return true;
}

glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye) {
    if (!m_pHMD) return glm::mat4();

    vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, 0.1f, 30.0f);

    return glm::mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0], mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], mat.m[0][2],
                     mat.m[1][2], mat.m[2][2], mat.m[3][2], mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
}

glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye) {
    if (!m_pHMD) return glm::mat4();

    vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
    glm::mat4 matrixObj(matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0, matEyeRight.m[0][1], matEyeRight.m[1][1],
                        matEyeRight.m[2][1], 0.0, matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0, matEyeRight.m[0][3],
                        matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f);

    return glm::inverse(matrixObj);
}

glm::mat4 m_mat4HMDPose;

void UpdateHMDMatrixPose() {
    if (!m_pHMD) return;

    vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];

    vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

    /*m_iValidPoseCount = 0;
    m_strPoseClasses = "";
    for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice) {
        if (m_rTrackedDevicePose[nDevice].bPoseIsValid) {
            m_iValidPoseCount++;
            m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
            if (m_rDevClassChar[nDevice] == 0) {
                switch (m_pHMD->GetTrackedDeviceClass(nDevice)) {
                    case vr::TrackedDeviceClass_Controller:
                        m_rDevClassChar[nDevice] = 'C';
                        break;
                    case vr::TrackedDeviceClass_HMD:
                        m_rDevClassChar[nDevice] = 'H';
                        break;
                    case vr::TrackedDeviceClass_Invalid:
                        m_rDevClassChar[nDevice] = 'I';
                        break;
                    case vr::TrackedDeviceClass_GenericTracker:
                        m_rDevClassChar[nDevice] = 'G';
                        break;
                    case vr::TrackedDeviceClass_TrackingReference:
                        m_rDevClassChar[nDevice] = 'T';
                        break;
                    default:
                        m_rDevClassChar[nDevice] = '?';
                        break;
                }
            }
            m_strPoseClasses += m_rDevClassChar[nDevice];
        }
    }*/

    if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid) {
        vr::HmdMatrix34_t mat = m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
        m_mat4HMDPose = glm::inverse(glm::mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0, mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,
                                               mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0, mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f));

        // m_mat4HMDPose.invert();
    }
}

void SetupCameras() {
    m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
    m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
    m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
    m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
}

glm::mat4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye) {
    glm::mat4 matMVP;
    if (nEye == vr::Eye_Left) {
        matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
    } else if (nEye == vr::Eye_Right) {
        matMVP = m_mat4ProjectionRight * m_mat4eyePosRight * m_mat4HMDPose;
    }

    return matMVP;
}

vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
unsigned int m_nCompanionWindowWidth = 640;
unsigned int m_nCompanionWindowHeight = 320;
unsigned int m_uiCompanionWindowIndexSize;
GLuint m_unCompanionWindowVAO;
GLuint m_glCompanionWindowIDVertBuffer;
GLuint m_glCompanionWindowIDIndexBuffer;

struct VertexDataWindow {
    glm::vec2 position;
    glm::vec2 texCoord;

    VertexDataWindow(const glm::vec2& pos, const glm::vec2 tex) : position(pos), texCoord(tex) {}
};

void SetupCompanionWindow() {
    if (!m_pHMD) return;

    std::vector<VertexDataWindow> vVerts;

    // left eye verts
    vVerts.push_back(VertexDataWindow(glm::vec2(-1, -1), glm::vec2(0, 1)));
    vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(1, 1)));
    vVerts.push_back(VertexDataWindow(glm::vec2(-1, 1), glm::vec2(0, 0)));
    vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(1, 0)));

    // right eye verts
    vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(0, 1)));
    vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1, 1)));
    vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(0, 0)));
    vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1, 0)));

    GLushort vIndices[] = {0, 1, 3, 0, 3, 2, 4, 5, 7, 4, 7, 6};
    m_uiCompanionWindowIndexSize = _countof(vIndices);

    glGenVertexArrays(1, &m_unCompanionWindowVAO);
    glBindVertexArray(m_unCompanionWindowVAO);

    glGenBuffers(1, &m_glCompanionWindowIDVertBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer);
    glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

    glGenBuffers(1, &m_glCompanionWindowIDIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void*)offsetof(VertexDataWindow, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void*)offsetof(VertexDataWindow, texCoord));

    glBindVertexArray(0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RenderCompanionWindow() {
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight);

    glBindVertexArray(m_unCompanionWindowVAO);
    glUseProgram(ShaderManager::CompanionWindow_Shader);

    // render left eye (first half of index array )
    glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT, 0);

    // render right eye (second half of index array )
    glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT,
                   (const void*)(uintptr_t)(m_uiCompanionWindowIndexSize));

    glBindVertexArray(0);
    glUseProgram(0);
}

int main(int argc, char* argv[]) {
    printf("HMD Present? %i\n", vr::VR_IsHmdPresent());
    printf("Runtime installed? %i\n", vr::VR_IsRuntimeInstalled());
    if (vr::VR_IsHmdPresent() && vr::VR_IsRuntimeInstalled()) {
        printf("We're ready to go!\n");
    } else {
        exit(-1);
    }
    vr::EVRInitError perror;
    m_pHMD = VR_Init(&perror, vr::VRApplication_Scene);
    if (m_pHMD == NULL) {
        printf("Failed to init VR system.");
        exit(-1);
    }

    if (!vr::VRCompositor()) {
        printf("Compositor initialization failed.\n");
        exit(-1);
    }

    // Parse command-line arguments
    bool window_size_specified = false;
    std::string map_file = "map2.txt";
    int result;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'w':
                    result = sscanf_s(argv[++i], "%ix%i", &screenWidth, &screenHeight);
                    if (result == 2) {
                        window_size_specified = true;
                    } else {
                        printf("%s\n", USAGE);
                        exit(1);
                    }
                    break;
                case 'm':
                    map_file = argv[++i];
                    break;
                default:
                    printf("%s\n", USAGE);
                    exit(1);
            }
        }
    }

    SDL_Init(SDL_INIT_VIDEO);  // Initialize Graphics (for OpenGL)

    // Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window =
        SDL_CreateWindow("My OpenGL Program", 100, 100, m_nCompanionWindowWidth, m_nCompanionWindowHeight, SDL_WINDOW_OPENGL);

    // Maximize the window if no size was specified
    // if (!window_size_specified) {
    //    SDL_SetWindowResizable(window, SDL_TRUE);                // Allow resizing
    //    SDL_MaximizeWindow(window);                              // Maximize
    //    SDL_GetWindowSize(window, &screenWidth, &screenHeight);  // Get the new size
    //    SDL_SetWindowResizable(window, SDL_FALSE);               // Disable future resizing
    //}

    // Create a context to draw in
    SDL_GLContext context = SDL_GL_CreateContext(window);

    SDL_SetRelativeMouseMode(SDL_TRUE);  // 'grab' the mouse

    // Load OpenGL extentions with GLAD
    if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        printf("\nOpenGL loaded\n");
        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n\n", glGetString(GL_VERSION));
    } else {
        printf("ERROR: Failed to initialize OpenGL context.\n");
        return -1;
    }

    MapLoader map_loader;
    Map* map = map_loader.LoadMap(map_file);
    Camera camera = Camera();

    Player player(&camera, map);
    map->Add(&player);

    // Load the textures
    TextureManager::InitTextures();

    // Build a Vertex Array Object (VAO) to store mapping of shader attributes to VBO
    GLuint vao;
    glGenVertexArrays(1, &vao);  // Create a VAO
    glBindVertexArray(vao);      // Bind the above created VAO to the current context

    ModelManager::InitVBO();

    ShaderManager::InitShader("textured-Vertex.glsl", "textured-Fragment.glsl");

    TextureManager::InitTextures();

    glBindVertexArray(0);  // Unbind the VAO in case we want to create a new one

    glEnable(GL_DEPTH_TEST);

    printf("%s\n", INSTRUCTIONS);

    SetupCameras();
    SetupStereoRenderTargets();
    SetupCompanionWindow();

    // Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    bool quit = false;
    double lastTime = SDL_GetTicks();
    int nbFrames = 0;
    while (!quit) {
        while (SDL_PollEvent(&windowEvent)) {  // inspect all events in the queue
            if (windowEvent.type == SDL_QUIT) quit = true;
            // List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            // Scancode refers to a keyboard position, keycode refers to the letter (e.g., EU keyboards)
            if (windowEvent.type == SDL_KEYUP) {  // Exit event loop
                if (windowEvent.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (windowEvent.key.keysym.sym == SDLK_F11) {  // If F11 is pressed
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);  // Toggle fullscreen
                } else if (windowEvent.key.keysym.sym == SDLK_LCTRL) {
                    player.UnCrouch();
                } else if (windowEvent.key.keysym.sym == SDLK_g) {
                    player.DropKey();
                }
            }
            if (windowEvent.type == SDL_KEYDOWN) {
                if (windowEvent.key.keysym.sym == SDLK_SPACE) {
                    player.Jump();
                } else if (windowEvent.key.keysym.sym == SDLK_LCTRL) {
                    player.Crouch();
                }
            }

            if (windowEvent.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
                // printf("Mouse movement (xrel, yrel): (%i, %i)\n", windowEvent.motion.xrel, windowEvent.motion.yrel);
                float factor = 0.002f;
                camera.Rotate(0, -windowEvent.motion.xrel * factor);
            }

            switch (windowEvent.window.event) {
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    SDL_Log("Window focus lost");
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    SDL_Log("Window focus gained");
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    break;
            }
        }

        double currentTime = SDL_GetTicks();
        nbFrames++;
        if (currentTime - lastTime >= 1000) {  // If last prinf() was more than 1 sec ago
            // printf and reset timer
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1000;
        }

        // glClearColor(.2f, 0.4f, 0.8f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glUseProgram(ShaderManager::Textured_Shader);

        // timePassed = SDL_GetTicks() / 1000.f;

        // player.Update();
        // camera.Update();

        // glm::mat4 proj = glm::perspective(3.14f / 2, screenWidth / (float)screenHeight, 0.01f, 1000.0f);  // FOV, aspect, near, far
        // glUniformMatrix4fv(ShaderManager::Attributes.projection, 1, GL_FALSE, glm::value_ptr(proj));

        // TextureManager::Update();

        // glBindVertexArray(vao);

        // map->UpdateAll();

        // SDL_GL_SwapWindow(window);  // Double buffering

        vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

        // Clear the screen to default color
        /*glClearColor(.2f, 0.4f, 0.8f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_MULTISAMPLE);

        // Left Eye
        glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
        glViewport(0, 0, screenHeight, screenWidth);

        // Render scene
        glUseProgram(ShaderManager::Textured_Shader);
        timePassed = SDL_GetTicks() / 1000.f;
        player.Update();
        camera.Update();
        glm::mat4 proj = glm::perspective(3.14f / 2, screenWidth / (float)screenHeight, 0.01f, 1000.0f);  // FOV, aspect, near, far
        // glUniformMatrix4fv(ShaderManager::Attributes.projection, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(ShaderManager::Attributes.projection, 1, GL_FALSE, glm::value_ptr(GetCurrentViewProjectionMatrix(vr::Eye_Left)));
        TextureManager::Update();
        glBindVertexArray(vao);
        map->UpdateAll();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_MULTISAMPLE);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);

        glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glEnable(GL_MULTISAMPLE);

        // Right Eye
        glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
        glViewport(0, 0, screenWidth, screenHeight);

        // Render scene
        glUseProgram(ShaderManager::Textured_Shader);
        player.Update();
        camera.Update();
        glUniformMatrix4fv(ShaderManager::Attributes.projection, 1, GL_FALSE,
                           glm::value_ptr(GetCurrentViewProjectionMatrix(vr::Eye_Right)));
        // glUniformMatrix4fv(ShaderManager::Attributes.projection, 1, GL_FALSE, glm::value_ptr(proj));
        TextureManager::Update();
        glBindVertexArray(vao);
        map->UpdateAll();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_MULTISAMPLE);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);

        glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // Submit to OpenVR
        vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
        vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
        vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

        RenderCompanionWindow();

        glFinish();

        SDL_GL_SwapWindow(window);  // Double buffering

        UpdateHMDMatrixPose();
    }

    // Clean Up
    ShaderManager::Cleanup();
    ModelManager::Cleanup();
    glDeleteVertexArrays(1, &vao);

    SDL_GL_DeleteContext(context);
    SDL_Quit();

    vr::VR_Shutdown();

    return 0;
}
