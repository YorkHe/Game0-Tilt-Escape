//
// Created by 何宇 on 2018/9/3.
//

#ifndef TILTESCAPE_SECURITYCAMERA_H
#define TILTESCAPE_SECURITYCAMERA_H

#include "BigBoss.hpp"
#include "Board.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct SecurityCamera {

    typedef enum {
        DIRECTION_UP,
        DIRECTION_DOWN,
        DIRECTION_LEFT,
        DIRECTION_RIGHT
    } DIRECTION;

    SecurityCamera(float x, float y, DIRECTION d);
    ~SecurityCamera();

    glm::vec2 position = glm::vec2(0.0f, 0.0f);
    DIRECTION direction;
    glm::vec3 rotate_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::quat camera_rotate = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    const float MAX_ANGLE = glm::radians(120.0f);
    float angle = glm::radians(60.0f);

    void update(float elapsed);
    bool intercept_with(BigBoss &b);

    bool spot = false;

    glm::mat4 get_cone_matrix(Board board);
};


#endif //TILTESCAPE_SECURITYCAMERA_H
