#pragma once

#include <memory>
#include "OpenVR/openvr.h"
#include "glm.hpp"
#include "transformable.h"

class VRCamera {
   public:
    VRCamera(float near_clip, float far_clip, vr::IVRSystem* vr_system);
    ~VRCamera();

    void Setup();
    glm::mat4 GetCurrentWorldToViewMatrix(vr::Hmd_Eye eye);
    void SetCurrentPose(glm::mat4 new_hmd_pose);

    void MakeChildOfHeadset(std::shared_ptr<Transformable> child);
    void SetPosition(glm::vec3 position);
    void Translate(float right, float absolute_up, float forward);
    glm::vec3 GetNormalizedLookPosition();

   private:
    glm::mat4 GetEyeProjection(vr::Hmd_Eye eye);
    glm::mat4 GetEyeOffset(vr::Hmd_Eye eye);

    glm::vec3 HorizontalForward();
    glm::vec3 HorizontalRight();
    glm::vec3 GetHorizontalDirection(glm::vec4 pose_direction);

    glm::mat4 projection_left, projection_right;
    glm::mat4 eye_offset_left, eye_offset_right;
    glm::mat4 current_pose_;

    // world_anchor -> hmd_offset_ -> player
    std::shared_ptr<Transformable> world_anchor_;  // Represents where, in game, the center of the playspace is
    std::shared_ptr<Transformable> hmd_offset_;    // Represents where the HMD currently is in the world

    float near_clip_, far_clip_;
    vr::IVRSystem* vr_system_;
};
