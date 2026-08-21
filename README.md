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

### Pokédex data

The game looks for the Pokédex CSVs in a few standard locations at startup. If you need a local copy, grab it from the [veekun/pokedex](https://github.com/veekun/pokedex) repo:

```bash
mkdir -p ~/.poke327
git clone https://github.com/veekun/pokedex ~/.poke327/pokedex
```

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
