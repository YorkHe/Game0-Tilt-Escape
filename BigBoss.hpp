#pragma once

#include "GL.hpp"
#include <glm/glm.hpp>

#include <vector>


struct BigBoss {
    BigBoss(float x, float y);
    ~BigBoss();

    glm::vec2 position = glm::vec2(0, 0);

    glm::vec2 velocity = glm::vec2(0.1, 0);

    void update(float elapsed);
    glm::mat4 get_view_matrix();
};