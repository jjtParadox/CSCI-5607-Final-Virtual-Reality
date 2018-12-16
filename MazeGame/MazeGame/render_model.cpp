// Credit for the foundations of this code goes to Valve Corporation
// https://github.com/ValveSoftware/openvr/blob/master/samples/hellovr_opengl/hellovr_opengl_main.cpp
#include "render_model.h"
#include "shader_manager.h"

RenderModel::RenderModel(const std::string& render_model_name) : model_name_(render_model_name) {
    gl_index_buffer_ = 0;
    gl_vert_array_ = 0;
    gl_vert_buffer_ = 0;
    gl_texture_ = 0;
}

RenderModel::~RenderModel() {
    Cleanup();
}

bool RenderModel::Init(const vr::RenderModel_t& vrModel, const vr::RenderModel_TextureMap_t& vrDiffuseTexture) {
    // create and bind a VAO to hold state for this model
    glGenVertexArrays(1, &gl_vert_array_);
    glBindVertexArray(gl_vert_array_);

    // Populate a vertex buffer
    glGenBuffers(1, &gl_vert_buffer_);
    glBindBuffer(GL_ARRAY_BUFFER, gl_vert_buffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW);

    // Identify the components in the vertex buffer
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, vPosition));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, vNormal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t),
                          (void*)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

    // Create and populate the index buffer
    glGenBuffers(1, &gl_index_buffer_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW);

    glBindVertexArray(0);

    // create and populate the texture
    glGenTextures(1, &gl_texture_);
    glBindTexture(GL_TEXTURE_2D, gl_texture_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 vrDiffuseTexture.rubTextureMapData);

    // If this renders black ask McJohn what's wrong.
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

    glBindTexture(GL_TEXTURE_2D, 0);

    vertex_count_ = vrModel.unTriangleCount * 3;

    return true;
}

void RenderModel::Cleanup() {
    if (gl_vert_buffer_) {
        glDeleteBuffers(1, &gl_index_buffer_);
        glDeleteVertexArrays(1, &gl_vert_array_);
        glDeleteBuffers(1, &gl_vert_buffer_);
        gl_index_buffer_ = 0;
        gl_vert_array_ = 0;
        gl_vert_buffer_ = 0;
    }
}

void RenderModel::Draw() {
    glUseProgram(ShaderManager::RenderModel_Shader);
    glBindVertexArray(gl_vert_array_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_texture_);

    glDrawElements(GL_TRIANGLES, vertex_count_, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
