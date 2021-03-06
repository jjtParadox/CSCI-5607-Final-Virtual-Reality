//#include "OpenVR/openvr.h"
//#include "glad.h"
//
//#if defined(__APPLE__) || defined(__linux__)
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
//#else
//#include <SDL.h>
//#include <SDL_opengl.h>
//#endif
//#include <iostream>
//
// const GLchar *vertexSource =
//    "#version 150 core\n"
//    "in vec2 position;"
//    "in vec3 inColor;"
//    "out vec3 Color;"
//    "void main() {"
//    "   Color = inColor;"
//    "   gl_Position = vec4(position, 0.0, 1.0);"
//    "}";
//
// const GLchar *fragmentSource =
//    "#version 150 core\n"
//    "in vec3 Color;"
//    "out vec4 outColor;"
//    "void main() {"
//    "   outColor = vec4(Color, 1.0);"  //(Red, Green, Blue, Alpha)
//    "}";
//
// bool fullscreen = false;
// unsigned int screenWidth = 800;
// unsigned int screenHeight = 600;
// vr::IVRSystem *m_pHMD;
//
// struct FramebufferDesc {
//    GLuint m_nDepthBufferId;
//    GLuint m_nRenderTextureId;
//    GLuint m_nRenderFramebufferId;
//    GLuint m_nResolveTextureId;
//    GLuint m_nResolveFramebufferId;
//};
// FramebufferDesc leftEyeDesc;
// FramebufferDesc rightEyeDesc;
//
// bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc) {
//    glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
//    glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);
//
//    glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
//    glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
//    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
//
//    glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
//    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
//    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);
//
//    glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
//    glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);
//
//    glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
//    glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);
//
//    // check FBO status
//    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    if (status != GL_FRAMEBUFFER_COMPLETE) {
//        return false;
//    }
//
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//    return true;
//}
//
// bool SetupStereoRenderTargets() {
//    if (!m_pHMD) return false;
//
//    m_pHMD->GetRecommendedRenderTargetSize(&screenWidth, &screenHeight);
//
//    CreateFrameBuffer(screenWidth, screenHeight, leftEyeDesc);
//    CreateFrameBuffer(screenWidth, screenHeight, rightEyeDesc);
//
//    return true;
//}
//
// void loadShader(GLuint shaderID, const GLchar *shaderSource) {
//    glShaderSource(shaderID, 1, &shaderSource, NULL);
//    glCompileShader(shaderID);
//
//    // Let's double check the shader compiled
//    GLint status;
//    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);  // Check for errors
//    if (!status) {
//        char buffer[512];
//        glGetShaderInfoLog(shaderID, 512, NULL, buffer);
//        printf("Shader Compile Failed. Info:\n\n%s\n", buffer);
//    }
//}
//
// int main(int argc, char *argv[]) {
//    printf("HMD Present? %i\n", vr::VR_IsHmdPresent());
//    printf("Runtime installed? %i\n", vr::VR_IsRuntimeInstalled());
//    if (vr::VR_IsHmdPresent() && vr::VR_IsRuntimeInstalled()) {
//        printf("We're ready to go!\n");
//    } else {
//        exit(-1);
//    }
//    vr::EVRInitError perror;
//    m_pHMD = vr::VR_Init(&perror, vr::VRApplication_Scene);
//    if (m_pHMD == NULL) {
//        printf("Failed to init VR system.");
//        exit(-1);
//    }
//
//    if (!vr::VRCompositor()) {
//        printf("Compositor initialization failed.\n");
//        exit(-1);
//    }
//
//    std::cout << "Hello World!\n";
//    SDL_Init(SDL_INIT_VIDEO);
//
//    SDL_version comp;
//    SDL_version linked;
//    SDL_VERSION(&comp);
//    SDL_GetVersion(&linked);
//    printf("\nCompiled against SDL version %d.%d.%d\n", comp.major, comp.minor, comp.patch);
//    printf("Linked SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);
//
//    // Ask SDL to get a recent version of OpenGL (3.2 or greater)
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//
//    m_pHMD->GetRecommendedRenderTargetSize(&screenWidth, &screenHeight);
//    printf("Recommended VR size: %ux%u\n", screenWidth, screenHeight);
//
//    // Create a window (offsetx, offsety, width, height, flags)
//    SDL_Window *window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
//
//    if (!window) {
//        printf("Could not create window: %s\n", SDL_GetError());
//        return EXIT_FAILURE;
//    }
//
//    // Initialize OpenGL through GLAD
//    SDL_GLContext context = SDL_GL_CreateContext(window);  // Bind OpenGL to the window
//
//    if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
//        printf("OpenGL loaded\n");
//        printf("Vendor: %s\n", glGetString(GL_VENDOR));
//        printf("Renderer: %s\n", glGetString(GL_RENDERER));
//        printf("Version: %s\n", glGetString(GL_VERSION));
//    } else {
//        printf("ERROR: Failed to initialize OpenGL context.\n");
//        return -1;
//    }
//
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    loadShader(vertexShader, vertexSource);
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    loadShader(fragmentShader, fragmentSource);
//
//    // Join the vertex and fragment shaders together into one program
//    GLuint shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glBindFragDataLocation(shaderProgram, 0, "outColor");  // set output
//    glLinkProgram(shaderProgram);                          // run the linker
//
//    GLfloat vertices[] = {0.0f,  0.5f,  1.0f, 0.0f, 0.0f,   // Vertex 1: postion = (0,.5) color = Red
//                          0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,   // Vertex 2: postion = (.5,-.5) color = Green
//                          -0.5f, -0.5f, 0.0f, 0.0f, 1.0f};  // Vertex 3: postion = (-.5,-.5) color = Blue
//
//    GLuint vbo;
//    glGenBuffers(1, &vbo);               // Create 1 buffer called vbo
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);  //(Only one buffer can be bound at a time)
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//    GLuint vao;
//    glGenVertexArrays(1, &vao);  // Create a VAO (on the GPU)
//    glBindVertexArray(vao);      // Bind the above created VAO to the current context
//
//    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
//    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
//    //(above params: Attribute, vals/attrib., type, isNormalized, stride, offset)
//    glEnableVertexAttribArray(posAttrib);  // Mark the attribute's location as valid
//
//    GLint colorAttrib = glGetAttribLocation(shaderProgram, "inColor");
//    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
//    glEnableVertexAttribArray(colorAttrib);
//
//    glBindVertexArray(0);  // Unbind the VAO so we don't accidentally modify it
//
//    // Main loop
//    SDL_Event windowEvent;
//    bool quit = false;
//    while (!quit) {
//        while (SDL_PollEvent(&windowEvent)) {
//            if (windowEvent.type == SDL_QUIT) quit = true;                                                // Exit Game Loop
//            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE) quit = true;  // Exit Game Loop
//            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) {
//                fullscreen = !fullscreen;
//                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
//            }
//        }
//
//        // glClearColor(.2f, 0.4f, 0.9f, 1.0f);  // Clear the screen to blue (background color)
//        // glClear(GL_COLOR_BUFFER_BIT);
//
//        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//        glEnable(GL_MULTISAMPLE);
//
//        // Left Eye
//        glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
//        glViewport(0, 0, screenHeight, screenWidth);
//
//        // Render scene
//        glUseProgram(shaderProgram);  // Set the active shader program
//        glBindVertexArray(vao);       // Bind the VAO for the shader(s) we are using
//        glDrawArrays(GL_TRIANGLES, 0, 3);
//
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//        glDisable(GL_MULTISAMPLE);
//
//        // Copy pixels from the renderframebuffer to the resolveframebuffer
//        // I think this has something to do with MSAA?
//        // Though disabling it causes odd z-buffering-related things to happen
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
//        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);
//
//        glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
//
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//
//        glEnable(GL_MULTISAMPLE);
//
//        // Right Eye
//        glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
//        glViewport(0, 0, screenWidth, screenHeight);
//
//        // Render scene
//        glUseProgram(shaderProgram);  // Set the active shader program
//        glBindVertexArray(vao);       // Bind the VAO for the shader(s) we are using
//        glDrawArrays(GL_TRIANGLES, 0, 3);
//
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//        glDisable(GL_MULTISAMPLE);
//
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
//        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);
//
//        glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
//
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//
//        // Submit to OpenVR
//        vr::Texture_t leftEyeTexture = {(void *)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
//        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
//        vr::Texture_t rightEyeTexture = {(void *)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL,
//        vr::ColorSpace_Gamma}; vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
//
//        SDL_GL_SwapWindow(window);  // Double buffering
//    }
//
//    glDeleteProgram(shaderProgram);
//    glDeleteShader(fragmentShader);
//    glDeleteShader(vertexShader);
//    glDeleteBuffers(1, &vbo);
//    glDeleteVertexArrays(1, &vao);
//    SDL_GL_DeleteContext(context);
//    SDL_Quit();
//
//    vr::VR_Shutdown();
//
//    return 0;
//}
