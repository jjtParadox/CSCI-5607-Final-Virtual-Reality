#pragma once
#include <SDL_stdinc.h>
#include <gtx/rotate_vector.hpp>
#include "glm.hpp"

const bool DEBUG_ON = true;
const int ELEMENTS_PER_VERT = 8;

const int ATTRIBUTE_STRIDE = ELEMENTS_PER_VERT;

const int VALUES_PER_POSITION = 3;
const int POSITION_OFFSET = 0;

const int VALUES_PER_NORMAL = 3;
const int NORMAL_OFFSET = 5;

const int VALUES_PER_TEXCOORD = 2;
const int TEXCOORD_OFFSET = 3;

const float CAMERA_ROTATION_SPEED = 0.01f;
const float CAMERA_MOVE_SPEED = 0.01f;
const float MAX_MOVE_SPEED = 2.0f * CAMERA_MOVE_SPEED;
const float VR_MOVE_SPEED_FACTOR = 2.0f;

const float ABSOLUTE_TOLERANCE = 0.00001f;

const float PLAYER_HALF_HEIGHT = 0.25f;
const float START_CAMERA_Z = 2 * PLAYER_HALF_HEIGHT;
const float GRAVITY = 0.0015f;
const float JUMP_VELOCITY = 0.04f;
const float GROUND_LEVEL = 0.0f;
const float JUMPING_LATERAL_MOVEMENT_FACTOR = 0.04f;
const float CROUCH_DISTANCE = 0.25f;
const float CROUCH_SPEED_FACTOR = 0.35f;

const float DOOR_SHRINK_FACTOR = 0.9f;
const float MIN_DOOR_SCALE = 0.005f;
const float DOOR_ROTATION_SPEED = 0.1f;
const float KEY_ROTATION_SPEED = 0.005f;
const float KEY_HEIGHT = 0.15f;
const int KEY_DROP_PICKUP_COOLDOWN_MS = 3500;

const glm::mat4 world_to_openvr_scale = glm::scale(glm::mat4(), glm::vec3(1.5, 1.5, 1.5));
const glm::mat4 openvr_to_world_rotation = glm::rotate(glm::mat4(), (float)(M_PI / 2.0f), glm::vec3(1, 0, 0));
const glm::mat4 openvr_to_world = openvr_to_world_rotation * glm::inverse(world_to_openvr_scale);
const glm::mat4 world_to_openvr = glm::inverse(openvr_to_world_rotation) * world_to_openvr_scale;

const glm::vec3 CONTROLLER_TIP_COLLIDER_HALF_DIAGONAL = glm::vec3(0.2f);

const int NUM_SHADER_MODES = 5;
