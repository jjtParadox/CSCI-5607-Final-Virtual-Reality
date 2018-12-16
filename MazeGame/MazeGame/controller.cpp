#include <gtc/type_ptr.hpp>
#include "constants.h"
#include "controller.h"
#include "shader_manager.h"
#include "vr_input_manager.h"
#include "vr_manager.h"

void Controller::Render(const glm::mat4& worldViewMatrix) const {
    if (!show_controller || !render_model) return;

    const glm::mat4& matDeviceToTracking = transform->WorldTransform();
    glm::mat4 matMVP = worldViewMatrix * matDeviceToTracking;
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::RenderModel_Shader, "matrix"), 1, GL_FALSE, glm::value_ptr(matMVP));

    render_model->Draw();
}

void Controller::UpdatePose() {
    show_controller = true;

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
        }
    }
}
