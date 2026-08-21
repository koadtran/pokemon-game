#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int my_stoi(string str);
string my_itos(int n);
vector<string> split(string str);
string find_path(string file_name);
void read_and_fill(string file_name);
void read_files();
string get_move_name(int move_id) ;

class Experience {
private:
    int growth_rate_id;
    int level;
    int experience;

public:
    Experience(vector<string> fields);
};

class Moves {
private:
    int id;
    string identifier;
    int generation_id;
    int type_id;
    int power;
    int pp;
    int accuracy;
    int priority;
    int target_id;
    int damage_class_id;
    int effect_id;
    int effect_chance;
    int contest_type_id;
    int contest_effect_id;
    int super_contest_effect_id;

public:
    Moves(vector<string> fields);
    int get_id() { return id;};
    string get_identifier() {return identifier;};
    int get_power() {return power;};
    int get_accuracy() {return accuracy;};
    int get_priority() {return priority;};
};

class Pokemon_moves {
private:
    int pokemon_id;
    int version_group_id;
    int move_id;
    int pokemon_move_method_id;
    int level;
    int order;

public:
    Pokemon_moves(vector<string> fields);

    int get_pokemon_id() {return pokemon_id;}
    int get_move_id() {return move_id;}
    int get_method_id() {return pokemon_move_method_id;}
    int get_level() {return level;}
};

class Pokemon_species {
private:
    int id;
    string identifier;
    int generation_id;
    int evolves_from_species_id;
    int evolution_chain_id;
    int color_id;
    int shape_id;
    int habitat_id;
    int gender_rate;
    int capture_rate;
    int base_happiness;
    bool is_baby;
    int hatch_counter;
    bool has_gender_differences;
    int growth_rate_id;
    bool forms_switchable;
    bool is_legendary;
    bool is_mythical;
    int order;
    int conquest_order;

public:
    Pokemon_species(vector<string> fields);
};

class Pokemon_stats {
private:
    int pokemon_id;
    int stat_id;
    int base_stat;
    int effort;

public:
    Pokemon_stats(vector<string> fields);

    int get_pokemon_id() {return pokemon_id;};
    int get_base_stat() {return base_stat;}
};

class Pokemon_types {
private:
    int pokemon_id;
    int type_id;
    int slot;

public:
    Pokemon_types(vector<string> fields);

    int get_pokemon_id() {return pokemon_id;};
    int get_type_id() {return type_id;};
};

class Pokemon {
private:
    int id;
    string identifier;
    int species_id;
    int height;
    int weight;
    int base_experience;
    int order;
    bool is_default;

public:
    Pokemon(vector<string> fields);

    int get_id() {return id;};
    string get_identifier() {return identifier;};
    int get_species_id() {return species_id;};
};

class Stats {
private:
    int id;
    int damage_class_id;
    string identifier;
    bool is_battle_only;
    int game_index;

public:
    Stats(vector<string> fields);
};

class Type_names {
private:
    int type_id;
    int local_language_id;
    string name;

public:
    Type_names(vector<string> fields);
};

class Spawned_pokemon {
public:
    Pokemon * pokemon;
    int level;
    int max_hp;
    int stats[6];
    bool is_shiny;
    bool is_male;
    vector<Pokemon_moves *> moves;

    Spawned_pokemon() {};
    Spawned_pokemon(int distance);
    string get_move_name(int move_id);
    string to_string();
    string to_string_one_line();
};

extern vector<Experience> experience_arr;
extern vector<Moves> moves_arr;
extern vector<Pokemon_moves> pokemon_moves_arr;
extern vector<Pokemon_species> pokemon_species_arr;
extern vector<Pokemon_stats> pokemon_stats_arr;
extern vector<Pokemon_types> pokemon_types_arr;
extern vector<Pokemon> pokemon_arr;
extern vector<Stats> stats_arr;
extern vector<Type_names> type_names_arr;

#endif