//
// Created by 何宇 on 2018/9/2.
//

#include "Board.h"

#include <iostream>

void Board::init_map() {
    for (int x = 0; x < 30; x ++) {
        for (int y = 0; y < 30; y++) {
            if (map[x][y] == 3) {
                map[x][y] = 2;
            } else {
                if (map[x][y] == 5) {
                    map[x][y] = 4;
                }
            }
        }
    }
}

int Board::hit_detect(glm::vec2 position){

    int position_x = 15 + round(position.x);
    int position_y = 15 - round(position.y);

    // Reference From https://jonathanwhiting.com/tutorial/collision/

    float object_left = position.x - 0.5f;
    float object_right = position.x + 0.5f;
    float object_up = position.y + 0.5f;
    float object_down = position.y - 0.5f;

    int result = 0;

    if (map[position_y][position_x] == 2) {
        for (int x = position_x - 2; x <= position_x + 2; x ++) {
            if (x < 0 || x >= 30) continue;
            for (int y = position_y - 2; y<=position_y + 2; y++) {
                if (y < 0 || y >= 30) continue;

                if (map[y][x] == 2) {
                    map[y][x] = 3;
                }
            }
        }
        checkpoint_counter ++;

        if (checkpoint_counter == 4) {
            map[0][27] = 5; map[0][28] = 5; map[0][29] = 5;
            map[1][27] = 5; map[1][28] = 5; map[1][29] = 5;
            map[2][27] = 5; map[2][28] = 5; map[2][29] = 5;
        }
    }

    if (map[position_y][position_x] == 5) {
        this->level_clear = true;
        std::cerr << "LEVEL CLEAR" << std::endl;
    }

    for (int x = position_x - 2; x <= position_x + 2; x++) {
        if (x < 0 || x >= 30) continue;
        for (int y = position_y - 2; y <= position_y + 2; y++) {
            if (y < 0 || y >= 30) continue;

            if (map[y][x] == 1) {


                float coordinate_x = x - 14.5f;
                float coordinate_y = 14.5f - y;

                float tile_left = coordinate_x - 0.5f;
                float tile_right = coordinate_x + 0.5f;
                float tile_up = coordinate_y + 0.5f;
                float tile_down = coordinate_y - 0.5f;

                bool x_overlaps_left = (object_left <= tile_left) && (object_right <= tile_right) && (tile_left <= object_right);
                bool x_overlaps_right = (tile_left <= object_left) && (tile_right <= object_right) && (object_left <= tile_right);
                bool y_overlaps_up = (object_up >= tile_up) && (object_down >= tile_down) && (tile_up >= object_down);
                bool y_overlaps_down = (tile_up >= object_up) && (tile_down >= object_down) && (object_up >= tile_down);

                if (x_overlaps_left && (y_overlaps_up || y_overlaps_down)) result |= 2;
                if (x_overlaps_right && (y_overlaps_up || y_overlaps_down)) result |= 1;
                if (y_overlaps_up && (x_overlaps_left || x_overlaps_right)) result |= 8;
                if (y_overlaps_down && (x_overlaps_left || x_overlaps_right)) result |= 4;
            }
        }
    }

    return result;
}
