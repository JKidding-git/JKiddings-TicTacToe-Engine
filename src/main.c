#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


// Core game logic and lookup_table stuff.
#include "ttt.h"
#include "lookup_table.h"

// for benchmarking i guess.
#include <time.h>

// This is just to remove code duplication from the benchmark and self-play modes.
int self_play(int moves_this_game) {
    while (true) {
        print_board();

        if (is_Draw()) {
            printf("It's a draw!\n");
            break;
        }

        if (is_winner(player[X])) {
            printf("Something is broken with the Lookup Table: PXW. \n");
            break;
        }

        if (is_winner(player[O])) {
            printf("Something is broken with the Lookup Table: POW. \n");
            break;
        }

        int lookup_table_move = -1;
        int board_idx = encode_board();
        TTTDecoded dec = lookup_table[board_idx];

        if (dec.current_player == current_player && dec.best_index <= 8) {
            lookup_table_move = dec.best_index;
        }

        int move = lookup_table_move;
        int played_player = current_player;
        
        bool moved = place(move);
        if (!moved) {
            printf("Self-play Error: Invalid move by player %c at position %d\n", (played_player == X) ? 'X' : 'O', move);
            break;
        }

        printf("Player %c plays at position %d\n", (played_player == X) ? 'X' : 'O', moved ? move : -1);
        ++moves_this_game;
    }
    return moves_this_game;
}

int self_play_benchmark(int moves_this_game) {
    while (true) {
        if (is_Draw()) break;

        int lookup_table_move = -1;
        int board_idx = encode_board();
        TTTDecoded dec = lookup_table[board_idx];

        if (dec.current_player == current_player && dec.best_index <= 8) {
            lookup_table_move = dec.best_index;
        }

        int move = lookup_table_move;
        int played_player = current_player;
        
        // check move validity
        if (!place(move)) {
            printf("Benchmark Error: Invalid move by player %c at position %d\n", (played_player == X) ? 'X' : 'O', move);
            break;
        }
        ++moves_this_game;
    }
    return moves_this_game;
}

void Benchmark(double target_seconds) {

    // Initialize game state or smth idk.
    current_player = X;
    printf("Running Benchmark...\n");

    // Benchmark how many full games we can simulate in ~10 seconds
    clock_t bench_start = clock();
    size_t games_completed = 0;
    size_t total_moves = 0;

    while (((double)(clock() - bench_start) / CLOCKS_PER_SEC) < target_seconds) {
        // reset game state
        current_player = X;
        player[X] = player[O] = 0;
        int moves_this_game = 0;

        // play out a full game (same logic as the interactive mode)
        moves_this_game = self_play_benchmark(moves_this_game);

        ++games_completed;
        total_moves += moves_this_game;
    }

    double elapsed = (double)(clock() - bench_start) / CLOCKS_PER_SEC;
    double games_per_sec = games_completed / elapsed;
    double avg_moves = games_completed ? (double)total_moves / games_completed : 0.0;

    printf("Benchmark: %zu full games in %.3f seconds -> %.2f games/sec, avg %.2f moves/game\n", games_completed, elapsed, games_per_sec, avg_moves);
}



void pva(bool human_as, bool bot_thinks, double bot_thinking_time) {
    while (true) {
        print_board();
        if (is_Draw()) {
            printf("It's a draw!\n");
            break;
        } else if (is_winner(player[X])) {
            printf("Player X wins!\n");
            break;
        } else if (is_winner(player[O])) {
            printf("Player O wins!\n");
            break;
        }
        
        if (human_as == X) {
            int move;
            printf("Player X, enter your move (0-8): ");
            if (scanf("%d", &move) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF); // clear bad input
                printf("Invalid input. Please enter a number 0-8.\n");
                continue;
            }

            if (!place(move)) {
                printf("Invalid move. Try again.\n");
                continue;
            }

            // swap to AI
            human_as = O;
        } else {
            int lookup_table_move = -1;
            int board_idx = encode_board();
            TTTDecoded dec = lookup_table[board_idx];

            if (dec.current_player == current_player && dec.best_index <= 8) {
                lookup_table_move = dec.best_index;
            }

            if (bot_thinks) {
                if (bot_thinking_time) {
                    printf("Bot is thinking...\n");
                    // simulate thinking time
                    clock_t start = clock();
                    while (((double)(clock() - start) / CLOCKS_PER_SEC) < bot_thinking_time) {
                        // busy wait for N seconds
                    }
                }
            }

            int move = lookup_table_move;
            int played_player = current_player;
            bool moved = false;
            if (move >= 0 && move <= 8) {
                moved = place(move);
            }
            if (!moved) {
                for (int i = 0; i < 9; ++i) {
                    if (place(i)) {
                        move = i;
                        moved = true;
                        break;
                    }
                }
            }
            printf("Player %c plays at position %d\n", (played_player == X) ? 'X' : 'O', moved ? move : -1);

            // swap back to human
            human_as = X;
        }
    }
}


