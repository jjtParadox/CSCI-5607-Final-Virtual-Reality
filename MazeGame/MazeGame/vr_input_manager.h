#pragma once
#include <vector>
#include "glm.hpp"
#include "render_model.h"
#include "transformable.h"

class VRCamera;

class VRInputManager {
   public:
    VRInputManager();
    explicit VRInputManager(vr::IVRSystem *vr_system, VRCamera *vr_camera);
    ~VRInputManager();

    void Init();  // Sets up action handles
    bool HandleInput();
    void RenderControllers(glm::mat4 worldViewMatrix) const;

    RenderModel *FindOrLoadRenderModel(const char *render_model_name);

    struct ControllerInfo_t {
        vr::VRInputValueHandle_t source = vr::k_ulInvalidInputValueHandle;
        vr::VRActionHandle_t action_pose = vr::k_ulInvalidActionHandle;
        vr::VRActionHandle_t action_haptic = vr::k_ulInvalidActionHandle;
        glm::mat4 raw_pose;
        std::shared_ptr<Transformable> transform = std::make_shared<Transformable>();
        RenderModel *render_model = nullptr;
        std::string render_model_name;
        bool show_controller;
    };

    enum Hand { Left = 0, Right = 1 };

   private:
    void ProcessVREvent(const vr::VREvent_t &event);

    ControllerInfo_t hands_[2];
    vr::VRActionSetHandle_t action_set_ = vr::k_ulInvalidActionSetHandle;
    vr::IVRSystem *vr_system_;
    std::vector<RenderModel *> render_models_;
};
