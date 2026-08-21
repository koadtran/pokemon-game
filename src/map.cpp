#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

#include "map.h"
#include "characters.h"
#include "data.h"
#include "world.h"

#define WHITE 1;
#define RED 2;
#define GREEN 3;
#define YELLOW 4;
#define BLUE 5;
#define MAGENTA 6;
#define CYAN 7;

// ORIGNALLY IN regions.cpp
int the_queue[1680][2];
int head = 0;
int tail = 0;

void push(int r, int c) {
    if (tail < 1680) {
        the_queue[tail][0] = r;
        the_queue[tail][1] = c;
        tail++;
    }
}

int pop(int *r, int *c) {
    if (head == tail) {
        return 0;
    }
    *r = the_queue[head][0];
    *c = the_queue[head][1];
    head++;
    return 1;
}

void flood_fill_search(char map[21][80], int visited[21][80], int r, int c) {
    head = tail = 0;
    char terrain = map[r][c];

    push(r, c);
    visited[r][c] = 1;

    int row, col;
    while (pop(&row, &col)) {
        int dirs[8][2] = {
            {-1,0},{1,0},{0,-1},{0,1},
            {-1,-1},{-1,1},{1,-1},{1,1}
        };
        for (int i = 0; i < 8; i++) {
            int nr = row + dirs[i][0];
            int nc = col + dirs[i][1];
            if (nr <= 0 || nr >= 20 || nc <= 0 || nc >= 79) {
                continue;
            }
            if (!visited[nr][nc] && map[nr][nc] == terrain){
                push(nr, nc);
                visited[nr][nc] = 1;
            }
        }
    }
}

void count_regions(char map[21][80], int *s, int *t, int *w, int *m) {
    int visited[21][80] = {{0}};

    *s = *t = *w = *m = 0;

    for (int r = 1; r <= 19; r++) {
        for (int c = 1; c <= 78; c++) {
            if (!visited[r][c]) {
                char terrain = map[r][c];
                switch (terrain) {
                    case '.':
                        (*s)++; 
                        break;
                    case ':':  
                        (*t)++; 
                        break;
                    case '~': 
                        (*w)++; 
                        break;
                    case '&': 
                        (*m)++; 
                        break;
                }
                flood_fill_search(map, visited, r, c);
            }
        }
    }
}

void make_walls(char map[21][80]) {
    for (int col = 0; col < 80; col++) {
        map[0][col] = '%';
        map[20][col] = '%';
    }
    for (int row = 1; row <= 19; row++) {
        map[row][0] = '%';
        map[row][79] = '%';
    }
}

void fill_random(char map[21][80]) {
    for (int r = 0; r < 21; r++) {
        for (int c = 0; c < 80; c++) {
            int chance = rand() % 100;
            if (chance <= 25) {
                map[r][c] = ':';
            } else if (chance <= 53) {
                map[r][c] = '~';
            } else if (chance <= 82) {
                map[r][c] = '&';
            } else {
                map[r][c] = '.';
            }
        }
    }
}

char count_neighbors(char map[21][80], int r, int c) {
    char terrain;
    int s = 0, t = 0, w = 0, m = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) {
                continue;
            }
            int r_neighbor = r + dr;
            int c_neighbor = c + dc;
            if (r_neighbor <= 0 || r_neighbor >= 20 || c_neighbor <= 0 || c_neighbor >= 79) {
                continue;
            }
            switch (map[r_neighbor][c_neighbor]) {
                case '.':
                    s++;
                    break;
                case ':':
                    t++;
                    break;
                case '~':
                    w++;
                    break;
                case '&':
                    m++;
                    break;
            }
        }
    }
    int max = s;
    terrain = '.';
    if (t > max) {
        max = t;
        terrain = ':';
    }
    if (w > max) {
        max = w;
        terrain = '~';
    }
    if (m > max) {
        max = m;
        terrain = '&';
    }
    return terrain;
}

void count_and_fill(char old_map[21][80]) {
    char new_map[21][80];
    for (int r = 0; r < 21; r++) {
        for (int c = 0; c < 80; c++) {
            new_map[r][c] = old_map[r][c];
        }
    }

    for (int r = 1; r <= 19; r++) {
        for (int c = 1; c <= 78; c++) {
            new_map[r][c] = count_neighbors(old_map, r, c);
        }
    }

    for (int r = 0; r < 21; r++) {
        for (int c = 0; c < 80; c++) {
            old_map[r][c] = new_map[r][c];
        }
    }
}

