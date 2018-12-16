#pragma once
#include <vector>
#include "controller.h"
#include "glm.hpp"
#include "map.h"
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

    // Given an action it returns the action data, and sets the device source if relevant
    static bool GetDigitalActionDataEdge(vr::VRActionHandle_t action, vr::InputDigitalActionData_t &action_data,
                                         vr::VRInputValueHandle_t *pDevicePath = nullptr);

    enum Hand { Left = 0, Right = 1 };
    Map *map_;

   private:
    void ProcessVREvent(const vr::VREvent_t &event);

    Controller hands_[2];
    vr::VRActionSetHandle_t action_set_ = vr::k_ulInvalidActionSetHandle;
    vr::VRActionHandle_t action_grab = vr::k_ulInvalidActionHandle;
    vr::IVRSystem *vr_system_;
    std::vector<RenderModel *> render_models_;
};
