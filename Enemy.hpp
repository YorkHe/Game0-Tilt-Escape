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

    float position_x;
    float position_y;

    void update(float elapsed);
    void draw();

    bool intercept_with(BigBoss &b);
};