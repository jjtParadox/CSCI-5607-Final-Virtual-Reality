#pragma once
#include <detail/type_vec4.hpp>
#include <string>
#include <vector>
#include "glad.h"

class Model {
   public:
    Model(const std::string& file, GLuint vao);

    void LoadTxt(const std::string& file);
    void LoadObj(const std::string& file);

    int NumElements() const;
    int NumVerts() const;

    std::vector<glm::vec3> Vertices() const;

    float* model_;
    int vbo_vertex_start_index_;
    GLuint model_vao_;

   private:
    int num_verts_;
};
