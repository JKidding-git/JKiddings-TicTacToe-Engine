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
    //   - B = board_encoded (15 bits) (0-32767)
    //   - P = current_player (1 bit) (0 or 1)
    //   - M = best_index (4 bits) (0-15, where 15 means
    //   - This means: BBBBBBBBBBBBBBBPMMMM
    //   - Total: 20 bits (uint32_t or int.)

    // encoded_result: stored as (board << 5) | (player << 4) | best_move
    int encoded_result;
};

// lookup_table table global definition
struct TTTlookup_table lookup_table[TTT_NODE_SIZE];

// Pack the fields into 20 bits
int encode_to_lookup_table(uint16_t board_encoded, bool current_player, uint8_t best_index) {

    // Make sure values fit their bit ranges
    board_encoded   &= 0x7FFF; // 15 bits
    current_player  &= 0x1;    // 1 bit
    best_index      &= 0xF;    // 4 bits

    return (board_encoded << 5) | (current_player << 4) | best_index;
}

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

int alphabeta(int ply, int alpha, int beta) {

    if (is_winner(player[current_player]))  return INF - ply;  // Winning position for current player
    if (is_winner(player[!current_player])) return -INF + ply; // Losing position for current player
    if (is_Draw()) return 0;                                   // Draw
    
    for (int i = 0; i < 9; i++) {
        if (place(i)) {
            int score = -alphabeta(ply + 1, -beta, -alpha);
            unplace(i);

            if (score > alpha) {
                alpha = score;

                best_indices[ply] = i;
            }
            if (alpha >= beta) break; // Beta cut-off
        }
    }

    return alpha;
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

            // Clear old data
            for (int j = 0; j < 9; j++) best_indices[j] = -1;

            if (!is_winner(player[X]) && !is_winner(player[O]) && !is_Draw()) {
                alphabeta(0, -INF, INF);
                if (best_indices[0] != -1) best_move = best_indices[0];
                generated_entries++;
            }
        }

        lookup_table[i].encoded_result = encode_to_lookup_table(i, current_player, best_move);
    }

    printf("Generated %.5f KB for %d entries.\n", sizeof(lookup_table) / 1024.0, TTT_NODE_SIZE);

    // saving to file: lookup_table.bin
    FILE *f = fopen("src/LookupTable/lookup_table.bin", "wb");
    if (!f) {
        printf("Error: Could not open file for writing lookup_table data.\n");
        exit(EXIT_FAILURE);
    }

    // Write the entire lookup_table array to the file
    size_t written = fwrite(lookup_table, sizeof(struct TTTlookup_table), TTT_NODE_SIZE, f);
    if (written != TTT_NODE_SIZE) {
        printf("Error: Could not write all lookup_table data to file.\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }
    fclose(f);

    printf("lookup_table Generated with %d entries.\n", generated_entries);
}

int main() {
    generate_lookup_table();
    return 0;
}