#pragma once

#include "BigBoss.hpp"

struct Enemy {

    typedef enum {
        DIRECTION_UP,
        DIRECTION_DOWN,
        DIRECTION_LEFT,
        DIRECTION_RIGHT
    } DIRECTION;

    Enemy(float x, float y, DIRECTION d);
    ~Enemy();

    glm::vec2 position = glm::vec2(0.0f, 0.0f);

    glm::vec2 velocity;
    const float MAX_DISTANCE = 12.0f;
    float distance = 0.0f;

    void update(float elapsed);

    bool intercept_with(BigBoss &b);

    glm::mat4 get_view_matrix(Board board);

    glm::mat4 get_cone_matrix(Board board);
};