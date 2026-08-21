#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "map.h"
#include "characters.h"
#include "distance.h"
#include "world.h"
#include "data.h"

int pc_costs[10] = {INT_MAX, INT_MAX, 10, 10, 10, 20, 10, INT_MAX, INT_MAX, 10}; //last one is for gates, disabled for now
int hiker_costs[10] = {INT_MAX, INT_MAX, 10, 50, 50, 15, 10, 15, INT_MAX, INT_MAX};
int rival_costs[10] = {INT_MAX, INT_MAX, 10, 50, 50, 20, 10, INT_MAX, INT_MAX, INT_MAX};
int swimmer_costs[10] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 7, INT_MAX};
//others have same costs as rival

//compare to feed to heap
int32_t cmp_npc(const void *key, const void *with) {
    return ((char_t *) key)->next - ((char_t *) with)->next;
}

//helper to check if a character can move to a certain location
int can_move(map_t *map, char_t *charater, int r, int c) {
    if ((r < 0 || r >= 21 || c < 0 || c >= 80) && charater->type != '@') {
        return 0;
    }

    if (map->occupancy[r][c] != NULL) {
        return 0;
    }

    int cost;
    char terrain = map->tiles[r][c];
    switch (terrain) {
        case '%':
            cost = charater->costs[0];
            break;
        case '^':
            cost = charater->costs[1];
            break;
        case '#':
            cost = charater->costs[2];
            break;
        case 'M':
            cost = charater->costs[3];
            break;
        case 'C':
            cost = charater->costs[4];
            break;
        case ':':
            cost = charater->costs[5];
            break;
        case '.':
            cost = charater->costs[6];
            break;
        case '&':
            cost = charater->costs[7];
            break;
        case '~':
            cost = charater->costs[8];
            break;
        case 'g':
            cost = charater->costs[9];
            break;
        default:
            return 0;
    }
    if (cost == INT_MAX) {
        return 0;
    }
    return 1;
}

//helper to place one npc
void place_one_npc(char type, world_t *world) {
    map_t *map = world->maps[world->map_r][world->map_c];
    char_t *npc = new char_t();

    switch (type) {
        case 'h':
            memcpy(npc->costs, hiker_costs, sizeof(int) * 10);
            break;
        case 'r':
            memcpy(npc->costs, rival_costs, sizeof(int) * 10);
            break;
        case 'p':
        case 'w':
        case 's':
        case 'e':
            memcpy(npc->costs, rival_costs, sizeof(int) * 10);
            break;
        default:
            delete npc;
            return;
    }

    int prow, pcol;

    do {
        prow = rand() % 19 + 1;
        pcol = rand() % 78 + 1;
    } while (can_move(map, npc, prow, pcol) == 0);

    npc->r = prow;
    npc->c = pcol;
    npc->type = type;
    npc->next = 0;
    npc->dir[0] = 0;
    npc->dir[1] = 0;
    npc->defeated = 0;
    npc->is_wild = 0;
    npc->escape_attempts = 0;
    npc->pokemon = 0;
    npc->pokemons = {};

    //put in occupancy array
    map->occupancy[prow][pcol] = npc;

    npc->pokemons.push_back(Spawned_pokemon(abs(map->x) + abs(map->y)));
 
    // 60% chance for each additional
    while (npc->pokemons.size() < 6 && rand() % 100 < 60) {
        npc->pokemons.push_back(Spawned_pokemon(abs(map->x) + abs(map->y)));
    }
}

