#include "Enemy.hpp"

Enemy::Enemy(float x, float y, Enemy::DIRECTION d) {
    this->position.x = x;
    this->position.y = y;
}

Enemy::~Enemy() {
    
}

glm::mat4 Enemy::get_view_matrix(Board board) {

}

glm::mat4 Enemy::get_cone_matrix(Board board) {

}

bool Enemy::intercept_with(BigBoss &b) {

}

void Enemy::update(float elapsed) {

}