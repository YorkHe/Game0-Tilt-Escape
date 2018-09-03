#include "Enemy.hpp"
#include "utils.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Enemy::Enemy(float x, float y, Enemy::DIRECTION d) {
    this->position.x = x;
    this->position.y = y;

    switch (d) {
        case DIRECTION_UP:
            this->velocity = glm::vec2(0.0f, 1.0f);
            break;
        case DIRECTION_DOWN:
            this->velocity = glm::vec2(0.0f, -1.0f);
            break;
        case DIRECTION_LEFT:
            this->velocity = glm::vec2(-1.0f, 0.0f);
            break;
        case DIRECTION_RIGHT:
            this->velocity = glm::vec2(1.0f, 0.0f);
            break;
    }
}

Enemy::~Enemy() {
    
}

glm::mat4 Enemy::get_view_matrix(Board board) {
    return glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            this->position.x, this->position.y, get_height(board.angle_horizontal, board.angle_vertical, this->position), 1.0f
    );
}

glm::mat4 Enemy::get_cone_matrix(Board board) {


    glm::mat4 cone_matrix = glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            this->position.x, this->position.y, get_height(board.angle_horizontal, board.angle_vertical, this->position), 1.0f
    );

    if (this->velocity.x > 0) {
        return glm::rotate(cone_matrix, glm::radians(180.0f), glm::vec3(0, 0, 1));
    }

    if (this->velocity.x < 0) {
        return cone_matrix;
    }

    if (this->velocity.y > 0){
        return glm::rotate(cone_matrix, glm::radians(270.0f), glm::vec3(0, 0, 1));
    }

    if (this->velocity.y < 0){
        return glm::rotate(cone_matrix, glm::radians(90.0f), glm::vec3(0, 0, 1));
    }

    return cone_matrix;
}

bool Enemy::intercept_with(BigBoss &b) {
    return false;
}

void Enemy::update(float elapsed) {
    float amt = elapsed * 1.0f;

    if (this->velocity.x != 0) {
        this->position.x += this->velocity.x * amt;
        distance += this->velocity.x * amt;
        if (abs(distance) >= MAX_DISTANCE) {
            distance = 0;
            this->velocity.x = -this->velocity.x;
        }
    }

    if (this->velocity.y != 0) {
        this->position.y += this->velocity.y * amt;
        distance += this->velocity.y * amt;
        if (abs(distance) >= MAX_DISTANCE) {
            distance = 0;
            this->velocity.y = -this->velocity.y;
        }
    };
}