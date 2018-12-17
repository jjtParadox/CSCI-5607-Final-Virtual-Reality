#pragma once
#include <string>
#include "glad.h"

typedef struct {
    GLint position;
    GLint normals;
    GLint texCoord;
    GLint view;
    GLint projection;
    GLint model;
    GLint color;
    GLint texID;
    GLint shaderMode;
} ShaderAttributes;

class ShaderManager {
   public:
    static int InitShaders();
    static void Cleanup();

    static GLuint Textured_Shader;
    static GLuint CompanionWindow_Shader;
    static GLuint RenderModel_Shader;
    static ShaderAttributes Attributes;

   private:
    static void InitShaderAttributes();
    static GLuint CompileShaderProgram(const std::string& vertex_shader_file, const std::string& fragment_shader_file);
    static char* ReadShaderSource(const char* shaderFile);
    static void VerifyShaderCompiled(GLuint shader);
};