void fill_terrains(char map[21][80]) {
    fill_random(map);
    make_walls(map);
    for (int i = 1; i <= 10; i++) {
        count_and_fill(map);
    }
    int tree_num = rand() % 40;
    for (int i = 1; i <= tree_num; i++){
        int r = rand() % 19 + 1; 
        int c = rand() % 78 + 1;
        if (map[r][c] != '~') {
            map[r][c] = '^';
        }
    }
}

void check_count(char map[21][80]) {
    int s = 0;
    int t = 0;
    int w = 0;
    int m = 0;
    count_regions(map, &s, &t, &w, &m);

    int attempts = 1;
    int max_attempts = 50;
    while ((s < 2 || t < 2 || w < 1) && attempts < max_attempts) {
        fill_terrains(map);
        count_regions(map, &s, &t, &w, &m);
        attempts++;
    }
}

void connect_ns(char map[21][80], int N, int S) {
    map[0][N] = 'g';
    map[20][S] = 'g';

    int diff = S - N;
    int step = 1;
    if (diff < 0) {
        step = -1;
    }
    int curr = N;
    for (int r = 1; r <= 19; r++) {
        if (diff != 0) {
            if (abs(diff) < 19 - r) {
                if (rand() % 2) {
                    //map[r][curr] = '#';
                    curr += step;
                    diff -= step;
                }
            } else {
                //map[r][curr] = '#';
                curr += step;
                diff -= step;
            }
        }
        map[r][curr] = '#';
    }
}

void connect_ew(char map[21][80], int E, int W) {
    map[W][0] = 'g';
    map[E][79] = 'g';

    int diff = E - W;
    int step = 1;
    if (diff < 0) {
        step = -1;
    }
    
    int curr = W;
    for (int c = 1; c <= 78; c++) {
        if (diff != 0) {
            if (abs(diff) < 78 - c) {
                if (rand() % 100 < 10) {
                    //map[curr][c] = '#';
                    curr += step;
                    diff -= step;
                }
            } else {
                //map[curr][c] = '#';
                curr += step;
                diff -= step;
            }
        }
        map[curr][c] = '#';
    }
}

void find_intersection(char map[21][80], int *ir, int *ic) {
    *ir = -1;
    *ic = -1;
    for (int r = 4; r <= 20 - 4; r++) {
        for (int c = 4; c <= 79 - 4; c++) {
            if (map[r][c] == '#') {
                int count = 0;
                //check eight neighbor cells
                int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
                for (int i = 0; i < 4; i++) {
                    int nr = r + dirs[i][0];
                    int nc = c + dirs[i][1];
                    if (map[nr][nc] == '#'){
                        count++;
                    }
                }
                if (count >= 3) {
                    *ir = r;
                    *ic = c;
                    return;
                }
            }
        }
    }
}

void make_buildings(char map[21][80]) {
    int ir = -1;
    int ic = -1;
    find_intersection(map, &ir, &ic);

    char buildings[2] = {'C', 'M'};
    if (rand() % 2) {
        char temp = buildings[0];
        buildings[0] = buildings[1];
        buildings[1] = temp;
    }

    //NS
    int r = -1;
    if (ir >= 10) {
        r = ir - 7;
    } else {
        r = ir + 7;
    }
    int c;
    for (c = 0; c <= 79; c++) {
        if (map[r][c] == '#') {
            break;
        }
    }

    int step;
    if (c <= 40) {
        step = 1;
    } else {
        step = -1;
    }

    int distance = rand() % 4 + 7;
    while (distance > 0) {
        distance--;
        c += step;
        map[r][c] = '#';
    }
    int size = rand() % 2 + 2;
    for (int dr = -1; dr <= size - 2; dr++) {
        for (int dc = -1; dc <= size - 2; dc++) {
            map[r + dr][c + dc] = buildings[0];
        }
    }

    //EW
    c = -1;
    if (ic > 40) {
        c = ic - (rand() % 10 + 13);
    } else {
        c = ic + (rand() % 10 + 13);
    }

    for (r = 0; r <= 20; r++) {
        if (map[r][c] == '#') {
            break;
        }
    }

    if (r < 10) {
        step = 1;
    } else {
        step = -1;
    }

    distance = rand() % 4 + 3;
    while (distance > 0) {
        distance--;
        r += step;
        map[r][c] = '#';
    }
    size = rand() % 2 + 2;
    for (int dr = -1; dr <= size - 2; dr++) {
        for (int dc = -1; dc <= size - 2; dc++) {
            map[r + dr][c + dc] = buildings[1];
        }
    }
}

