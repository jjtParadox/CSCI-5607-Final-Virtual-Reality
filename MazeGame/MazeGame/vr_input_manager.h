#pragma once
#include <vector>
#include "controller.h"
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
    void RenderControllers(const glm::mat4 &worldViewMatrix) const;

    RenderModel *FindOrLoadRenderModel(const char *render_model_name);

    enum Hand { Left = 0, Right = 1 };

   private:
    void ProcessVREvent(const vr::VREvent_t &event);

    Controller hands_[2];
    vr::VRActionSetHandle_t action_set_ = vr::k_ulInvalidActionSetHandle;
    vr::IVRSystem *vr_system_;
    std::vector<RenderModel *> render_models_;
};
