#include <iostream>
#include <ncurses.h>
#include <time.h>

#include "world.h"
#include "map.h"
#include "distance.h"
#include "characters.h"
#include "data.h"

using namespace std;

void init_ui() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    //init colors
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_CYAN, COLOR_BLACK);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    world_t world;
    world.init();

    int num_npc = 10;
    int max_numtrainers = 30;
    if (argc > 1) {
        num_npc = atoi(argv[1]);
        num_npc = 10;
    }
    if (num_npc > max_numtrainers) {
        num_npc = max_numtrainers;
    }
    world.numtrainers = num_npc;

    read_files();
    
    init_ui();
    
    world.maps[world.map_r][world.map_c] = new map_t();
    if (!world.maps[world.map_r][world.map_c]) {
        exit(1);
    }
    generate_map(world.maps, world.map_r, world.map_c);
    place_npcs(&world, world.numtrainers);
    place_pc(&world);
    world.print();

    heap_t h;
    int offset = 0;
    while (!world.quit_game) {
        if (world.jumped) {
            //insert all characters into heap
            heap_t new_heap;
            heap_init(&new_heap, cmp_npc, NULL);
            for (int r = 0; r < 21; r++) {
                for (int c = 0; c < 80; c++) {
                    if (world.maps[world.map_r][world.map_c]->occupancy[r][c]) {
                        heap_insert(&new_heap, world.maps[world.map_r][world.map_c]->occupancy[r][c]);
                    }
                }
            }
            h = new_heap;
            world.jumped = 0;
            world.print();
            continue;
        }
        char_t *ch = (char_t *) heap_remove_min(&h);

        if (ch->type == '@') {
            int input = getch();
            world.handle_input(input, &offset);
        } else {
            move_npc(ch, &world);
        }
        heap_insert(&h, ch);
    }
    world.free_all();
    endwin();
    return 0;
}