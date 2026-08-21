#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <climits>

#include "data.h"

using namespace std;

int my_stoi(string str) {
    return (str == "") ? INT_MAX : stoi(str);
}

string my_itos(int n) {
    return (n == INT_MAX) ? "" : to_string(n);
}


Experience::Experience(vector<string> fields) {
    growth_rate_id = my_stoi(fields[0]);
    level = my_stoi(fields[1]);
    experience = my_stoi(fields[2]);
}


Moves::Moves(vector<string> fields) {
    id = my_stoi(fields[0]);
    identifier = fields[1];
    generation_id = my_stoi(fields[2]);
    type_id = my_stoi(fields[3]);
    power = my_stoi(fields[4]);
    pp = my_stoi(fields[5]);
    accuracy = my_stoi(fields[6]);
    priority = my_stoi(fields[7]);
    target_id = my_stoi(fields[8]);
    damage_class_id = my_stoi(fields[9]);
    effect_id = my_stoi(fields[10]);
    effect_chance = my_stoi(fields[11]);
    contest_type_id = my_stoi(fields[12]);
    contest_effect_id = my_stoi(fields[13]);
    super_contest_effect_id = my_stoi(fields[14]);
}

Pokemon_moves::Pokemon_moves(vector<string> fields) {
    pokemon_id = my_stoi(fields[0]);
    version_group_id = my_stoi(fields[1]);
    move_id = my_stoi(fields[2]);
    pokemon_move_method_id = my_stoi(fields[3]);
    level = my_stoi(fields[4]);
    order = my_stoi(fields[5]);
}

Pokemon_species::Pokemon_species(vector<string> fields) {
    id = my_stoi(fields[0]);
    identifier = fields[1];
    generation_id = my_stoi(fields[2]);
    evolves_from_species_id = my_stoi(fields[3]);
    evolution_chain_id = my_stoi(fields[4]);
    color_id = my_stoi(fields[5]);
    shape_id = my_stoi(fields[6]);
    habitat_id = my_stoi(fields[7]);
    gender_rate = my_stoi(fields[8]);
    capture_rate = my_stoi(fields[9]);
    base_happiness = my_stoi(fields[10]);
    is_baby = fields[11] == "1";
    hatch_counter = my_stoi(fields[12]);
    has_gender_differences = fields[13] == "1";
    growth_rate_id = my_stoi(fields[14]);
    forms_switchable = fields[15] == "1";
    is_legendary = fields[16] == "1";
    is_mythical = fields[17] == "1";
    order = my_stoi(fields[18]);
    conquest_order = my_stoi(fields[19]);
}

Pokemon_stats::Pokemon_stats(vector<string> fields) {
    pokemon_id = my_stoi(fields[0]);
    stat_id = my_stoi(fields[1]);
    base_stat = my_stoi(fields[2]);
    effort = my_stoi(fields[3]);
}
 
Pokemon_types::Pokemon_types(vector<string> fields) {
    pokemon_id = my_stoi(fields[0]);
    type_id = my_stoi(fields[1]);
    slot = my_stoi(fields[2]);
}

Pokemon::Pokemon(vector<string> fields) {
    id = my_stoi(fields[0]);
    identifier = fields[1];
    species_id = my_stoi(fields[2]);
    height = my_stoi(fields[3]);
    weight = my_stoi(fields[4]);
    base_experience = my_stoi(fields[5]);
    order = my_stoi(fields[6]);
    is_default = fields[7] == "1";
}

Stats::Stats(vector<string> fields) {
    id = my_stoi(fields[0]);
    damage_class_id = my_stoi(fields[1]);
    identifier = fields[2];
    is_battle_only = fields[3] == "1";
    game_index = my_stoi(fields[4]);
}

Type_names::Type_names(vector<string> fields) {
    type_id = my_stoi(fields[0]);
    local_language_id = my_stoi(fields[1]);
    name = fields[2];
}

vector<string> split(string str) {
    vector<string> arr;
    string acc = "";
    for (int i = 0; i < (int) str.length(); i++) {
        char ch = str.at(i);
        if (ch != ',') {
            acc.push_back(ch);
        }
        else {
            arr.push_back(acc);
            acc = "";
        }
    }
    arr.push_back(acc);
    return arr;
}

