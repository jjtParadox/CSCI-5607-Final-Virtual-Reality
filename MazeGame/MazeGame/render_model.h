#pragma once
#include <OpenVR/openvr.h>
#include <string>
#include "glad.h"

class RenderModel {
   public:
    RenderModel(const std::string& sRenderModelName);
    ~RenderModel();

    // Allocate and populate OpenGL resources for a render model
    bool Init(const vr::RenderModel_t& vrModel, const vr::RenderModel_TextureMap_t& vrDiffuseTexture);

    void Cleanup();
    void Draw();
    const std::string& GetName() const {
        return model_name_;
    }

   private:
    GLuint gl_vert_buffer_;
    GLuint gl_index_buffer_;
    GLuint gl_vert_array_;
    GLuint gl_texture_;
    GLsizei vertex_count_;
    std::string model_name_;
};
