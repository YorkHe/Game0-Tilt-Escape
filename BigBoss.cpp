#include "BigBoss.hpp"

BigBoss::BigBoss(float x, float y) {
    this->position = glm::vec2(x, y);
}

BigBoss::~BigBoss() {

}

glm::mat4 BigBoss::get_view_matrix() {
    return glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        this->position.x - 6, this->position.y + 6, 0.0f, 1.0f
    );
}

void BigBoss::update(float elapsed) {
    // float amt = elapsed * 1.0f;
    this->position = this->position + this->velocity * elapsed;
}