void help() {
    printf("=========================================\n");
    printf("   Tic-Tac-Toe lookup_table Engine - Help\n");
    printf("=========================================\n\n");

    printf("Usage:\n");
    printf("  Linux: ./main [command] [options]\n");
    printf("  Windows: main.exe [command] [options]\n\n");

    printf("Commands and options:\n");
    printf("  %-30s %s\n", "--help, -h", "Show this help message");
    printf("  %-30s %s\n", "--credits, -c", "Show credits");
    printf("  %-30s %s\n", "--benchmark", "Run benchmark mode");
    printf("  %-30s %s\n", "    --seconds N", "Set seconds for benchmark (default 1.0)");
    printf("  %-30s %s\n", "--selfplay", "Run self-play mode (lookup_table vs lookup_table)");
    printf("  %-30s %s\n", "--pva [X|O]", "Play against the AI as player X or O");
    printf("  %-30s %s\n", "    --bot-thinks", "Make the bot simulate thinking time in pva");
    printf("  %-30s %s\n", "        --thinking-time N", "Thinking time in seconds for the bot\n");

    printf("Examples:\n");
    printf("  main --benchmark --seconds 5\n");
    printf("      - Run a 5-second benchmark\n\n");
    printf("  main --selfplay\n");
    printf("      - Let the lookup_table play both sides and show moves\n\n");
    printf("  main --pva O --bot-thinks --thinking-time 0.5\n");
    printf("      - Play as O while the bot (X) simulates 0.5s thinking per move\n\n");

    printf("Notes:\n");
    printf("  - Moves are numbered 0->8 (left-to-right, top-to-bottom).\n");
    printf("  - lookup_table must be initialized (embedded) for AI moves to work.\n\n");
}

void credits() {
    printf("Tic-Tac-Toe lookup_table Engine\n");
    printf("Developed by JKidding\n");
    printf("lookup_table generation logic inspired by Chess's endgame lookup_tables\n\n");
    printf("\n\n-INFO-\n");

    printf("The reason why the TTT size is limited to 3x3, is because\n");
    printf("the total number of possible board states is small enough\n");
    printf("to fit into a reasonable lookup_table size (19683 entries) (77KB).\n");
    printf("Larger board sizes (4x4 and beyond) would need significantly more memory and computation.\n");
    printf("Example at 10x10, there would be 3^100 possible board states,\n");
    printf("which is approximately 5.1537752e+47 states, which is infeasible to store or compute. (for the current (2025)'s hardware)\n");
}

void parse_pva_thinking_args(int argc, char **argv) {
    // Currently no arguments to parse

    if (argc > 3 && (strcmp(argv[3], "--bot-thinks") == 0)) {
        double bot_time = 0.0;
        if (argc > 4 && strcmp(argv[4], "--thinking-time") == 0 && argc > 5) {
            bot_time = atof(argv[5]);
        }
        pva(O, true, bot_time);
        exit(0);
    }
}

void parse_args(int argc, char **argv) {

    // Help mode
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        help();
        exit(0);
    }

    // Credits mode
    if (argc > 1 && (strcmp(argv[1], "--credits") == 0 || strcmp(argv[1], "-c") == 0)) {
        credits();
        exit(0);
    }

    // Benchmark mode
    if (argc > 1 && strcmp(argv[1], "--benchmark") == 0) {
        // get the seconds using --seconds to run from args.
        double seconds = 1.0;
        if (argc > 3 && strcmp(argv[2], "--seconds") == 0) {
            seconds = atof(argv[3]);
        }
        Benchmark(seconds);
        exit(0);
    }

    // Self-play mode (explicit flag only)
    if (argc > 1 && strcmp(argv[1], "--selfplay") == 0) {
        self_play(0);
        exit(0);
    }

    // PVA Mode
    if (argc > 1 && strcmp(argv[1], "--pva") == 0) {

        // get the human player side from args.
        bool human_as = X; // default human as X if not specified
        if (argc > 2 && (strcmp(argv[2], "O") == 0 || strcmp(argv[2], "o") == 0)) {

            // parse any bot thinking args
            parse_pva_thinking_args(argc, argv);
            human_as = O;
        } else if (argc > 2 && (strcmp(argv[2], "X") == 0 || strcmp(argv[2], "x") == 0)) {

            // parse any bot thinking args
            parse_pva_thinking_args(argc, argv);
            human_as = X;
        }

        // default pva without bot thinking
        pva(human_as, false, 0.0);
        exit(0);
    }

    // Default: show help
    printf("\n\nError: No valid command provided.\n\n");
    help();
    exit(0);
}


// screw this, this shit took me 10 hours. I need a break.
int main(int argc, char **argv) {
    init_lookup_table(); // load lookup_table table from embedded binary
    // init_decode_table();
    parse_args(argc, argv);
    return 0;
}