Spawned_pokemon::Spawned_pokemon(int distance){
    this->pokemon = &pokemon_arr[rand() % pokemon_arr.size()];
    int min_level = 1;
    int max_level = distance / 2;

    if (distance > 200) {
        min_level = (distance - 200) / 2;
        max_level = 100;
    }

    if (max_level < 1) {
        max_level = 1;
    }
    int level = min_level + rand() % (max_level - min_level + 1);

    vector<Pokemon_moves *> available_moves;
    do {
        available_moves.clear();
        for (auto &move : pokemon_moves_arr) {
            if (move.get_pokemon_id() == pokemon->get_species_id() && move.get_method_id() == 1 && move.get_level() <= level) {
                available_moves.push_back(&move);
            }
        }
        if (available_moves.empty()) {
            level++;
        }
    } while (available_moves.empty());
    int i = rand() % available_moves.size();
    this->moves.push_back(available_moves[i]);
    available_moves.erase(available_moves.begin() + i);
    if (!available_moves.empty()) {
        this->moves.push_back(available_moves[rand() % available_moves.size()]);
    }
    this->level = level;

    int start_index = -1; 
    for (int i = 0; i < (int) pokemon_stats_arr.size(); i++) {
        if (pokemon->get_id() == pokemon_stats_arr[i].get_pokemon_id()) {
            start_index = i;
            break;
        }
    }

    for (int i = 0; i < 6; i++) {
        int iv = rand() % 16;
        if (i == 0) {
            int hp = ((pokemon_stats_arr[start_index + i].get_base_stat() + iv) * 2) * level / 100 + level + 10;
            this->stats[i] = hp;
            max_hp = hp;
        } else {
            this->stats[i] = ((pokemon_stats_arr[start_index + i].get_base_stat() + iv) * 2) * level / 100 + 5;
        }
    }

    this->is_male = rand() % 2;
    this->is_shiny = rand() % 8192 == 0;
}

string Spawned_pokemon::get_move_name(int move_id) {
    for (auto move : moves_arr) {
        if (move.get_id() == move_id) {
            return move.get_identifier();
        }
    }
    return "";
}

string Spawned_pokemon::to_string() {
    string s = pokemon->get_identifier() + "\n";
    s += "lv." + my_itos(level) + (is_shiny ? " shiny" : "") + (is_male ? " M" : " F") + "\n";
    s += "hp:" + my_itos(stats[0]) + " atk:" + my_itos(stats[1]) + " def:" + my_itos(stats[2]) + " spatk:" + my_itos(stats[3]) + " spdef:" + my_itos(stats[4]) + " spd:" + my_itos(stats[5]) + "\n";
    for (auto &move : moves) s += get_move_name(move->get_move_id()) + " ";
    return s;
}

string Spawned_pokemon::to_string_one_line() {
    string s = pokemon->get_identifier() + " lv." + my_itos(level) + (is_shiny ? " shiny" : "") + (is_male ? " M" : " F");
    s += " [" + my_itos(stats[0]) + "," + my_itos(stats[1]) + "," + my_itos(stats[2]) + "," + my_itos(stats[3]) + "," + my_itos(stats[4]) + "," + my_itos(stats[5]) + "]";
    for (auto &move : moves) s += " " + get_move_name(move->get_move_id());
    return s;
}

vector<Experience> experience_arr;
vector<Moves> moves_arr;
vector<Pokemon_moves> pokemon_moves_arr;
vector<Pokemon_species> pokemon_species_arr;
vector<Pokemon_stats> pokemon_stats_arr;
vector<Pokemon_types> pokemon_types_arr;
vector<Pokemon> pokemon_arr;
vector<Stats> stats_arr;
vector<Type_names> type_names_arr;

string find_path(string file_name) {
    string paths[3];
    paths[0] = "./files_to_parse/";

    char *home = getenv("HOME");
    if (home) {
        paths[1] = string(home) + "/.poke327/pokedex/pokedex/data/csv/";
    } else {
        paths[1] = "";
    }

    paths[2] = "/Users/dangboi/Desktop/CS327/files_to_parse/";

    for (int i = 0; i < 3; i++) {
        if (paths[i].empty()) continue;

        string full_path = paths[i] + file_name + ".csv";
        ifstream file(full_path);

        if (file.good()) {
            file.close();
            return full_path;
        }
    }

    cerr << "Error: File not found: " << file_name << ".csv" << endl;
    exit(1);
}

void read_and_fill(string file_name) {
    ifstream file(find_path(file_name));
    
    string line;
    getline(file, line);
    while (getline(file, line)) {
        vector<string> fields = split(line);
        if (file_name == "experience") {
            experience_arr.push_back(Experience(fields));
        }
        else if (file_name == "moves") {
            moves_arr.push_back(Moves(fields));
        }
        else if (file_name == "pokemon_moves") {
            pokemon_moves_arr.push_back(Pokemon_moves(fields));
        }
        else if (file_name == "pokemon_species") {
            pokemon_species_arr.push_back(Pokemon_species(fields));
        }
        else if (file_name == "pokemon_stats") {
            pokemon_stats_arr.push_back(Pokemon_stats(fields));
        }
        else if (file_name == "pokemon_types") {
            pokemon_types_arr.push_back(Pokemon_types(fields));
        }
        else if (file_name == "pokemon") {
            pokemon_arr.push_back(Pokemon(fields));
        }
        else if (file_name == "stats") {
            stats_arr.push_back(Stats(fields));
        }
        else if (file_name == "type_names") {
            type_names_arr.push_back(Type_names(fields));
        }
    }
}

void read_files() {
    read_and_fill("experience");
    read_and_fill("moves");
    read_and_fill("pokemon_moves");
    read_and_fill("pokemon_species");
    read_and_fill("pokemon_stats");
    read_and_fill("pokemon_types");
    read_and_fill("pokemon");
    read_and_fill("stats");
    read_and_fill("type_names");
}
