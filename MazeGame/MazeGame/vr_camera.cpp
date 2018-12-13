#define _USE_MATH_DEFINES
#include "vr_camera.h"

#include "gtx/rotate_vector.hpp"
#include "vr_manager.h"

using glm::mat4;
using glm::vec3;
using glm::vec4;

VRCamera::VRCamera(float near_clip, float far_clip, vr::IVRSystem* vr_system) : near_clip_(near_clip), far_clip_(far_clip) {
    if (vr_system == nullptr) {
        printf("VRCamera cannot initialize with a null vr_system. Exiting...");
        exit(-1);
    }

    vr_system_ = vr_system;

    world_anchor_ = std::make_shared<Transformable>();
    hmd_offset_ = std::make_shared<Transformable>();
    hmd_offset_->SetParent(world_anchor_);
}

VRCamera::~VRCamera() {}

void VRCamera::Setup() {
    projection_left = GetEyeProjection(vr::Eye_Left);
    projection_right = GetEyeProjection(vr::Eye_Right);
    eye_offset_left = GetEyeOffset(vr::Eye_Left);
    eye_offset_right = GetEyeOffset(vr::Eye_Right);
}

glm::mat4 VRCamera::GetCurrentWorldToViewMatrix(vr::Hmd_Eye eye) {
    mat4 matMVP;
    if (eye == vr::Eye_Left) {
        matMVP = projection_left * eye_offset_left;
    } else if (eye == vr::Eye_Right) {
        matMVP = projection_right * eye_offset_right;
    }

    matMVP = matMVP * glm::inverse(current_pose_);
    matMVP = glm::rotate(matMVP, -(float)(M_PI / 2.0f), vec3(1, 0, 0));  // Convert coordinate systems
    matMVP = glm::scale(matMVP, vec3(1.5, 1.5, 2.0));                    // Make the world larger

    matMVP = matMVP * glm::inverse(world_anchor_->WorldTransform());  // Kind of an addition for world to camera

    return matMVP;
}

void VRCamera::SetCurrentPose(mat4 new_hmd_pose) {
    // printf("HMD Pose: %f, %f, %f\n", new_hmd_pose[3][0], new_hmd_pose[3][1], new_hmd_pose[3][2]);
    vec3 pos = glm::rotate(vec3(new_hmd_pose[3]), (float)(M_PI / 2.0f), vec3(1, 0, 0));
    hmd_offset_->ResetAndSetTranslation(pos);
    pos = hmd_offset_->WorldPosition();
    // printf("HMD Offset: %f, %f, %f\n", pos.x, pos.y, pos.z);
    current_pose_ = new_hmd_pose;
}

void VRCamera::MakeChildOfHeadset(std::shared_ptr<Transformable> child) {
    hmd_offset_->AddChild(child);
}

void VRCamera::SetPosition(vec3 position) {
    world_anchor_->ResetAndSetTranslation(position);
}

void VRCamera::Translate(float right, float absolute_up, float forward) {
    vec3 translation = right * HorizontalRight() + absolute_up * vec3(0, 0, 1) + forward * HorizontalForward();
    world_anchor_->Translate(translation);
}

vec3 VRCamera::GetNormalizedLookPosition() {
    return HorizontalForward();
}

mat4 VRCamera::GetEyeProjection(vr::Hmd_Eye eye) {
    vr::HmdMatrix44_t mat = vr_system_->GetProjectionMatrix(eye, near_clip_, far_clip_);

    return mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0], mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], mat.m[0][2],
                mat.m[1][2], mat.m[2][2], mat.m[3][2], mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
}

mat4 VRCamera::GetEyeOffset(vr::Hmd_Eye eye) {
    vr::HmdMatrix34_t matEyeRight = vr_system_->GetEyeToHeadTransform(eye);

    return glm::inverse(VRManager::ConvertSteamVRMatrixToMat4(matEyeRight));
}

vec3 VRCamera::HorizontalForward() {
    return GetHorizontalDirection(vec4(0, 0, -1, 0));
}

vec3 VRCamera::HorizontalRight() {
    return GetHorizontalDirection(vec4(1, 0, 0, 0));
}

glm::vec3 VRCamera::GetHorizontalDirection(vec4 pose_direction) {
    vec4 openvr_dir = current_pose_ * pose_direction;
    vec4 ourcoords_dir = glm::rotate(openvr_dir, (float)(M_PI / 2.0f), vec3(1, 0, 0));
    vec3 real_dir = vec3(ourcoords_dir);
    real_dir.z = 0;  // Ensure horizontal
    return glm::normalize(real_dir);
}
