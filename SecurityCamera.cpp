//
// Created by 何宇 on 2018/9/3.
//

#include "SecurityCamera.h"
#include "utils.h"

#include <glm/gtc/quaternion.hpp>
#include <iostream>

SecurityCamera::SecurityCamera(float x, float y, DIRECTION d) {
    this->position.x = x;
    this->position.y = y;
    switch (d) {
        case DIRECTION_RIGHT:
            camera_rotate = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case DIRECTION_UP:
            camera_rotate = glm::angleAxis(glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case DIRECTION_DOWN:
            camera_rotate = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        default:
            break;
    }
    this->direction = d;
}

SecurityCamera::~SecurityCamera() {
}

void SecurityCamera::update(float elapsed) {
    float amt = elapsed * 1.0f;

    if (angle + amt > MAX_ANGLE) {
        rotate_axis.z = - rotate_axis.z;
        angle = 0.0f;
    } else {
        angle += amt;
    }

    camera_rotate *= glm::angleAxis(amt, rotate_axis);
}

glm::mat4 SecurityCamera::get_cone_matrix(Board board) {
    return glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            this->position.x, this->position.y, 1.0f + get_height(board.angle_horizontal, board.angle_vertical, position), 1.0f
    ) * glm::mat4_cast(camera_rotate);
}

bool SecurityCamera::intercept_with(BigBoss &b) {

    float distance = glm::distance(this->position, b.position);

    glm::vec4 orientation = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * camera_rotate;
    glm::vec2 orientation_2d;
    orientation_2d.x = -orientation.x;
    orientation_2d.y = orientation.y;

    float angle = glm::acos(glm::dot(
            glm::normalize(orientation_2d),
            glm::normalize(b.position - this->position)
    ));

    // MAGIC NUMBER. HOORAY!
    spot = (distance < 5 && (3.14 - angle) > 2.7);
    return spot;
}

