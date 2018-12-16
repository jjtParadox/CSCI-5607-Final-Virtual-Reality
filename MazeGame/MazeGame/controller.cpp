#include <gtc/type_ptr.hpp>
#include "constants.h"
#include "controller.h"
#include "shader_manager.h"
#include "vr_input_manager.h"
#include "vr_manager.h"

Controller::Controller() {
    glm::vec3 tip_pos = transform->WorldPosition();
    glm::vec3 lower = tip_pos - CONTROLLER_TIP_COLLIDER_HALF_DIAGONAL;
    glm::vec3 upper = tip_pos + CONTROLLER_TIP_COLLIDER_HALF_DIAGONAL;
    std::vector<glm::vec3> points = {lower, upper};

    bounding_box_ = std::make_shared<BoundingBox>(points);
    bounding_box_->transform->SetParent(transform);
}

void Controller::Render(const glm::mat4& worldViewMatrix) const {
    if (!show_controller || !render_model) {
        return;
    }

    glUseProgram(ShaderManager::RenderModel_Shader);
    const glm::mat4& matDeviceToTracking = transform->WorldTransform();
    glm::mat4 matMVP = worldViewMatrix * matDeviceToTracking;
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::RenderModel_Shader, "matrix"), 1, GL_FALSE, glm::value_ptr(matMVP));

    render_model->Draw();
    bounding_box_->Render();
}

void Controller::HandleInput() {
    // Check for grab action
    vr::InputDigitalActionData_t action_data;
    if (VRInputManager::GetDigitalActionDataEdge(action_grab, action_data)) {
        if (action_data.bState) {  // Just grabbed
            printf("Grabbed!\n");
            Grab();
        } else {  // Just ungrabbed
            printf("Ungrabbed!\n");
            Ungrab();
        }
    }

    // Update pose
    vr::InputPoseActionData_t poseData;
    if (vr::VRInput()->GetPoseActionData(action_pose, vr::TrackingUniverseStanding, 0, &poseData, sizeof(poseData),
                                         vr::k_ulInvalidInputValueHandle) != vr::VRInputError_None ||
        !poseData.bActive || !poseData.pose.bPoseIsValid) {
        show_controller = false;
    } else {
        raw_pose = VRManager::ConvertSteamVRMatrixToMat4(poseData.pose.mDeviceToAbsoluteTracking);
        transform->Set(openvr_to_world * raw_pose);

        vr::InputOriginInfo_t originInfo;
        if (vr::VRInput()->GetOriginTrackedDeviceInfo(poseData.activeOrigin, &originInfo, sizeof(originInfo)) == vr::VRInputError_None &&
            originInfo.trackedDeviceIndex != vr::k_unTrackedDeviceIndexInvalid) {
            std::string sRenderModelName =
                VRManager::GetTrackedDeviceString(originInfo.trackedDeviceIndex, vr::Prop_RenderModelName_String);
            if (sRenderModelName != render_model_name) {
                render_model = input_manager->FindOrLoadRenderModel(sRenderModelName.c_str());
                render_model_name = sRenderModelName;
            }

            if (held_key_ == nullptr) {
                show_controller = true;
            } else {
                show_controller = false;
            }
        }
    }
}

void Controller::Grab() {
    Key* key = input_manager->map_->FirstIntersectedKey(*bounding_box_);
    if (key != nullptr && held_key_ == nullptr) {
        held_key_ = key;
        held_key_->SetHolder(this);

        held_key_->transform->ResetAndSetTranslation(glm::vec3(0));
        held_key_->transform->Rotate(M_PI / 2, glm::vec3(1, 0, 0));
        held_key_->transform->Rotate(M_PI / 2, glm::vec3(0, 1, 0));
        held_key_->transform->Translate(glm::vec3(-0.01, -0.15, 0));
        held_key_->transform->SetParent(transform);
    }
}

void Controller::Ungrab() {
    if (held_key_ == nullptr) return;

    held_key_->Drop();
    held_key_ = nullptr;
}

void Controller::UseKey() {
    held_key_ = nullptr;
}
