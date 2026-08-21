#ifndef MAP_H
#define MAP_H

class Character;
using char_t = Character;

class World;
using world_t = World;

class Spawned_pokemon;

class Map {
public:
    int N, S, E, W;
    char tiles[21][80];
    int x, y;
    char_t *occupancy[21][80];
    int distance_hiker[21][80];
    int distance_rival[21][80];

    Map() {
        for (int r = 0; r < 21; r++) {
            for (int c = 0; c < 80; c++) {
                occupancy[r][c] = NULL;
            }
        }
    }
};

using map_t = Map;

void generate_map(map_t *world[401][401], int r, int c);
void print_map(map_t *map);
bool spawn_pokemon(world_t *world, Spawned_pokemon &out);

#endif
