#pragma once

#include "../glm/vec3.hpp"

const float DAMPING = 0.02f; // how much to damp the cloth simulation each frame
const float COLLISION_OFFSET_MULTIPLIER = 1.15f;

const float FIXED_TIME_STEP = (1.0f / 60.0f);   
const float FIXED_TIME_STEP2 = (FIXED_TIME_STEP * FIXED_TIME_STEP);

const glm::vec3 DEFAULT_GRAVITY_FORCE = glm::vec3(0.0f, -9.8f, 0.0f);