void place_pc(world_t *world) {
    map_t *map = world->maps[world->map_r][world->map_c];
    char_t *pc = new char_t();

    int prow, pcol;
    
    if (world->pc_r == -1 && world->pc_c == -1) {
        do {
            prow = rand() % 19 + 1;
            pcol = rand() % 78 + 1;
        } while (map->tiles[prow][pcol] != '#');
    } else {
        prow = world->pc_r;
        pcol = world->pc_c;
    }

    pc->r = prow;
    pc->c = pcol;
    pc->type = '@';
    memcpy(pc->costs, pc_costs, sizeof(int) * 10);
    pc->next = 0;
    pc->dir[0] = 0;
    pc->dir[1] = 0;
    pc->pokemons = {};

    //put in occupancy array
    map->occupancy[prow][pcol] = pc;

    world->pc_r = prow;
    world->pc_c = pcol;

    update_distance_maps_after_pc_move(world);

    pc->inventory.push_back(default_potions);
    pc->inventory.push_back(default_revives);
    pc->inventory.push_back(default_pokeballs);
    
    Spawned_pokemon p1 = Spawned_pokemon(abs(map->x) + abs(map->y));
    Spawned_pokemon p2 = Spawned_pokemon(abs(map->x) + abs(map->y));
    Spawned_pokemon p3 = Spawned_pokemon(abs(map->x) + abs(map->y));

    clear();
    mvprintw(0, 0, "Choose your first Pokemon! Enter 1, 2, or 3 to choose.");
    mvprintw(2, 0, ("1. " + p1.to_string()).c_str());
    mvprintw(7, 0, ("2. " + p2.to_string()).c_str());
    mvprintw(12, 0, ("3. " +p3.to_string()).c_str());
    refresh();

    char ch;
    do {
        ch = getch();
    } while ((ch != '1') && (ch != '2') && (ch != '3') && (ch != 'q'));
    switch (ch) {
        case '1': pc->pokemons.push_back(p1); break;
        case '2': pc->pokemons.push_back(p2); break;
        case '3': pc->pokemons.push_back(p3); break;
        case 'q': exit(0);
    }
}
//place characters and put them in the occupancy array
void place_npcs(world_t *world, int numtrainers) {
    //place pc
    map_t *map = world->maps[world->map_r][world->map_c];

    if (numtrainers == 0) {
        return;
    }

    //numtrainers now at least 1
    //place hikers
    place_one_npc('h', world);


    if (numtrainers >= 2) {
        //place rival
        place_one_npc('r', world);
    }

    //place other npc;
    char symbols[4] = {'p', 'w', 's', 'e'}; 
    for (int i = 3; i <= numtrainers; i++) {
        char type = symbols[(i + 1) % 4];
        place_one_npc(type, world);
    }

    //generate distance maps for hiker, rival
    find_distance(map->distance_hiker, map, world->pc_r, world->pc_c, hiker_costs);
    find_distance(map->distance_rival, map, world->pc_r, world->pc_c, rival_costs);
}

void move_gradient(map_t *map, char_t *npc, int current_time) {
    //free current location in occupancy array
    int old_r = npc->r, old_c = npc->c;
    int best_r = old_r;
    int best_c = old_c;

    int best_d;

    if (npc->type == 'h') {
        best_d = map->distance_hiker[old_r][old_c];
    } else {
        best_d = map->distance_rival[old_r][old_c];
    }
    //find new r and c
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) {
                continue;
            }

            int nr = old_r + dr;
            int nc = old_c + dc;

            if (nr < 0 || nr >= 21 || nc < 0 || nc >= 80) {
                continue;
            }

            if (map->distance_hiker[nr][nc] == INT_MAX) {
                continue;
            }

            if (can_move(map, npc, nr, nc) && (map->distance_hiker[nr][nc]) < (map->distance_hiker[best_r][best_c])) {
                int nd = (npc->type == 'h') ? map->distance_hiker[nr][nc] : map->distance_rival[nr][nc];
                if (nd < best_d) {
                    best_d = nd;
                    best_r = nr;
                    best_c = nc;
                }
            }
        }
    }

    if (best_r != old_r || best_c != old_c) {
        map->occupancy[old_r][old_c] = NULL;
        npc->r = best_r;
        npc->c = best_c;
        map->occupancy[best_r][best_c] = npc;
    }

    //update next
    int cost;
    switch (map->tiles[npc->r][npc->c]) {
        case '%':
            cost = npc->costs[0];
            break;
        case '^':
            cost = npc->costs[1];
            break;
        case '#':
            cost = npc->costs[2];
            break;
        case 'M':
            cost = npc->costs[3];
            break;
        case 'C':
            cost = npc->costs[4];
            break;
        case ':':
            cost = npc->costs[5];
            break;
        case '.':
            cost = npc->costs[6];
            break;
        case '&':
            cost = npc->costs[7];
            break;
        case '~':
            cost = npc->costs[8];
            break;
        case 'g':
            cost = npc->costs[9];
            break;
        default:
            cost = INT_MAX;
    }
    if (cost == INT_MAX) {
        cost = 10;
    }
    npc->next = current_time + cost;
}

