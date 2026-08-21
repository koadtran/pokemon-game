#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

class Map;
using map_t = Map;

class World {
public:
    map_t *maps[401][401];
    int map_r;
    int map_c;
    int pc_r;
    int pc_c;
    char message[81];
    int numtrainers;
    int jumped;
    int quit_game;

    void init();
    void free_all();
    void print();
    void move_charaters();
    void handle_input(int input, int *offset);
};

using world_t = World;

#endif