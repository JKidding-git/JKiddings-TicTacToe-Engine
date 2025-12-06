#ifndef TTT_H // XYyJCQAwCANX6Qi5%2FZczPhVakXL1QoQE4uTrGfBe8CW%2FHl8qUqrkq5xd6J7OfIUNq7JpFAQ%3D
#define TTT_H // Decoded: Made By JKidding
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#define TTT_NODE_SIZE 19683 // 3^9 possible board states, each cell can be empty, X, or O (maybe also illegal states, but whatever. I only have 100 lines to work with)

// Global game state variables
int board                = 0b000000000;                // 9 bits for 9 cells
int player[2]            = {0b000000000, 0b000000000}; // Player X and O boards
enum {X, O} current_player = X;                        // X starts first, that one is obvious.

// Places a mark at the index's position (0-8), returns true if successful, false if illegal move. 
static inline bool place(int index) { 
    if ((board & (1 << index)) == 0 && index >= 0 && index < 9) {
        player[current_player] |= (1 << index);
        board |= (1 << index);
        current_player = (current_player == X) ? O : X; // Switch player
        return true;
    }
    return false;
}

// Removes a mark from position "index" (0-8), could be useful for backtracking for a game tree search.
static inline bool unplace(int index) {
    if (board & (1 << index) && index >= 0 && index < 9) {
        current_player = (current_player == X) ? O : X; // Switch back
        player[current_player] &= ~(1 << index);
        board &= ~(1 << index);
        return true;
    }
    return false;
}

// If all of the cells are filled, it's a draw. Obviously lmao
static inline bool is_Draw() { return board == 0b111111111; }

// Checks if the given player has won
static inline bool is_winner(int players_board) {
    // Check all winning combinations using bitmasks
    const int win_masks[8] = {0b111000000, 0b000111000, 0b000000111, 0b100100100, 0b010010010, 0b001001001, 0b100010001, 0b001010100};
    for (int i = 0; i < 8; i++) {
        if ((players_board & win_masks[i]) == win_masks[i]) {
            return true;
        }
    }
    return false;
}

// Encodes the current board state into a unique integer (0-19682) Useful for either hashing, or a table lookup.
int encode_board() {
    int encoded = 0;
    for (int i = 0; i < 9; i++) {
        int cell_value = 0;
        if (player[X] & (1 << i)) cell_value = 1;
        else if (player[O] & (1 << i)) cell_value = 2;
        encoded += cell_value * (int)pow(3, i);
    }
    return encoded;
}

// Decodes an integer (0-19682) back into the board state, could be used for loading data up from a table.
bool decode_board(int encoded) {
    board = 0;
    player[X] = 0;
    player[O] = 0;
    for (int i = 0; i < 9; i++) {
        int cell_value = (encoded / (int)pow(3, i)) % 3;
        if (cell_value == 1) {
            player[X] |= (1 << i);
            board |= (1 << i);
        } else if (cell_value == 2) {
            player[O] |= (1 << i);
            board |= (1 << i);
        }
    }
    return true; // success
}

// Prints the current board state
void print_board() {
    char symbols[3] = {'.', 'X', 'O'};
    printf("\n");
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int i = row * 3 + col;
            char sym = symbols[0];
            if (player[X] & (1 << i)) sym = symbols[1];
            else if (player[O] & (1 << i)) sym = symbols[2];
            printf(" %c ", sym);
            if (col < 2) printf("|");
        }
        printf("\n");
        if (row < 2) printf("---+---+---\n");
    }
    printf("    Current player: %c\n", (current_player == X) ? 'X' : 'O');
    printf("    Encoded board: %d\n", encode_board());
}

#endif // TTT_H