void move_others(world_t *world, map_t *map, char_t *npc, int current_time) {
    if (npc->type == 's') {
        npc->next = current_time + 50;
        return;
    }
    int dirs[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    int new_dir;
    if (npc->dir[0] == 0 && npc->dir[1] == 0) {
        new_dir = rand() % 8;
        npc->dir[0] = dirs[new_dir][0];
        npc->dir[1] = dirs[new_dir][1];
    }

    int old_r = npc->r, old_c = npc->c;
    int new_r = old_r + npc->dir[0];
    int new_c = old_c + npc->dir[1];

    //not necessary, should never happen, just in case there's a bug in map generation and npc is facing out of the map
    if (new_r < 0 || new_r >= 21 || new_c < 0 || new_c >= 80) {
        npc->next = current_time + 10;
    }

    int wanderer_will_move = (map->tiles[new_r][new_c] == map->tiles[old_r][old_c]); //still same terrain

    if (npc->type == 'w' && !wanderer_will_move) {
        do {
            new_dir = rand() % 8;
        } while (npc->dir[0] == dirs[new_dir][0] && npc->dir[1] == dirs[new_dir][1]);

        npc->dir[0] = dirs[new_dir][0];
        npc->dir[1] = dirs[new_dir][1];
        npc->next = current_time + 10;
        return;
    }

    if (can_move(map, npc, new_r, new_c)) {
        map->occupancy[old_r][old_c] = NULL;
        npc->r = new_r;
        npc->c = new_c;
        map->occupancy[new_r][new_c] = npc;

        if (npc->type == '@') {
            world->pc_r = new_r;
            world->pc_c = new_c;
        }
    } else {
        switch (npc->type) {
            case 'p':
                npc->dir[0] = -npc->dir[0];
                npc->dir[1] = -npc->dir[1];
                break;
            case 'h':
            case 'r':
            case 'w':
            case 'e':
                new_dir = rand() % 8;
                npc->dir[0] = dirs[new_dir][0];
                npc->dir[1] = dirs[new_dir][1];
                break;
            default:
                break;
        }
        npc->next = current_time + 10;
        return;
    }

    //update next
    int cost;
    switch (map->tiles[npc->r][npc->c]) {
        case '%':
            cost = npc->costs[0];
            break;
        case '^':
            cost = npc->costs[1];
            break;
        case '#':
            cost = npc->costs[2];
            break;
        case 'M':
            cost = npc->costs[3];
            break;
        case 'C':
            cost = npc->costs[4];
            break;
        case ':':
            cost = npc->costs[5];
            break;
        case '.':
            cost = npc->costs[6];
            break;
        case '&':
            cost = npc->costs[7];
            break;
        case '~':
            cost = npc->costs[8];
            break;
        case 'g':
            cost = npc->costs[9];
            break;
        default:
            cost = INT_MAX;
    }
    if (cost == INT_MAX) {
        cost = 10;
    }
    npc->next = current_time + cost;
}

void update_distance_maps_after_pc_move(world_t *world) {
    map_t *map = world->maps[world->map_r][world->map_c];
    find_distance(map->distance_hiker, map, world->pc_r, world->pc_c, hiker_costs);
    find_distance(map->distance_rival, map, world->pc_r, world->pc_c, rival_costs);
}

string get_move_name(int move_id) {
    for (auto move : moves_arr) {
        if (move.get_id() == move_id) {
            return move.get_identifier();
        }
    }
    return "";
}

Moves * get_move(Pokemon_moves poke_move) {
    for (auto &move : moves_arr) {
        if (move.get_id() == poke_move.get_move_id()) {
            return &move;
        }
    }
    return nullptr;
}

int get_damage(char_t &attackingt, char_t &defendingt, Moves &move) {
    Spawned_pokemon &attackingp = attackingt.pokemons[attackingt.pokemon];
    Spawned_pokemon &defendingp = defendingt.pokemons[defendingt.pokemon];
    int level = attackingp.level;
    int power = move.get_power();
    int attack = attackingp.stats[1];
    int defense = defendingp.stats[2];
    float critical = ((rand()) % 256 < (attackingp.stats[5] / 2)) ? 1.5 : 1;
    float random = ((rand() % 16) + 85) / 100.0;

    if (power <= 0 || power == INT_MAX) return 0;
    if (defense <= 0) defense = 1;

    int first_type = -1;
    int second_type = -1;
    for (int i = 0; i < (int) pokemon_types_arr.size(); i++) {
        auto poketype = pokemon_types_arr[i];
        if (attackingp.pokemon->get_id() == poketype.get_pokemon_id()) {
            first_type = poketype.get_type_id();
            if (i < (int) pokemon_types_arr.size() - 1) {
                if (attackingp.pokemon->get_id() == pokemon_types_arr[i + 1].get_pokemon_id()) {
                    second_type = pokemon_types_arr[i + 1].get_type_id();
                }
            }
        }
    }
    float stab = (first_type != -1 || second_type != -1) ? 1.5 : 1;
    int type = 1;

    int damage = (int)((((2.0f * level / 5.0f + 2) * power * ((float)attack / defense) / 50.0f) + 2)
                 * critical * random * stab * type);

    return max(1, damage);
}

bool switch_pokemon(char_t &trainer) {
    clear();
    mvprintw(0, 0, "Choose a Pokemon to switch to:");

    vector<int> valid;
    for (int i = 0; i < (int)trainer.pokemons.size(); i++) {
        if (i == trainer.pokemon) continue;
        if (trainer.pokemons[i].stats[0] <= 0) continue;
        valid.push_back(i);
    }

    if (valid.empty()) {
        mvprintw(2, 0, "No Pokemon available to switch to!");
        refresh();
        getch();
        return false;
    }

    for (int i = 0; i < (int)valid.size(); i++) {
        Spawned_pokemon &p = trainer.pokemons[valid[i]];
        mvprintw(i + 2, 0, "%d. %s (HP: %d)",
            i + 1,
            p.pokemon->get_identifier().c_str(),
            p.stats[0]);
    }
    refresh();

    int choice = -1;
    while (choice < 1 || choice > (int)valid.size()) {
        int key = getch();
        if (key >= '1' && key <= '6')
            choice = key - '0';
    }

    trainer.pokemon = valid[choice - 1];
    return true;
}

void handle_faint(char_t &trainer, bool is_pc, bool &battle_over) {
    bool any_alive = false;
    for (auto &p : trainer.pokemons) {
        if (p.stats[0] > 0) { any_alive = true; break; }
    }

    if (!any_alive) {
        battle_over = true;
        return;
    }

    if (is_pc) {
        bool switched = false;
        while (!switched) {
            switched = switch_pokemon(trainer);
        }
    } else {
        for (int i = 0; i < (int)trainer.pokemons.size(); i++) {
            if (trainer.pokemons[i].stats[0] > 0) {
                trainer.pokemon = i;
                break;
            }
        }
    }
}

void wait_for_space(int row = 15) {
    mvprintw(row, 0, "Press SPACE to continue...");
    refresh();
    int k;
    do { k = getch(); } while (k != ' ');
    mvprintw(row, 0, "%-40s", "");
}

void do_move(char_t &attackingt, char_t &defendingt, Moves &move, bool &battle_over) {
    Spawned_pokemon &attackingp = attackingt.pokemons[attackingt.pokemon];
    Spawned_pokemon &defendingp = defendingt.pokemons[defendingt.pokemon];

    mvprintw(10, 0, "                                                                                ");
    mvprintw(11, 0, "                                                                                ");
    mvprintw(12, 0, "                                                                                ");
    mvprintw(13, 0, "                                                                                ");
    mvprintw(14, 0, "                                                                                ");

    int acc = move.get_accuracy();
    if (acc != INT_MAX && rand() % 100 > acc) {
        mvprintw(10, 0, "%s used %s... but it missed!",
            attackingp.pokemon->get_identifier().c_str(),
            move.get_identifier().c_str());
        wait_for_space(11);
        return;
    }

    int dmg = get_damage(attackingt, defendingt, move);
    defendingp.stats[0] = max(defendingp.stats[0] - dmg, 0);

    mvprintw(10, 0, "%s used %s on %s!",
        attackingp.pokemon->get_identifier().c_str(),
        move.get_identifier().c_str(),
        defendingp.pokemon->get_identifier().c_str());
    if (dmg == 0) {
        mvprintw(11, 0, "But it had no effect!");
    } else {
        mvprintw(11, 0, "It dealt %d damage!", dmg);
        mvprintw(12, 0, "%s has %d/%d HP remaining.",
            defendingp.pokemon->get_identifier().c_str(),
            defendingp.stats[0], defendingp.max_hp);
    }

    if (defendingp.stats[0] == 0) {
        mvprintw(13, 0, "%s fainted!", defendingp.pokemon->get_identifier().c_str());
        wait_for_space(14);
        handle_faint(defendingt, defendingt.type == '@', battle_over);
    } else {
        wait_for_space(13);
    }
    
}

void pokemon_battle(world_t *world, char_t &attackingt, char_t &defendingt) {
    bool battle_over = false;
    attackingt.escape_attempts = 0;

    while (!battle_over) {
        Spawned_pokemon &pc_pokemon  = attackingt.pokemons[attackingt.pokemon];
        Spawned_pokemon &npc_pokemon = defendingt.pokemons[defendingt.pokemon];

        // build title based on battle type
        const char *title;
        if (defendingt.is_wild) {
            title = "A wild Pokemon appeared!";
        } else {
            switch (defendingt.type) {
                case 'h': title = "Battle with a Hiker!";   break;
                case 'r': title = "Battle with a Rival!";   break;
                case 'p': title = "Battle with a Pacer!";   break;
                case 'w': title = "Battle with a Wanderer!"; break;
                case 's': title = "Battle with a Sentry!";  break;
                case 'e': title = "Battle with an Explorer!"; break;
                default:  title = "Pokemon Battle!";         break;
            }
        }

        clear();
        mvprintw(0, 0, "%s", title);
        mvprintw(1, 0, "Enemy: %-15s HP: %d/%d",
            npc_pokemon.pokemon->get_identifier().c_str(),
            npc_pokemon.stats[0], npc_pokemon.max_hp);
        mvprintw(2, 0, "Yours: %-15s HP: %d/%d  Lv.%d",
            pc_pokemon.pokemon->get_identifier().c_str(),
            pc_pokemon.stats[0], pc_pokemon.max_hp,
            pc_pokemon.level);
        mvprintw(4, 0, "1. Fight");
        mvprintw(5, 0, "2. Bag");
        mvprintw(6, 0, "3. Run");
        mvprintw(7, 0, "4. Pokemon");
        refresh();

        Moves *pc_move = nullptr;
        bool pc_switched = false;
        bool pc_ran = false;

        int key = getch();
        switch (key) {
        case '1': // Fight
            if (pc_pokemon.stats[0] == 0) {
                mvprintw(10, 0, "%s has fainted! Use the Bag to revive first.",
                pc_pokemon.pokemon->get_identifier().c_str());
                wait_for_space(11);
                continue;
            }
            clear();
            mvprintw(0, 0, "Choose a move:");
            for (int i = 0; i < (int)pc_pokemon.moves.size(); i++) {
                mvprintw(i + 2, 0, "%d. %s", i + 1,
                    get_move_name(pc_pokemon.moves[i]->get_move_id()).c_str());
            }
            refresh();
            {
                int choice = -1;
                while (choice < 1 || choice > (int)pc_pokemon.moves.size()) {
                    int k = getch();
                    if (k >= '1' && k <= '2') choice = k - '0';
                }
                pc_move = get_move(*pc_pokemon.moves[choice - 1]);
            }
            break;

        case '2': { // Bag
            clear();
            mvprintw(0, 0, "Choose an item:");
            mvprintw(2, 0, "1. Potion   (x%d) - restore 20 HP", attackingt.inventory[0]);
            mvprintw(3, 0, "2. Revive   (x%d) - revive fainted pokemon to half HP", attackingt.inventory[1]);
            if (defendingt.is_wild)
                mvprintw(4, 0, "3. Pokeball (x%d) - attempt to catch wild pokemon", attackingt.inventory[2]);
            mvprintw(6, 0, "Press any button to cancel");
            refresh();

            int item_key = getch();

            if (item_key == '1') { // Potion
                if (attackingt.inventory[0] <= 0) {
                    mvprintw(10, 0, "No potions left!"); wait_for_space(11); continue;
                }
                clear();
                mvprintw(0, 0, "Use potion on which Pokemon?");
                vector<int> alive_idx;
                for (int i = 0; i < (int)attackingt.pokemons.size(); i++) {
                    mvprintw((int)alive_idx.size() + 2, 0, "%d. %s  HP: %d/%d",
                        (int)alive_idx.size() + 1,
                        attackingt.pokemons[i].pokemon->get_identifier().c_str(),
                        attackingt.pokemons[i].stats[0],                            attackingt.pokemons[i].max_hp);
                        alive_idx.push_back(i);
                }
                refresh();
                int pick = getch() - '1';
                if (pick >= 0 && pick < (int)alive_idx.size()) {
                    int idx = alive_idx[pick];
                    attackingt.pokemons[idx].stats[0] = min(
                        attackingt.pokemons[idx].stats[0] + 20,
                        attackingt.pokemons[idx].max_hp);
                    attackingt.inventory[0]--;
                    mvprintw(10, 0, "Used potion on %s!",
                        attackingt.pokemons[idx].pokemon->get_identifier().c_str());
                    wait_for_space(11);
                    pc_switched = true; continue;
                } else {
                    mvprintw(10, 0, "Invalid choice."); wait_for_space(11); continue;
                }

            } else if (item_key == '2') { // Revive
                if (attackingt.inventory[1] <= 0) {
                    mvprintw(10, 0, "No revives left!"); wait_for_space(11); continue;
                }
                clear();
                mvprintw(0, 0, "Revive which Pokemon?");
                vector<int> fainted_idx;
                for (int i = 0; i < (int)attackingt.pokemons.size(); i++) {
                    if (attackingt.pokemons[i].stats[0] <= 0) {
                        mvprintw((int)fainted_idx.size() + 2, 0, "%d. %s (fainted)",
                            (int)fainted_idx.size() + 1,
                            attackingt.pokemons[i].pokemon->get_identifier().c_str());
                        fainted_idx.push_back(i);
                    }
                }
                if (fainted_idx.empty()) {
                    mvprintw(10, 0, "No fainted Pokemon!"); wait_for_space(11); continue;
                }
                refresh();
                int pick = getch() - '1';
                if (pick >= 0 && pick < (int)fainted_idx.size()) {
                    int idx = fainted_idx[pick];
                    attackingt.pokemons[idx].stats[0] = attackingt.pokemons[idx].max_hp / 2;
                    attackingt.inventory[1]--;
                    mvprintw(10, 0, "Revived %s!",
                        attackingt.pokemons[idx].pokemon->get_identifier().c_str());
                    wait_for_space(11);
                    pc_switched = true; continue;
                } else {
                    mvprintw(10, 0, "Invalid choice."); wait_for_space(11); continue;
                }

            } else if (item_key == '3' && defendingt.is_wild) { // Pokeball
                if (attackingt.inventory[2] <= 0) {
                    mvprintw(10, 0, "No Pokeballs left!"); wait_for_space(11); continue;
                }
                attackingt.inventory[2]--;
                if ((int)attackingt.pokemons.size() < 6) {
                    attackingt.pokemons.push_back(defendingt.pokemons[defendingt.pokemon]);
                    mvprintw(10, 0, "Gotcha! %s was caught!",
                        defendingt.pokemons[defendingt.pokemon].pokemon->get_identifier().c_str());
                    wait_for_space(11);
                    battle_over = true;
                } else {
                    mvprintw(10, 0, "Party full! %s broke free and fled!",
                        defendingt.pokemons[defendingt.pokemon].pokemon->get_identifier().c_str());
                    wait_for_space(11);
                    battle_over = true;
                }
            } else {
                continue;
            }
            break;
        }

        case '3': { // Run
            if (!defendingt.is_wild) {
                mvprintw(10, 0, "Can't run from a trainer battle!");
                wait_for_space(11);
                continue;
            }
            attackingt.escape_attempts++;
            int spd_trainer = pc_pokemon.stats[5];
            int spd_wild    = npc_pokemon.stats[5];
            int denom = max((spd_wild / 4) % 256, 1);
            int odds  = min((spd_trainer * 32 / denom) + 30 * attackingt.escape_attempts, 255);
            if (rand() % 256 < odds) {
                mvprintw(10, 0, "Got away safely!");
                wait_for_space(11);
                battle_over = true;
            } else {
                mvprintw(10, 0, "Couldn't escape!");
                wait_for_space(11);
                pc_ran = true;
            }
            break;
        }

        case '4':
            pc_switched = switch_pokemon(attackingt);
            continue;

        default:
            continue;
        }

        if (battle_over) break;

        Moves *npc_move = get_move(*npc_pokemon.moves[rand() % npc_pokemon.moves.size()]);
        if (pc_switched || pc_ran) {
            if (npc_pokemon.stats[0] > 0)
            do_move(defendingt, attackingt, *npc_move, battle_over);
            continue;
        }

        bool pc_goes_first;
        if (pc_move->get_priority() != npc_move->get_priority()) {
            pc_goes_first = pc_move->get_priority() > npc_move->get_priority();
        } else {
            int pc_speed  = pc_pokemon.stats[5];
            int npc_speed = npc_pokemon.stats[5];
            if (pc_speed != npc_speed) {
                pc_goes_first = pc_speed > npc_speed;
            } else {
                pc_goes_first = rand() % 2;
            }
        }

        if (pc_goes_first) {
            do_move(attackingt, defendingt, *pc_move, battle_over);
            if (!battle_over && npc_pokemon.stats[0] > 0 && pc_pokemon.stats[0] > 0)
                do_move(defendingt, attackingt, *npc_move, battle_over);
        } else {
            do_move(defendingt, attackingt, *npc_move, battle_over);
            if (!battle_over && pc_pokemon.stats[0] > 0 && npc_pokemon.stats[0] > 0)
                do_move(attackingt, defendingt, *pc_move, battle_over);
        }
    }
    
    {
        bool pc_alive = false;
        for (auto &p : attackingt.pokemons)
            if (p.stats[0] > 0) { pc_alive = true; break; }
        bool npc_alive = false;
        for (auto &p : defendingt.pokemons)
            if (p.stats[0] > 0) { npc_alive = true; break; }

        clear();
        if (!pc_alive) {
            mvprintw(0, 0, "You blacked out! All your Pokemon have fainted.");
        } else if (!npc_alive && defendingt.is_wild) {
            mvprintw(0, 0, "The wild %s was defeated!",
                defendingt.pokemons[defendingt.pokemon].pokemon->get_identifier().c_str());
        } else if (!npc_alive) {
            mvprintw(0, 0, "You won the battle! The trainer has been defeated.");
            defendingt.defeated = 1;
        }
        // caught / fled messages already shown inline; skip if battle_over from those
        wait_for_space(2);
    }
}

void move_pc_long(char_t *pc, world_t *world, int dr, int dc) {
    int old_map_r = world->map_r;
    int old_map_c = world->map_c;
    int old_r = world->pc_r;
    int old_c = world->pc_c;
    int new_r = old_r + dr;
    int new_c = old_c + dc;

    // Check jump first, before can_move
    if (new_r <= 0) {
        new_r = 20;
        world->map_r -= 1;
        world->jumped = 1;
    } else if (new_r >= 20) {
        new_r = 0;
        world->map_r += 1;
        world->jumped = 1;
    }
    if (new_c <= 0) {
        new_c = 79;
        world->map_c -= 1;
        world->jumped = 1;
    } else if (new_c >= 79) {
        new_c = 0;
        world->map_c += 1;
        world->jumped = 1;
    }

    // Generate new map if needed
    if (world->jumped && world->maps[world->map_r][world->map_c] == NULL) {
        world->maps[world->map_r][world->map_c] = new map_t();
        if (!world->maps[world->map_r][world->map_c]) {
            world->map_r = old_map_r;
            world->map_c = old_map_c;
            world->jumped = 0;
            return;
        }
        generate_map(world->maps, world->map_r, world->map_c);
        place_npcs(world, world->numtrainers);
    }

    map_t *new_map = world->maps[world->map_r][world->map_c];
    map_t *old_map = world->maps[old_map_r][old_map_c];

    if (can_move(new_map, pc, new_r, new_c)) {
        old_map->occupancy[old_r][old_c] = NULL;
        pc->r = new_r;
        pc->c = new_c;
        new_map->occupancy[new_r][new_c] = pc;
        world->pc_r = new_r;
        world->pc_c = new_c;
        update_distance_maps_after_pc_move(world);
        strcpy(world->message, "");

        //if new location is tall grass — trigger wild battle
        if (new_map->tiles[new_r][new_c] == ':') {
            Spawned_pokemon p;
            if (spawn_pokemon(world, p)) {
                char_t wild_trainer;
                wild_trainer.type = 'w';
                wild_trainer.defeated = 0;
                wild_trainer.is_wild = 1;
                wild_trainer.escape_attempts = 0;
                wild_trainer.pokemon = 0;
                wild_trainer.pokemons.push_back(p);
                char_t *pc_char = new_map->occupancy[world->pc_r][world->pc_c];
                pokemon_battle(world, *pc_char, wild_trainer);
            }
        }
    } else {
        // undo jump if move failed
        world->map_r = old_map_r;
        world->map_c = old_map_c;
        world->jumped = 0;

        if (old_map->occupancy[new_r][new_c] != NULL) {
            if (old_map->occupancy[new_r][new_c]->type != '@') {
                char_t *npc = old_map->occupancy[new_r][new_c];
                char_t *pc = old_map->occupancy[world->pc_r][world->pc_c];
                if (!npc->defeated) {
                    pokemon_battle(world, *pc, *npc);
                } else {
                    strcpy(world->message, "This trainer has already been defeated.");
                }
            } else {
                strcpy(world->message, "");
            }
        } else {
            char message[81] = "";
            switch (old_map->tiles[new_r][new_c]) {
                case '%': strcpy(message, "Don't go there. That's the edge of the world!!!"); break;
                case '^': strcpy(message, "That's a very high tree. I don't think you should climb it!"); break;
                case '&': strcpy(message, "You can climb that mountain, but you wouldn't want to!!!"); break;
                case '~': strcpy(message, "Don't go there. You might drown!!!"); break;
                case 'g': strcpy(message, "There's no exit there (yet)!"); break;
                default:  strcpy(message, "You can't move there."); break;
            }
            strcpy(world->message, message);
        } 
    }

    // Update PC cost regardless of success/failure
    int cost;
    map_t *cur_map = world->maps[world->map_r][world->map_c];
    switch (cur_map->tiles[pc->r][pc->c]) {
        case '%': cost = pc->costs[0]; break;
        case '^': cost = pc->costs[1]; break;
        case '#': cost = pc->costs[2]; break;
        case 'M': cost = pc->costs[3]; break;
        case 'C': cost = pc->costs[4]; break;
        case ':': cost = pc->costs[5]; break;
        case '.': cost = pc->costs[6]; break;
        case '&': cost = pc->costs[7]; break;
        case '~': cost = pc->costs[8]; break;
        case 'g': cost = pc->costs[9]; break;
        default:  cost = INT_MAX;      break;
    }
    if (cost == INT_MAX) {
        cost = 10;
    }
    pc->next = pc->next + cost;
}

void move_pc(world_t *world, int dr, int dc) {
    //get pc
    map_t *map = world->maps[world->map_r][world->map_c];
    char_t *pc = map->occupancy[world->pc_r][world->pc_c];
    move_pc_long(pc, world, dr, dc);
}

void move_npc(char_t *ch, world_t *world) { 
    map_t *map = world->maps[world->map_r][world->map_c];
    int current_time = ch->next;
    switch (ch->type) {
        case 'h':
        case 'r': {
             if (!ch->defeated) {
                move_gradient(map, ch, current_time);
            } else {
                move_others(world, map, ch, current_time);
            }
            break;           
        }
        case 'p':
        case 'w':
        case 's':
        case 'e':
            move_others(world, map, ch, current_time);
            break;
        default:
            break;
    }
}

void print_character_list(world_t *world, int offset) {
    char_t *npcs[50];
    int count = 0;

    for (int r = 0; r < 21; r++) {
        for (int c = 0; c < 80; c++) {
            if (world->maps[world->map_r][world->map_c]->occupancy[r][c] != NULL) {
                if (world->maps[world->map_r][world->map_c]->occupancy[r][c]->type != '@') {
                    npcs[count] = world->maps[world->map_r][world->map_c]->occupancy[r][c];
                    count++;
                }
            }
        }
    }
    
    int visible = 21;

    clear();
    mvprintw(0, 0, "NPCs list (arrow keys to scroll, esc to exit):");

    for (int i = 0; i < visible && (i + offset) < count; i++) {
        char_t *npc = npcs[i + offset];

        const char * vdir = "";
        int vertical = npc->r - world->pc_r;
        if (vertical <= 0) {
            vdir = "North";
        } else if (vertical > 0) {
            vdir = "South";
        }
        const char * hdir = "";
        int horizontal = npc->c - world->pc_c;
        if (horizontal <= 0) {
            hdir = "West";
        } else if (horizontal > 0) {
            hdir = "East";
        }
        mvprintw(i + 1, 0, "%2d. %c, %2d %s, %2d %s", i + 1 + offset, npc->type, abs(vertical), vdir, abs(horizontal), hdir);
    }
    refresh();
} 

void open_bag(world_t *world) {
    char_t *pc = world->maps[world->map_r][world->map_c]->occupancy[world->pc_r][world->pc_c];
    bool done = false;
    while (!done) {
        clear();
        mvprintw(0, 0, "=== Bag ===");
        mvprintw(2, 0, "1. Potion   (x%d) - restore 20 HP to a Pokemon", pc->inventory[0]);
        mvprintw(3, 0, "2. Revive   (x%d) - revive fainted Pokemon to half HP", pc->inventory[1]);
        mvprintw(5, 0, "0 / ESC - Close bag");
        refresh();

        int key = getch();
        if (key == '0' || key == 27) { done = true; break; }

        if (key == '1') { // Potion
            if (pc->inventory[0] <= 0) {
                mvprintw(10, 0, "No potions left!");
                wait_for_space(11); continue;
            }
            clear();
            mvprintw(0, 0, "Use potion on which Pokemon?");
            vector<int> alive_idx;
            for (int i = 0; i < (int)pc->pokemons.size(); i++) {
                if (pc->pokemons[i].stats[0] > 0 && pc->pokemons[i].stats[0] < pc->pokemons[i].max_hp) {
                    mvprintw((int)alive_idx.size() + 2, 0, "%d. %s  HP: %d/%d",
                        (int)alive_idx.size() + 1,
                        pc->pokemons[i].pokemon->get_identifier().c_str(),
                        pc->pokemons[i].stats[0], pc->pokemons[i].max_hp);
                    alive_idx.push_back(i);
                }
            }
            if (alive_idx.empty()) {
                mvprintw(10, 0, "All Pokemon are already at full HP!");
                wait_for_space(11); continue;
            }
            mvprintw((int)alive_idx.size() + 3, 0, "0 / ESC - Cancel");
            refresh();
            int pick = getch();
            if (pick == '0' || pick == 27) continue;
            pick = pick - '1';
            if (pick >= 0 && pick < (int)alive_idx.size()) {
                int idx = alive_idx[pick];
                pc->pokemons[idx].stats[0] = min(pc->pokemons[idx].stats[0] + 20, pc->pokemons[idx].max_hp);
                pc->inventory[0]--;
                mvprintw(10, 0, "Used potion on %s! HP restored.",
                    pc->pokemons[idx].pokemon->get_identifier().c_str());
                wait_for_space(11);
            } else {
                mvprintw(10, 0, "Invalid choice."); wait_for_space(11);
            }

        } else if (key == '2') { // Revive
            if (pc->inventory[1] <= 0) {
                mvprintw(10, 0, "No revives left!");
                wait_for_space(11); continue;
            }
            clear();
            mvprintw(0, 0, "Revive which Pokemon?");
            vector<int> fainted_idx;
            for (int i = 0; i < (int)pc->pokemons.size(); i++) {
                if (pc->pokemons[i].stats[0] <= 0) {
                    mvprintw((int)fainted_idx.size() + 2, 0, "%d. %s (fainted)",
                        (int)fainted_idx.size() + 1,
                        pc->pokemons[i].pokemon->get_identifier().c_str());
                    fainted_idx.push_back(i);
                }
            }
            if (fainted_idx.empty()) {
                mvprintw(10, 0, "No fainted Pokemon!"); wait_for_space(11); continue;
            }
            mvprintw((int)fainted_idx.size() + 3, 0, "0 / ESC - Cancel");
            refresh();
            int pick = getch();
            if (pick == '0' || pick == 27) continue;
            pick = pick - '1';
            if (pick >= 0 && pick < (int)fainted_idx.size()) {
                int idx = fainted_idx[pick];
                pc->pokemons[idx].stats[0] = pc->pokemons[idx].max_hp / 2;
                pc->inventory[1]--;
                mvprintw(10, 0, "Revived %s to half HP!",
                    pc->pokemons[idx].pokemon->get_identifier().c_str());
                wait_for_space(11);
            } else {
                mvprintw(10, 0, "Invalid choice."); wait_for_space(11);
            }
        }
    }
}

void enter_building(world_t *world) {
    char t = world->maps[world->map_r][world->map_c]->tiles[world->pc_r][world->pc_c];
    char_t *pc = world->maps[world->map_r][world->map_c]->occupancy[world->pc_r][world->pc_c];
    switch (t) {
        case 'M':
            pc->inventory[0] = default_potions;
            pc->inventory[1] = default_revives;
            pc->inventory[2] = default_pokeballs;
            strcpy(world->message, "All items in bags refilled!");
            break;
        case 'C':
            for (auto &pokemon : pc->pokemons) {
                pokemon.stats[0] = pokemon.max_hp;
            }
            strcpy(world->message, "All Pokemons revived!");
            break;
        default:
            break;
    }
}
