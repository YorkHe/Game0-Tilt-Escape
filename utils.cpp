#include "utils.h"

float get_height(float rotate_horizontal, float rotate_vertical, glm::vec2 position) {
    return -rotate_horizontal * position.x+ position.y* rotate_vertical * (1-rotate_horizontal);
}