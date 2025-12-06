# JKidding's TicTacToe Engine

A small, fast (enough) C implementation of a Tic-Tac-Toe (3x3) engine that includes:

- An offline lookup_table generator capable of enumerating and computing best moves for every legal 3x3 board state (in `src/Generator/generate_lookup_table.c`).
- Embedding of the generated "lookup_table" binary into the main executable using `INCBIN` (`src/INCBIN/incbin.h`) to provide instant lookups at runtime.
- A small command-line game (`src/main.c`) that lets you play against the lookup_table or run benchmarks.

Why this project exists
-----------------------
This project exists because of me trying to learn bit packing for the first time.

Quick features
--------------
- Generate a lookup_table and save it to `src/lookup_table/lookup_table.bin`.
- Embed the binary lookup_table into `main` using `incbin`.
- Play modes: human vs AI, self-play (AI vs AI), and benchmarking.
- Small, readable code: `ttt.h` holds the board logic; `lookup_table.h` handles decoding/loading the lookup_table.

Build
-----
Build both the `main` program and the `generator` using the top-level `Makefile`:

```bash
make
```

This should produce two executables:

- `main` — the Tic-Tac-Toe runtime engine that loads the embedded lookup_table and runs the requested mode.
- `generator` — the special program that generates `src/lookup_table/lookup_table.bin` (used if you want to regenerate the lookup_table).

If `src/lookup_table/lookup_table.bin` is missing or you want to regenerate it, run:

```bash
./generator
```

Then rebuild `main` if necessary.

Usage
-----
From the repository root (after building):

```bash
./main --help
```

Common commands:

- `./main --benchmark [--seconds N]` — Run a CPU benchmark that plays full game simulations for N seconds (default 1.0). Shows games/sec and average moves per game.
- `./main --selfplay` — Let the lookup_table play both sides and print moves until the game completes.
- `./main --pva [X|O] [--bot-thinks] [--thinking-time N]` — Play against the AI (pick human side X or O). `--bot-thinks` simulates a delay; `--thinking-time N` sets time per move.
- `./main --credits` — Show credits and info.

Controls and board layout
-------------------------
- The board is 3x3 with positions numbered `0` to `8` in left-to-right, top-to-bottom order:

```
0 | 1 | 2
--+---+--
3 | 4 | 5
--+---+--
6 | 7 | 8
```

- In `PVA` (player vs AI), the CLI prompts for moves and expects a number in 0–8. Invalid input or illegal moves will be rejected.

Implementation details
----------------------
- The game state is stored using bitboards in `ttt.h`:
  - `player[0]` (X) and `player[1]` (O) are 9-bit bitboards; `board` is a 9-bit occupancy mask.
  - `encode_board()` converts the 3^9 board (~19683 unique states) to an integer index used by the lookup_table.
- The lookup_table generator (`src/Generator/generate_lookup_table.c`) does the following for each legal state:
  - Verifies state legality (counts, mutual exclusive squares, winning constraints).
  - Uses a negamax search per position to find the best move and encodes the result.
  - Stores 20-bit packed entries in a simple binary file format (`lookup_table.bin`).
- Encoding format used by the generator (20-bit result stored in a 32-bit int):
  - 15 bits: encoded board index (0–19682)
  - 1 bit: current player to move (0/1)
  - 4 bits: best move index (0–8) or `0xF` to indicate invalid/none

- At runtime, `init_lookup_table()` in `src/lookup_table.h` reads `src/lookup_table/lookup_table.bin` (embedded using `INCBIN`) and decodes each 32-bit entry into a `TTTDecoded` structure:
  - `uint16_t board_encoded` — 15-bit board index
  - `bool current_player` — who moves
  - `uint8_t best_index` — best move (0–8) or 0xF

Notes
-----
- The Makefile attempts to enable aggressive CPU flags (e.g., `-march=native` and `-mavx2`) on Linux for speed; adjust CFLAGS or the Makefile if you need portability.
- `INCBIN` is used to embed the binary data into `main`; you can change this if you prefer to load the file at runtime.
- The reason why I choose the name "lookup_table" is because I have no other name that pops up in my mind.

Project structure
-----------------
```
README.md
Makefile
src/
  main.c            # CLI and modes (pva, selfplay, benchmark)
  ttt.h             # core Tic-Tac-Toe bitboard logic & IO helpers
  lookup_table.h    # lookup_table decode & initialization, INCBIN integration
  INCBIN/           # incbin helper for embedding binary data
  Generator/        # generator for `lookup_table.bin`
  LookupTable/      # data folder; `lookup_table.bin` stored here
```

How to contribute
------------------
If you want to contribute:
1. Fork and create a branch for your feature or fix.
2. Be sure to re-run `./generator` if you change generation logic and then `make` to rebuild.
3. Add tests or example builds if you change core logic.

Credits
-------
This project was authored by JKidding (see credits).
