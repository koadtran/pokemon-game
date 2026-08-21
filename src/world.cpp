#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ncurses.h>

#include "map.h"
#include "distance.h"
#include "characters.h"
#include "world.h"

void World::init() {
    jumped = 1;
    quit_game = 0;
    pc_r = -1;
    pc_c = -1;
    map_r = 200;
    map_c = 200;
    for (int r = 0; r < 401; r++) {
        for (int c = 0; c < 401; c++) {
            maps[r][c] = NULL;
        }
    }
    strcpy(message, "");
}

void World::free_all() {
    for (int r = 0; r < 401; r++) {
        for (int c = 0; c < 401; c++) {
            if (maps[r][c] != NULL) {
                delete(maps[r][c]);
            }
        }
    }
}

int read_number(const char *prompt) {
    char buf[10] = "";
    int len = 0;
    int ch;

    mvprintw(0, 0, "%s: ", prompt);
    clrtoeol();
    refresh();

    while ((ch = getch()) != '\n' && ch != KEY_ENTER) {
        if (ch == KEY_BACKSPACE || ch == 127) {
            if (len > 0) {
                buf[--len] = '\0';
            }
        } else if ((ch >= '0' && ch <= '9') || (ch == '-' && len == 0)) {
            if (len < 9) {
                buf[len++] = ch;
                buf[len] = '\0';
            }
        }
        mvprintw(0, 0, "%s: %s ", prompt, buf);
        clrtoeol();
        refresh();
    }

    if (len == 0) return INT_MIN;
    return atoi(buf);
}

void World::print() {
    clear();
    mvprintw(0, 0, "%s", message);
    print_map(maps[map_r][map_c]);
    refresh();
}

void World::handle_input(int input, int *offset) {
    switch (input) {
        case '7':
        case 'y':
            move_pc(this, -1, -1);
            print();
            break;
        case '8':
        case 'k':
            move_pc(this, -1, 0);
            print();
            break;
        case '9':
        case 'u':
            move_pc(this, -1, 1);
            print();
            break;
        case '6':
        case 'l':
            move_pc(this, 0, 1);
            print();
            break;
        case '3':
        case 'n':
            move_pc(this, 1, 1);
            print();
            break;
        case '2':
        case 'j':
            move_pc(this, 1, 0);
            print();
            break;
        case '1':
        case 'b':
            move_pc(this, 1, -1);
            print();
            break;
        case '4':
        case 'h':
            move_pc(this, 0, -1);
            print();
            break;
        case '5':
        case ' ':
        case '.':
            move_pc(this, 0, 0);
            print();
            break;
        case 'f': {
            int x;
            do {
                x = read_number("Enter x coordinate");
            } while (x > 200 || x < -200 || x == INT_MIN);
            int y;
            do {
                y = read_number("Enter y coordinate");
            } while (y > 200 || y < -200 || y == INT_MIN);
            
            int new_map_r = 200 - y;
            int new_map_c = x + 200;

            char_t *pc = maps[map_r][map_c]->occupancy[pc_r][pc_c];

            maps[map_r][map_c]->occupancy[pc_r][pc_c] = NULL;
            map_r = new_map_r;
            map_c = new_map_c;

            if (maps[map_r][map_c] == NULL) {
                maps[map_r][map_c] = new Map();
                generate_map(maps, map_r, map_c);
                place_npcs(this, numtrainers);
            }

            map_t *new_map = maps[map_r][map_c];

            for (int r = 0; r < 21; r++) {
                for (int c = 0; c < 21; c++) {
                    if (new_map->tiles[r][c] == '#' && new_map->occupancy[r][c] == NULL) {
                        pc_r = r;
                        pc_c = c;
                        new_map->occupancy[r][c] = pc;
                        pc->r = r;
                        pc->c = c;
                        goto placed;
                    }
                }
            }
            placed:
            jumped = 1;
            strcpy(message, "");
            update_distance_maps_after_pc_move(this);
            print();
            break;
        }
        case 't': {
            int ch;
            print_character_list(this, *offset);

            while ((ch = getch()) != 27) { 

                int count = 0;

                for (int r = 0; r < 21; r++) {
                    for (int c = 0; c < 80; c++) {
                        if (maps[map_r][map_c]->occupancy[r][c] != NULL) {
                            if (maps[map_r][map_c]->occupancy[r][c]->type != '@') {
                                count++;
                            }
                        }
                    }
                }

                int visible = 21;
                int max_offset = count - visible;
                if (max_offset < 0) max_offset = 0;

                if (ch == KEY_DOWN) {
                    (*offset)++;
                } else if (ch == KEY_UP) {
                    (*offset)--;
                }

                if (*offset < 0) *offset = 0;
                if (*offset > max_offset) *offset = max_offset;

                print_character_list(this, *offset); 
            }
            print();
            break;
        }
        case '>':
            enter_building(this);
            print();
            //will need code for exiting building 
            break;
        case 'B':
            open_bag(this);
            print();
            break;
        case 'q':
            quit_game = 1;
            break;
    }
}

