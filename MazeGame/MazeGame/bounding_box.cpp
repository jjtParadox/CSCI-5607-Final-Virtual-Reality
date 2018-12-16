#include <algorithm>
#include <cmath>
#include <gtc/type_ptr.hpp>
#include "bounding_box.h"
#include "shader_manager.h"
#include "texture_manager.h"

using std::max;
using std::min;

BoundingBox::BoundingBox() {
    transform = std::make_shared<Transformable>();
}

BoundingBox::BoundingBox(std::vector<glm::vec4> points) : BoundingBox() {
    float min_x = INFINITY, min_y = INFINITY, min_z = INFINITY, max_x = -INFINITY, max_y = -INFINITY, max_z = -INFINITY;
    for (auto point : points) {
        min_x = min(min_x, point.x);
        min_y = min(min_y, point.y);
        min_z = min(min_z, point.z);
        max_x = max(max_x, point.x);
        max_y = max(max_y, point.y);
        max_z = max(max_z, point.z);
    }

    InitToBounds(glm::vec3(min_x, min_y, min_z), glm::vec3(max_x, max_y, max_z));
}

BoundingBox::BoundingBox(std::vector<glm::vec3> points) : BoundingBox() {  // Terrible copy of above but meh
    float min_x = INFINITY, min_y = INFINITY, min_z = INFINITY, max_x = -INFINITY, max_y = -INFINITY, max_z = -INFINITY;
    for (auto point : points) {
        min_x = min(min_x, point.x);
        min_y = min(min_y, point.y);
        min_z = min(min_z, point.z);
        max_x = max(max_x, point.x);
        max_y = max(max_y, point.y);
        max_z = max(max_z, point.z);
    }

    InitToBounds(glm::vec3(min_x, min_y, min_z), glm::vec3(max_x, max_y, max_z));
}

BoundingBox::~BoundingBox() = default;

void BoundingBox::ExpandToBound(const BoundingBox& other) {
    glm::vec3 new_min = glm::vec3(min(min_->X(), other.min_->X()), min(min_->Y(), other.min_->Y()), min(min_->Z(), other.min_->Z()));
    glm::vec3 new_max = glm::vec3(max(max_->X(), other.max_->X()), max(max_->Y(), other.max_->Y()), max(max_->Z(), other.max_->Z()));

    min_->ResetAndSetTranslation(new_min);
    max_->ResetAndSetTranslation(new_max);
}

void BoundingBox::ExpandToBound(const std::vector<BoundingBox> bounding_boxes) {
    for (BoundingBox bounding_box : bounding_boxes) {
        ExpandToBound(bounding_box);
    }
}

bool BoundingBox::ContainsOrIntersects(const BoundingBox& other) const {
    return Overlaps(other.min_->X(), other.max_->X(), min_->X(), max_->X()) &&
           Overlaps(other.min_->Y(), other.max_->Y(), min_->Y(), max_->Y()) &&
           Overlaps(other.min_->Z(), other.max_->Z(), min_->Z(), max_->Z());
}

void BoundingBox::Render() const {
    glm::vec3 color = glm::vec3(1, 0, 0);

    glUseProgram(ShaderManager::Textured_Shader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::mat4 model_matrix;
    float side_length = max_->X() - min_->X();
    glm::vec3 center = glm::vec3(max_->WorldPosition() + min_->WorldPosition());
    center *= 0.5f;
    model_matrix = glm::translate(model_matrix, center);
    model_matrix = glm::scale(model_matrix, glm::vec3(side_length));
    // printf("Center: %f, %f, %f\n", center.x, center.y, center.z);

    glUniformMatrix4fv(ShaderManager::Attributes.model, 1, GL_FALSE, glm::value_ptr(model_matrix));

    // glUniformMatrix4fv(ShaderManager::Attributes.model, 1, GL_FALSE,
    //                   glm::value_ptr(transform->WorldTransform()));  // pass model matrix to shader

    glUniform1i(ShaderManager::Attributes.texID, UNTEXTURED);
    glUniform3fv(ShaderManager::Attributes.color, 1, glm::value_ptr(color));  // Set the color

    glDrawArrays(GL_TRIANGLES, debug_render_model->vbo_vertex_start_index_, debug_render_model->NumVerts());

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(0);
}

glm::vec3 BoundingBox::Max() const {
    return glm::vec3(max_->X(), max_->Y(), max_->Z());
}

glm::vec3 BoundingBox::Min() const {
    return glm::vec3(min_->X(), min_->Y(), min_->Z());
}

bool BoundingBox::Overlaps(double otherMin, double otherMax, double min, double max) {
    return IsBetween(otherMin, min, max) || IsBetween(otherMax, min, max) || IsBetween(min, otherMin, otherMax) ||
           IsBetween(max, otherMin, otherMax);
}

bool BoundingBox::IsBetween(double val, double low, double high) {
    return val >= low && val <= high;
}

void BoundingBox::InitToBounds(glm::vec3 min, glm::vec3 max) {
    min_ = std::make_shared<Transformable>(min);
    max_ = std::make_shared<Transformable>(max);

    min_->SetParent(transform);
    max_->SetParent(transform);
}

Model* BoundingBox::debug_render_model;
