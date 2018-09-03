//
// Created by 何宇 on 2018/9/2.
//

#include "Board.h"

#include <iostream>

int Board::hit_detect(glm::vec2 position){

    int position_x = 15 + round(position.x);
    int position_y = 15 - round(position.y);

    // Reference From https://jonathanwhiting.com/tutorial/collision/

    float object_left = position.x - 0.375f;
    float object_right = position.x + 0.375f;
    float object_up = position.y + 0.375f;
    float object_down = position.y - 0.375f;

    int result = 0;

    if (map[position_y][position_x] == 2) {
        std::cerr << "HIT CHECKPOINT" << std::endl;
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
            map[1][27] = 5; map[1][28] = 5; map[1][29] = 5;
            map[2][27] = 5; map[2][28] = 5; map[2][29] = 5;
            map[3][27] = 5; map[3][28] = 5; map[3][29] = 5;
        }
    }

    if (map[position_y][position_x] == 5) {
        level_clear = true;
    }

    for (int x = position_x - 1; x <= position_x + 1; x++) {
        if (x < 0 || x >= 30) continue;
        for (int y = position_y - 1; y <= position_y + 1; y++) {
            if (y < 0 || y >= 30) continue;

            if (map[y][x] == 1) {


                float coordinate_x = x - 14.5f;
                float coordinate_y = 14.5f - y;

                float tile_left = coordinate_x - 0.5f;
                float tile_right = coordinate_x + 0.5f;
                float tile_up = coordinate_y + 0.5f;
                float tile_down = coordinate_y - 0.5f;

//                std::cerr << x << "<" << y << std::endl;
//                std::cerr << "(" << tile_left << "," << tile_right << "," << tile_up << "," << tile_down << ")" << std::endl;
//                std::cerr << "(" << object_left<< "," << object_right<< "," << object_up << "," << object_down << ")" << std::endl;

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
