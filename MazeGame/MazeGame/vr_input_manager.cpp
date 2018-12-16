// Credit for the foundations of this code goes to Valve Corporation
// https://github.com/ValveSoftware/openvr/blob/master/samples/hellovr_opengl/hellovr_opengl_main.cpp
#include "vr_input_manager.h"

#include <filesystem>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include "constants.h"
#include "gtx/rotate_vector.hpp"
#include "shader_manager.h"
#include "vr_camera.h"
#include "vr_manager.h"

namespace fs = std::experimental::filesystem;

void ThreadSleep(unsigned long nMilliseconds) {
#if defined(_WIN32)
    ::Sleep(nMilliseconds);
#elif defined(POSIX)
    usleep(nMilliseconds * 1000);
#endif
}

VRInputManager::VRInputManager(vr::IVRSystem* vr_system, VRCamera* vr_camera) : VRInputManager() {
    vr_system_ = vr_system;
    vr_camera->MakeChildOfTrackingCenter(hands_[Left].transform);
    vr_camera->MakeChildOfTrackingCenter(hands_[Right].transform);
}

VRInputManager::VRInputManager() : hands_{}, action_set_(0), vr_system_(nullptr), map_(nullptr) {}

VRInputManager::~VRInputManager() {}

void VRInputManager::Init() {
    fs::path action_manifest_path = fs::current_path() / "mazegame_actions.json";
    std::cout << "Current path is " << fs::current_path() << '\n'
              << "Expecting to find action manifest at " << canonical(action_manifest_path) << '\n';

    std::string action_manifest_string(action_manifest_path.string());
    vr::EVRInputError error = vr::VRInput()->SetActionManifestPath(action_manifest_string.c_str());

    vr::VRInput()->GetActionHandle("/actions/game/in/grab", &action_grab);

    vr::VRInput()->GetActionHandle("/actions/game/out/Haptic_Left", &hands_[Left].action_haptic);
    vr::VRInput()->GetInputSourceHandle("/user/hand/left", &hands_[Left].source);
    vr::VRInput()->GetActionHandle("/actions/game/in/Hand_Left", &hands_[Left].action_pose);

    vr::VRInput()->GetActionHandle("/actions/game/out/Haptic_Right", &hands_[Right].action_haptic);
    vr::VRInput()->GetInputSourceHandle("/user/hand/right", &hands_[Right].source);
    vr::VRInput()->GetActionHandle("/actions/game/in/Hand_Right", &hands_[Right].action_pose);

    vr::VRInput()->GetActionSetHandle("/actions/game", &action_set_);

    for (Controller& controller : hands_) {
        controller.input_manager = this;
    }
}

bool VRInputManager::HandleInput() {
    if (vr_system_ == nullptr) {
        printf("Tried to handle input for uninitialized VR!");
        return false;
    }

    // Process SteamVR events
    vr::VREvent_t event;
    while (vr_system_->PollNextEvent(&event, sizeof(event))) {
        ProcessVREvent(event);
    }

    // Process SteamVR action state
    // UpdateActionState is called each frame to update the state of the actions themselves. The application
    // controls which action sets are active with the provided array of VRActiveActionSet_t structs.
    vr::VRActiveActionSet_t actionSet = {0};
    actionSet.ulActionSet = action_set_;
    vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);

    vr::InputDigitalActionData_t action_data;
    vr::VRInputValueHandle_t origin;
    if (GetDigitalActionDataEdge(action_grab, action_data, &origin)) {
        Controller* source_controller = nullptr;
        if (origin == hands_[Left].source) {
            source_controller = &hands_[Left];
        } else if (origin == hands_[Right].source) {
            source_controller = &hands_[Right];
        }

        if (source_controller != nullptr) {
            if (action_data.bState) {  // Just grabbed
                // printf("Grabbed!\n");
                source_controller->Grab();
            } else {  // Just ungrabbed
                // printf("Ungrabbed!\n");
                source_controller->Ungrab();
            }
        }
    }

    for (Hand eHand = Left; eHand <= Right; ((int&)eHand)++) {
        hands_[eHand].UpdatePose();
    }

    return false;
}

void VRInputManager::RenderControllers(const glm::mat4& worldViewMatrix) const {
    for (Hand eHand = Left; eHand <= Right; ((int&)eHand)++) {
        hands_[eHand].Render(worldViewMatrix);

        /*printf("Rendermodel at location %f, %f, %f\n", hands_[eHand].pose[3][0], hands_[eHand].pose[3][1],
               hands_[eHand].pose[3][2]);*/
    }
}

RenderModel* VRInputManager::FindOrLoadRenderModel(const char* render_model_name) {
    RenderModel* pRenderModel = NULL;
    for (RenderModel* i : render_models_) {
        if (!_stricmp(i->GetName().c_str(), render_model_name)) {
            pRenderModel = i;
            break;
        }
    }

    // load the model if we didn't find one
    if (!pRenderModel) {
        vr::RenderModel_t* pModel;
        vr::EVRRenderModelError error;
        while (1) {
            error = vr::VRRenderModels()->LoadRenderModel_Async(render_model_name, &pModel);
            if (error != vr::VRRenderModelError_Loading) break;

            ThreadSleep(1);
        }

        if (error != vr::VRRenderModelError_None) {
            printf("Unable to load render model %s - %s\n", render_model_name,
                   vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
            return NULL;  // move on to the next tracked device
        }

        vr::RenderModel_TextureMap_t* pTexture;
        while (1) {
            error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
            if (error != vr::VRRenderModelError_Loading) break;

            ThreadSleep(1);
        }

        if (error != vr::VRRenderModelError_None) {
            printf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, render_model_name);
            vr::VRRenderModels()->FreeRenderModel(pModel);
            return NULL;  // move on to the next tracked device
        }

        pRenderModel = new RenderModel(render_model_name);
        if (!pRenderModel->Init(*pModel, *pTexture)) {
            printf("Unable to create GL model from render model %s\n", render_model_name);
            delete pRenderModel;
            pRenderModel = NULL;
        } else {
            render_models_.push_back(pRenderModel);
        }
        vr::VRRenderModels()->FreeRenderModel(pModel);
        vr::VRRenderModels()->FreeTexture(pTexture);
    }

    return pRenderModel;
}

bool VRInputManager::GetDigitalActionDataEdge(vr::VRActionHandle_t action, vr::InputDigitalActionData_t& action_data,
                                              vr::VRInputValueHandle_t* pDevicePath) {
    vr::VRInput()->GetDigitalActionData(action, &action_data, sizeof(action_data), vr::k_ulInvalidInputValueHandle);
    if (pDevicePath) {
        *pDevicePath = vr::k_ulInvalidInputValueHandle;
        if (action_data.bActive) {
            vr::InputOriginInfo_t originInfo;
            if (vr::VRInputError_None ==
                vr::VRInput()->GetOriginTrackedDeviceInfo(action_data.activeOrigin, &originInfo, sizeof(originInfo))) {
                *pDevicePath = originInfo.devicePath;
            }
        }
    }

    return action_data.bActive && action_data.bChanged;
}

void VRInputManager::ProcessVREvent(const vr::VREvent_t& event) {
    switch (event.eventType) {
        case vr::VREvent_TrackedDeviceDeactivated: {
            printf("Device %u detached.\n", event.trackedDeviceIndex);
        } break;
        case vr::VREvent_TrackedDeviceUpdated: {
            printf("Device %u updated.\n", event.trackedDeviceIndex);
        } break;
    }
}
