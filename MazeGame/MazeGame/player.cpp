#define GLM_FORCE_RADIANS
#define NOMINMAX

#include <SDL.h>
#include <algorithm>
#include "constants.h"
#include "map.h"
#include "player.h"

Player::Player(VRCamera* camera, Map* map) : GameObject() {
    camera_ = camera;
    if (camera == nullptr) {
        printf("Player was given null camera. Exiting...\n");
        exit(1);
    }

    map_ = map;
    if (map == nullptr) {
        printf("Player was given null map. Exiting...\n");
        exit(1);
    }

    float num = 0.15f;
    box_ = std::vector<glm::vec4>(8);
    box_[0] = glm::vec4(-num, -num, -PLAYER_HALF_HEIGHT, 1);
    box_[1] = glm::vec4(num, -num, -PLAYER_HALF_HEIGHT, 1);
    box_[2] = glm::vec4(num, num, -PLAYER_HALF_HEIGHT, 1);
    box_[3] = glm::vec4(-num, num, -PLAYER_HALF_HEIGHT, 1);
    box_[4] = glm::vec4(-num, -num, PLAYER_HALF_HEIGHT, 1);
    box_[5] = glm::vec4(num, -num, PLAYER_HALF_HEIGHT, 1);
    box_[6] = glm::vec4(num, num, PLAYER_HALF_HEIGHT, 1);
    box_[7] = glm::vec4(-num, num, PLAYER_HALF_HEIGHT, 1);

    glm::vec3 start_position = map_->SpawnPosition();
    // start_position.z = START_CAMERA_Z;
    camera->SetPosition(start_position);

    // glm::vec3 look_position = map_->GoalPosition();
    // look_position.z = start_position.z;
    // camera->SetLookAt(look_position);

    camera->MakeChildOfHeadset(transform);
    RegenerateBoundingBox();

    held_key_ = nullptr;
}

void Player::Update() {
    float move_speed = CAMERA_MOVE_SPEED;

    //// Player movement ////
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    /*if (key_state[SDL_SCANCODE_RIGHT]) {
        camera_->Rotate(0, -CAMERA_ROTATION_SPEED);
    } else if (key_state[SDL_SCANCODE_LEFT]) {
        camera_->Rotate(0, CAMERA_ROTATION_SPEED);
    }*/

    if (key_state[SDL_SCANCODE_W]) {
        forward_velocity += move_speed;  // movement forward
    } else if (key_state[SDL_SCANCODE_S]) {
        forward_velocity -= move_speed;
    }
    if (key_state[SDL_SCANCODE_D]) {
        right_velocity += move_speed;
    } else if (key_state[SDL_SCANCODE_A]) {
        right_velocity -= move_speed;
    }

    // Clamp the forward and right velocities
    forward_velocity = std::max(std::min(forward_velocity, MAX_MOVE_SPEED), -MAX_MOVE_SPEED);
    right_velocity = std::max(std::min(right_velocity, MAX_MOVE_SPEED), -MAX_MOVE_SPEED);

    camera_->Translate(right_velocity, vertical_velocity, forward_velocity);

    RegenerateBoundingBox();

    if (map_->IntersectsAnySolidObjects(this)) {
        camera_->Translate(-right_velocity, -vertical_velocity, -forward_velocity);  // Undo the movement
        RegenerateBoundingBox();

        right_velocity = 0;
        forward_velocity = 0;
        stuck_in_object = true;
    } else {
        stuck_in_object = false;
    }

    // printf("Player bounds: min: %f, %f, %f, max:: %f, %f, %f\n", bounding_box_->Min().x, bounding_box_->Min().y, bounding_box_->Min().z,
    //       bounding_box_->Max().x, bounding_box_->Max().y, bounding_box_->Max().z);
    // printf("Player z: %f\n", transform->Z());

    forward_velocity = 0;
    right_velocity = 0;

    // printf("Player ");
    bounding_box_->Render();
}

void Player::RegenerateBoundingBox() {
    InitBoundingBox(box_);
    bounding_box_->transform->ClearParent();
}
