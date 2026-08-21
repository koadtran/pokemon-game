# pokemon-game

A terminal Pokémon-inspired roguelike written in C++ with ncurses. Wander a huge procedurally-generated world, dodge (or fight) rival trainers driven by Dijkstra pathfinding, and run into wild Pokémon spawned from the real Pokédex database.

## What it does

- **Infinite-feeling world.** A 401×401 grid of procedurally-generated terrain maps — grasslands, forests, mountains, water, roads — with gates that align between neighbors so the world stitches together as you explore. Pokémon Centers and Pokémarts thin out the farther you get from the origin.
- **Smart adversaries.** Six trainer archetypes (Hikers, Rivals, Pacers, Wanderers, Sentries, Explorers), each with their own movement AI. Hikers and Rivals pathfind toward the player using Dijkstra over terrain-specific movement costs.
- **Priority-queue turn scheduler.** A min-heap keyed on next-move time drives every character; faster movers act more often and time only ticks when something actually happens.
- **Real Pokémon data.** Loads the full Pokédex CSV database at startup — 1,092 species, their moves, base stats, types, and experience curves.
- **Wild encounters.** Step into tall grass and a Pokémon spawns with level scaled to your Manhattan distance from the origin, IVs rolled per stat, and a moveset drawn from its actual level-up learnset.

## Tech

- **Language:** C++ (started in C, ported once the design settled)
- **UI:** ncurses (unbuffered input, colored terminal rendering)
- **Data structures:** custom min-heap priority queue, per-map turn queues, 2D terrain grids
- **Data:** Pokédex CSVs parsed on startup

## Build & run

```bash
cd src
make
./game            # default: 10 trainers
./game 20         # optional: number of trainers (capped at 30)
```

Requires a C++ compiler and `libncurses-dev` (or equivalent on your platform).

The Pokédex CSVs ship with the repo under `src/files_to_parse/`, so no extra setup is needed.

## Controls

| Key | Action |
|-----|--------|
| `7 y` `8 k` `9 u` `6 l` `3 n` `2 j` `1 b` `4 h` | Move one cell (8 directions — vi keys or numpad) |
| `5` `space` `.` | Rest one turn |
| `>` | Enter Pokémart or Pokémon Center |
| `f` | Fly to map coordinates |
| `t` | List nearby trainers (scroll with arrows, `esc` closes) |
| `B` | Open bag |
| `q` | Quit |

## Project layout

src/
├── game.cpp            # main loop, ncurses setup
├── world.cpp           # world state, input handling
├── world.h
├── map.cpp             # terrain generation
├── map.h
├── characters.cpp      # PC, NPCs, movement AI, encounters
├── characters.h
├── distance.cpp        # Dijkstra pathfinding
├── distance.h
├── data.cpp            # Pokédex CSV parsing
├── data.h
├── files_to_parse/     # Pokédex CSV database
└── Makefile


## Credits

The Pokédex CSV data under `src/files_to_parse/` comes from the [veekun/pokedex](https://github.com/veekun/pokedex) project, an open-source community Pokédex. Used here under its MIT license.

## Author

Khoa Dang Tran
