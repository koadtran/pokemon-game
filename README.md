# poke327

A terminal-based Pokémon-inspired roguelike, built incrementally over the semester for **COM S 327 (Advanced Programming Techniques)** at Iowa State University, Spring 2026.

The game generates a 401×401 world of procedurally-generated terrain maps, populates them with trainers driven by Dijkstra-based pathfinding, and lets the player wander, battle, and catch Pokémon loaded from the official Pokédex CSV database.

## Features (by assignment)

| # | Milestone | Notes |
|---|-----------|-------|
| 1.02 | World of maps | 401×401 grid, gates align between neighboring maps, buildings thin out with distance from origin |
| 1.03 | Pathfinding | Dijkstra's algorithm computes travel-cost maps for hikers and rivals |
| 1.04 | Trainers | Hikers, Rivals, Pacers, Wanderers, Sentries, Explorers; priority-queue turn scheduler |
| 1.05 | ncurses UI | Unbuffered input, vi/numpad movement, trainer list with scrolling |
| 1.06 | Multi-map travel + C++ port | Per-map turn queues, `fly` command, ported from C to C++ with `character` → `pc`/`npc` inheritance |
| 1.07 | CSV parsing | Loads the Pokédex database (pokemon, moves, species, stats, types, experience, etc.) |
| 1.08 | Pokémon spawning | Wild encounters in tall grass, level scales with Manhattan distance, IV generation, level-up movesets |
| 1.09 | Battles | Turn-based battles vs. wild Pokémon and trainers, priority/speed ordering, MSG damage formula |

## Build

```bash
make
./poke327
```

Requires a C++ compiler and `ncurses` (`libncurses-dev` on Debian/Ubuntu, preinstalled on the ISU Pyrite server).

## Running

The game looks for the Pokédex CSV database in this order:

1. `/share/cs327/pokedex/pokedex/data/csv/` (Pyrite / grading environment)
2. `$HOME/.poke327/pokedex/data/csv/`
3. *(optional third location — see source)*

If none contain the database, the program exits with an error.

**The database is not included in this repository** — see the "Database" section below.

### Controls

| Key | Action |
|-----|--------|
| `7 y` `8 k` `9 u` `6 l` `3 n` `2 j` `1 b` `4 h` | Move one cell (8 directions) |
| `5` `space` `.` | Rest one turn |
| `>` | Enter Pokémart / Pokémon Center (when standing on one) |
| `<` | Exit building |
| `t` | List trainers (arrow keys scroll, `esc` returns) |
| `f x y` | Fly to map coordinates `(x, y)` |
| `Q` | Quit |

## Database

The Pokédex CSV data lives on the CS department's Pyrite server at `/share/cs327/pokedex/`. To fetch a local copy:

```bash
mkdir -p ~/.poke327
scp -r <netid>@pyrite.cs.iastate.edu:/share/cs327/pokedex ~/.poke327/
```

Per course policy, the database is **not** committed to this repository.

## Project layout
