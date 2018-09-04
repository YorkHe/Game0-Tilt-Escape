#include "BigBoss.hpp"
#include "utils.h"


#include <iostream>

BigBoss::BigBoss(float x, float y) {
    this->position = glm::vec2(x, y);
}

BigBoss::~BigBoss() {

}

glm::mat4 BigBoss::get_view_matrix(float angle_horizontal, float angle_vertical) {
    return glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        this->position.x, this->position.y, get_height(angle_horizontal, angle_vertical, this->position), 1.0f
    );
}

void BigBoss::update(float elapsed, Board& board) {
    float amt = elapsed * 5.0f;

    int num_steps = 5;

    float step_amt = amt / num_steps;

//    std::cerr << "("<< this->velocity.x << "," << this->velocity.y << ")" << std::endl;


    for (int i = 0; i < num_steps; i++) {
        glm::vec2 old_position = glm::vec2(position.x, position.y);

        this->velocity += glm::vec2(9.8 * board.angle_horizontal* step_amt, 9.8 * -board.angle_vertical* step_amt);

        if (this->velocity.x < 0) {
            this->velocity.x = std::max(this->velocity.x, -MAX_SPEED);
        } else {
            this->velocity.x = std::min(this->velocity.x, MAX_SPEED);
        }

        if (this->velocity.y < 0) {
            this->velocity.y = std::max(this->velocity.y, -MAX_SPEED);
        } else {
            this->velocity.y = std::min(this->velocity.y, MAX_SPEED);
        }

        this->position = this->position + velocity * step_amt;

        if (hit_wall(board)) {
            this->position = old_position;
        }
    }

//    glm::vec2 old_position = glm::vec2(this->position.x, this->position.y);

//    this->position = this->position + this->velocity;


}
int BigBoss::hit_wall(Board& board) {
    int hit = 0;

    bool hit_left = abs(position.x + 14.75) < 0.1;
    bool hit_right = abs(position.x - 14.75) < 0.1;
    bool hit_up = abs(position.y - 14.75) < 0.1;
    bool hit_down = abs(position.y + 14.75) < 0.1;

    int board_hit = board.hit_detect(position);

    if ((hit_left || board_hit & 1) && velocity.x < 0) {
        velocity.x = -velocity.x * BOUNCE_FACTOR;
        std::cerr << "HIT LEFT" << std::endl;
        hit = 1;
    }

    if ((hit_right || board_hit & 2) && velocity.x > 0) {
        velocity.x = -velocity.x * BOUNCE_FACTOR;
        std::cerr << "HIT RIGHT" << std::endl;
        hit = 1;
    }

    if ((hit_up || board_hit & 4) && velocity.y > 0) {
        velocity.y = -velocity.y * BOUNCE_FACTOR;
        std::cerr << "HIT UP" << std::endl;
        hit = 1;
    }

    if ((hit_down || board_hit& 8) && velocity.y < 0) {
        velocity.y = -velocity.y *BOUNCE_FACTOR;
        std::cerr << "HIT DOWN" << std::endl;
        hit = 1;
    }

    return hit;
}