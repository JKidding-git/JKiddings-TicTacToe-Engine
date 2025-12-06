// library for saving/loading the lookup_table table
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#include "INCBIN/incbin.h"
#include "ttt.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

// Decoded representation used by decode_lookup_table
typedef struct TTTDecoded {
    uint16_t board_encoded;
    bool current_player;
    uint8_t best_index;
} TTTDecoded;
TTTDecoded lookup_table[TTT_NODE_SIZE];

INCBIN(lookup_table, "src/LookupTable/lookup_table.bin");

// Unpack the 20-bit value back into parts
TTTDecoded decode_lookup_table(int encoded) {
    TTTDecoded t = {0};

    t.board_encoded  = (encoded >> 5) & 0x7FFF;
    t.current_player = (encoded >> 4) & 0x1;
    t.best_index     = encoded & 0xF;
    return t;
}

void init_lookup_table() {
    size_t data_size = (size_t)glookup_table_size;
    size_t entry_count = data_size / sizeof(uint32_t);
    printf("Loading lookup_table with %zu entries...\n", entry_count);

    const unsigned char *src = (const unsigned char *)glookup_table_data;
    for (size_t i = 0; i < entry_count; ++i) {
        uint32_t encoded = 0;
        // memcpy to avoid unaligned access and follow host endianness
        memcpy(&encoded, src + (i * sizeof(uint32_t)), sizeof(encoded));
        lookup_table[i] = decode_lookup_table((int)encoded);
    }
}