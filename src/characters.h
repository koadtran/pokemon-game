#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <stdint.h>
#include <vector>

typedef struct heap heap_t;

class Map;
using map_t = Map;

class World;
using world_t = World;

class Spawned_pokemon;

class Character {
public:
    int r;
    int c;
    char type;
    int costs[10];
    int next;
    int dir[2];
    int defeated = 0;
    int is_wild = 0;
    int escape_attempts = 0;

    int pokemon = 0;
    std::vector<Spawned_pokemon> pokemons;
    
    std::vector<int> inventory;
};

using char_t = Character;

extern int pc_costs[10];
extern int hiker_costs[10];
extern int rival_costs[10];
extern int swimmer_costs[10];

const int default_potions = 6;
const int default_revives = 3;
const int default_pokeballs = 5;

int32_t cmp_npc(const void *key, const void *with);
void update_distance_maps_after_pc_move(world_t *world);
void place_npcs(world_t *world, int numtrainers);
void place_pc(world_t *world);
void move_npc(char_t *npc, world_t *world);
void move_pc(world_t *world, int dr, int dc);
void print_character_list(world_t *world, int offset);
void enter_building(world_t *world);
void pokemon_battle(world_t *world, char_t &attackingt, char_t &defendingt);
void open_bag(world_t *world);

#endif