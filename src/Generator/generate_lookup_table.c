#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../ttt.h"
#include <stdbool.h>
#include <stdint.h>

#define INVALID_MOVE 15
#define INF 32000

// Representation for encoded lookup_table entry
struct TTTlookup_table {
    // Explanation:
    // B = board_encoded (15 bits) (0-32767)
    // P = current_player (1 bit) (0 or 1)
    // M = best_index (4 bits) (0-15, where 15 means
    // This means: BBBBBBBBBBBBBBBPMMMM
    // Total: 20 bits (uint32_t or int.)

    // encoded_result: stored as (board << 5) | (player << 4) | best_move
    int encoded_result;
};

// lookup_table table global definition
struct TTTlookup_table lookup_table[TTT_NODE_SIZE];

bool is_position_legal() {

    // Count bits for each player
    int x_count = __builtin_popcount(player[X]);
    int o_count = __builtin_popcount(player[O]);

    // Check for winners
    bool x_win = is_winner(player[X]);
    bool o_win = is_winner(player[O]);

    // Players cannot occupy the same square
    if (player[X] & player[O]) return false;

    // Move count must be either equal (O to move) or X has exactly one more move
    if (!(x_count == o_count || x_count == o_count + 1)) return false;

    // Both players can't win simultaneously
    if (x_win && o_win) return false;

    // If X won, X must have played the last move (one more than O)
    if (x_win && x_count != o_count + 1) return false;

    // If O won, move counts must be equal (O played last)
    if (o_win && x_count != o_count) return false;

    return true;
}

int best_indices[9] = {0};

int alphabeta(int depth, int alpha, int beta) {

    // Check if the previous move resulted in a win
    if (is_winner(player[!current_player])) return -100 + depth; // Loss for current_player
    
    // Check for draw
    if (is_Draw()) return 0;
    
    for (int i = 0; i < 9; i++) {
        if (place(i)) {
            int score = -alphabeta(depth + 1, -beta, -alpha);
            unplace(i);

            if (score > alpha) {
                alpha = score;
                best_indices[depth] = i;
            }
            if (alpha >= beta) break; // Beta cut-off
        }
    }
    
    // If no moves were possible (should be caught by is_Draw, but just in case)
    if (alpha == -INF) return 0;

    return alpha;
}


// Pack the fields into 20 bits
int encode_to_lookup_table(int board_encoded, int current_player, int best_index) {

    // Make sure values fit their bit ranges
    board_encoded   &= 0x7FFF; // 15 bits
    current_player  &= 0x1;    // 1 bit
    best_index      &= 0xF;    // 4 bits

    return (board_encoded << 5) | (current_player << 4) | best_index;
}


void generate_lookup_table() {
    int generated_entries = 0;
    printf("Generating lookup_table Table...\n");
    for (int i = 0; i < TTT_NODE_SIZE; i++) {
        decode_board(i);

        int x_count = __builtin_popcount(player[X]);
        int o_count = __builtin_popcount(player[O]);
        current_player = (x_count == o_count) ? X : O;

        int best_move = INVALID_MOVE; // 15 (invalid/none)

        if (is_position_legal()) {
            if (!is_winner(player[X]) && !is_winner(player[O]) && !is_Draw()) {
                alphabeta(0, -INF, INF);
                int move = best_indices[0];
                if (move != -1) best_move = move;
                generated_entries++;
            }
        }

        lookup_table[i].encoded_result = encode_to_lookup_table(i, current_player, best_move);
    }

    size_t size = sizeof(lookup_table->encoded_result) * TTT_NODE_SIZE;
    size_t entries = size / sizeof(struct TTTlookup_table);
    double kilobytes = size / 1024.0;
    printf("Generated %.2f KB for %zu entries.\n", kilobytes, entries);

    // saving to file: lookup_table.bin
    FILE *f = fopen("src/LookupTable/lookup_table.bin", "wb");
    if (f == NULL) {
        perror("Failed to open src/LookupTable/lookup_table.bin for writing");
        exit(EXIT_FAILURE);
    } else {
        size_t written = fwrite(lookup_table, sizeof(struct TTTlookup_table), TTT_NODE_SIZE, f);
        if (written != TTT_NODE_SIZE) {
            perror("Failed to write complete lookup_table table to file");
            fclose(f);
            exit(EXIT_FAILURE);
        }
        fclose(f);
    }

    printf("lookup_table Generated with %d entries.\n", generated_entries);
}


int main() {
    generate_lookup_table();
    return 0;
}