#pragma once
#include <OpenVR/openvr.h>
#include "glm.hpp"
#include "render_model.h"
#include "transformable.h"

class VRInputManager;

class Controller {
   public:
    void Render(const glm::mat4& worldViewMatrix) const;
    void UpdatePose();

    vr::VRInputValueHandle_t source = vr::k_ulInvalidInputValueHandle;
    vr::VRActionHandle_t action_pose = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t action_haptic = vr::k_ulInvalidActionHandle;
    glm::mat4 raw_pose;
    std::shared_ptr<Transformable> transform = std::make_shared<Transformable>();
    RenderModel* render_model = nullptr;
    std::string render_model_name;
    bool show_controller;
    VRInputManager* input_manager;
};
