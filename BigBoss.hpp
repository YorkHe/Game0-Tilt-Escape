#pragma once

#include "Board.h"
#include "GL.hpp"
#include <glm/glm.hpp>

#include <vector>



struct BigBoss {
    BigBoss(float x, float y);
    ~BigBoss();

    bool is_box = false;
    const float BOUNCE_FACTOR = 0.8f;
    const float MAX_SPEED = 0.004f;

    glm::vec2 position = glm::vec2(0, 0);
    glm::vec2 velocity = glm::vec2(0, 0);

    int hit_wall(Board& board);
    void update(float elapsed, Board& board);
    glm::mat4 get_view_matrix(float angle_horizontal, float angle_vertical);
};