int get_color(char terrain) {
    switch (terrain) {
        case '%':
            return RED;
            break;
        case '^':
            return GREEN;
            break;
        case 'g':
        case '#':
        case 'M':
        case 'C':
            return WHITE;
            break;
        case ':':
            return CYAN;
            break;
        case '.':
            return GREEN;
            break;
        case '&':
            return RED;
            break;
        case '~':
            return BLUE;
            break;
        case '@':
            return MAGENTA;
            break;
        default:
            return YELLOW;
    }
}

void print_map(map_t *map) {
    for (int row = 0; row < 21; row++) {
        for (int col = 0; col < 80; col++) {
            char to_print;
            if (map->occupancy[row][col] != NULL) {
                to_print = map->occupancy[row][col]->type;
            } else if (map->tiles[row][col] == 'g') {
                to_print = '#';
            } else if (map->tiles[row][col] == '&') {
                to_print = '%';
            } else {
                to_print = map->tiles[row][col];
            }
            attron(COLOR_PAIR(get_color(to_print)));
            mvprintw(row + 1, col, "%c", to_print);
            attroff(COLOR_PAIR(get_color(to_print)));
        }
    }
}

void make_map_with_exits(map_t *map) {
    map->N = (map->N == -1) ? rand() % 20 + 30 : map->N;
    map->S = (map->S == -1) ? rand() % 20 + 30 : map->S;
    map->E = (map->E == -1) ? 4 + rand() % 13 : map->E;
    map->W = (map->W == -1) ? 4 + rand() % 13 : map->W;

    fill_terrains(map->tiles);
    check_count(map->tiles);

    connect_ns(map->tiles, map->N, map->S);
    connect_ew(map->tiles, map->E, map->W);

    //calculate probability for buiding spawning using x and y
    int d = abs(map->x) + abs(map->y);
    int p = (-45 * d) / 200 + 50;

    if (d == 0) {
        p = 100;
    }

    if (d >= 200) {
        p = 5;
    }

    if (rand() % 100 <= p) {
        make_buildings(map->tiles);
    }
}

void generate_map(map_t *world[401][401], int r, int c) {
    world[r][c]->y = 200 - r;
    world[r][c]->x = c - 200;

    world[r][c]->N = -1;
    world[r][c]->S = -1;
    world[r][c]->E = -1;
    world[r][c]->W = -1;
    //find exits from 4 neighbors: top, left, right, bottom
    int dr[4] = {-1, 0, 0, 1};
    int dc[4] = {0, -1 , 1, 0};
    for (int i = 0; i < 4; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];

        if (nr < 0 || nr > 400 || nc < 0 || nc > 400) {
            continue;
        }
        if (world[nr][nc] == NULL) {
            continue;
        }
        switch (i) {
            case 0:
                world[r][c]->N = world[nr][nc]->S;
                break;
            case 1: 
                world[r][c]->W = world[nr][nc]->E;
                break;
            case 2: 
                world[r][c]->E = world[nr][nc]->W;
                break;
            case 3:
                world[r][c]->S = world[nr][nc]->N;
                break;
        }
    }
    make_map_with_exits(world[r][c]);

    //check maps on edges
    if (r == 0) {
        world[r][c]->tiles[0][world[r][c]->N] = '%';
    }
    if (r == 400) {
        world[r][c]->tiles[20][world[r][c]->S] = '%';
    }
    if (c == 0) {
        world[r][c]->tiles[world[r][c]->W][0] = '%';
    }
    if (c == 400) {
        world[r][c]->tiles[world[r][c]->E][79] = '%';
    }
}

bool spawn_pokemon(world_t *world, Spawned_pokemon &out) {
    if (rand() % 100 < 10) {
        map_t *map = world->maps[world->map_r][world->map_c];
        out = Spawned_pokemon(abs(map->x) + abs(map->y));
        return true;
    }
    return false;
}

