#include <memory>
#include "transformable.h"

Transformable::Transformable(bool inherits_rotation) : inherits_rotation_(inherits_rotation) {
    children_ = std::unordered_set<std::shared_ptr<Transformable>>();
    parent_ = std::weak_ptr<Transformable>();
    local_transform_ = glm::mat4();
    world_transform_ = glm::mat4();
    Reset();
}

Transformable::Transformable(const glm::vec3& position, bool inherits_rotation) : Transformable(inherits_rotation) {
    Translate(position);
}

Transformable::~Transformable() = default;

void Transformable::Reset() {
    ClearChildren();
    ClearParent();
    world_transform_ = local_transform_ = glm::mat4();
}

void Transformable::ResetLocalTransform() {
    local_transform_ = glm::mat4();
    RecalculateWorldTransform();
}

void Transformable::Rotate(float radians, const glm::vec3& around) {
    local_transform_ = glm::rotate(local_transform_, radians, around);
    RecalculateWorldTransform();
}

void Transformable::Translate(float x, float y, float z) {
    Translate(glm::vec3(x, y, z));
}

void Transformable::Translate(const glm::vec3& translate_by) {
    local_transform_ = glm::translate(local_transform_, translate_by);
    RecalculateWorldTransform();
}

void Transformable::Scale(const glm::vec3& scale) {
    local_transform_ = glm::scale(local_transform_, scale);
    RecalculateWorldTransform();
}

void Transformable::Scale(float scale) {
    Scale(glm::vec3(scale, scale, scale));
}

void Transformable::ApplyMatrix(const glm::mat4 matrix) {
    local_transform_ = matrix * local_transform_;
    RecalculateWorldTransform();
}

void Transformable::ResetAndSetTranslation(const glm::vec3& translation) {
    ResetLocalTransform();
    Translate(translation);
}

void Transformable::Set(const glm::mat4 new_local_transform) {
    local_transform_ = new_local_transform;
    RecalculateWorldTransform();
}

void Transformable::SetInheritsRotation(bool inherits_rotation) {
    inherits_rotation_ = inherits_rotation;
}

void Transformable::AddChild(std::shared_ptr<Transformable> child) {
    children_.insert(child);
    if (!child->IsParent(shared_from_this())) {
        child->SetParent(shared_from_this());
    }
}

void Transformable::RemoveChild(std::shared_ptr<Transformable> child) {
    children_.erase(child);
    if (child->IsParent(shared_from_this())) {
        child->ClearParent();
    }
}

void Transformable::ClearChildren() {
    for (auto child : children_) {
        RemoveChild(child);
    }
}

void Transformable::SetParent(std::shared_ptr<Transformable> parent) {
    if (parent == nullptr) {
        printf("Warning: SetParent was called with a null parent. Should call ClearParent() instead\n");
    }

    parent_ = parent;
    if (!parent->HasChild(shared_from_this())) {
        parent->AddChild(shared_from_this());
    }
    RecalculateWorldTransform();
}

void Transformable::ClearParent() {
    auto former_parent = parent_.lock();
    parent_.reset();
    if (former_parent && former_parent->HasChild(shared_from_this())) {
        former_parent->RemoveChild(shared_from_this());
    }
    RecalculateWorldTransform();
}

bool Transformable::HasChild(std::shared_ptr<Transformable> child) const {
    return children_.find(child) != children_.end();
}

bool Transformable::IsParent(std::shared_ptr<Transformable> parent) const {
    return parent_.lock() == parent;
}

void Transformable::RecalculateWorldTransform() {
    if (parent_.expired()) {
        world_transform_ = local_transform_;
    } else {
        if (inherits_rotation_) {
            world_transform_ = parent_.lock()->world_transform_ * local_transform_;
        } else {
            auto parent_transform = glm::translate(glm::mat4(), parent_.lock()->WorldPosition());
            parent_transform = glm::scale(parent_transform, parent_.lock()->GetScale());
            world_transform_ = parent_transform * local_transform_;
        }
    }
    NotifyChildrenOfUpdate();
}

void Transformable::NotifyChildrenOfUpdate() {
    if (children_.size() == 0) return;

    for (auto child : children_) {
        NotifyChildOfUpdate(child);
    }
}

void Transformable::NotifyChildOfUpdate(std::shared_ptr<Transformable> child) {
    if (child == nullptr) {
        printf("A transform had a null child. Exiting...\n");
        exit(1);
    }

    child->RecalculateWorldTransform();
}

glm::mat4 Transformable::LocalTransform() const {
    return local_transform_;
}

glm::mat4 Transformable::WorldTransform() const {
    return world_transform_;
}

float Transformable::X() const {
    return world_transform_[3][0];  // Only translations (stored in the last column of the matrix) will come through if the
                                    // Transformable is interpreted as a position. GLM stores such that the first index is the first column
}

float Transformable::Y() const {
    return world_transform_[3][1];
}

float Transformable::Z() const {
    return world_transform_[3][2];
}

glm::vec3 Transformable::WorldPosition() const {
    return glm::vec3(world_transform_[3]);
}

glm::vec3 Transformable::LocalPosition() const {
    return glm::vec3(local_transform_[3]);
}

glm::vec3 Transformable::GetScale() const {
    glm::vec3 scale;
    scale.x = glm::length(glm::vec3(world_transform_[0]));
    scale.y = glm::length(glm::vec3(world_transform_[1]));
    scale.z = glm::length(glm::vec3(world_transform_[2]));

    return scale;
}
