#ifndef TTT_H // XYyJCQAwCANX6Qi5%2FZczPhVakXL1QoQE4uTrGfBe8CW%2FHl8qUqrkq5xd6J7OfIUNq7JpFAQ%3D
#define TTT_H // Decoded: Made By JKidding
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define TTT_NODE_SIZE 19683 // 3^9 possible board states, each cell can be empty, X, or O (maybe also illegal states, but whatever. I only have 100 lines to work with)

// Global game state variables
static uint16_t player[2]            = {0b000000000, 0b000000000}; // Player X and O boards
static enum {X, O} current_player = X;                        // X starts first, that one is obvious.

// Places a mark at the index's position (0-8), returns true if successful, false if illegal move. 
static inline bool place(int index) { 
    if (((player[X] | player[O]) & (1 << index)) == 0 && index >= 0 && index < 9) {
        player[current_player] |= (1 << index);
        current_player = (current_player == X) ? O : X; // Switch player
        return true;
    }
    return false;
}

// Removes a mark from position "index" (0-8), could be useful for backtracking for a game tree search.
static inline bool unplace(int index) {
    if (((player[X] | player[O]) & (1 << index)) && index >= 0 && index < 9) {
        current_player = (current_player == X) ? O : X; // Switch back
        player[current_player] &= ~(1 << index);
        return true;
    }
    return false;
}

// If all of the cells are filled, it's a draw. Obviously lmao
#define is_Draw() (((player[X] | player[O]) & 0x1FF) == 0x1FF)

// Checks if the given player has won
static inline bool is_winner(uint16_t b) {
    // Unrolled checks (avoids array + loop; lets the compiler generate straight-line code)
    return ((b & 0x1C0u) == 0x1C0u) || // 111000000 (top row)
           ((b & 0x038u) == 0x038u) || // 000111000 (middle row)
           ((b & 0x007u) == 0x007u) || // 000000111 (bottom row)
           ((b & 0x124u) == 0x124u) || // 100100100 (left col)
           ((b & 0x092u) == 0x092u) || // 010010010 (mid col)
           ((b & 0x049u) == 0x049u) || // 001001001 (right col)
           ((b & 0x111u) == 0x111u) || // 100010001 (diag)
           ((b & 0x054u) == 0x054u);   // 001010100 (anti-diag)
}

// Encodes the current board state into a unique integer (0-19682) Useful for either hashing, or a table lookup.
static inline int encode_board(void) {
    int encoded = 0;
    int pow3 = 1;
    for (int i = 0; i < 9; i++) {
        int cell = 0;
        if (player[X] & (1u << i)) cell = 1;
        else if (player[O] & (1u << i)) cell = 2;
        encoded += cell * pow3;
        pow3 *= 3;
    }
    return encoded;
}

// Decodes an integer (0-19682) back into the board state, could be used for loading data up from a table.
static inline bool decode_board(int encoded) {
    if (encoded < 0 || encoded >= TTT_NODE_SIZE) return false;
    uint16_t x = 0;
    uint16_t o = 0;
    for (int i = 0; i < 9; i++) {
        int cell = encoded % 3;
        encoded /= 3;
        // These evaluate to either 0 or (1 << i)
        x |= (uint16_t)((cell == 1) ? (1u << i) : 0u);
        o |= (uint16_t)((cell == 2) ? (1u << i) : 0u);
    }
    player[X] = x;
    player[O] = o;
    return true;
}


// Prints the current board state
static inline void print_board() {
    char symbols[3] = {'.', 'X', 'O'};
    printf("\n");
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int i = row * 3 + col;
            char sym = symbols[0];
            if (player[X] & (1u << i)) sym = symbols[1];
            else if (player[O] & (1u << i)) sym = symbols[2];
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