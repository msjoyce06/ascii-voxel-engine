#include "world.h"
#include <string.h>
#include <stdio.h>

static inline int block_index(int x, int y, int z) {
    if (x < 0 || CHUNK_X < x || y < 0 || CHUNK_Y < y || z < 0 || CHUNK_Z < z)
        return -1;
    return x + CHUNK_X * (z + CHUNK_Z * y);
}

int get_block(const chunk_t *chunk, int x, int y, int z) {
    int i = block_index(x, y, z);
    if (i == -1) return 0;
    return (chunk->bits[i/8] >> (7 - (i % 8))) & 1;
}

void set_block(chunk_t *chunk, int x, int y, int z) {
    int i = block_index(x, y, z);
    if (i == -1) return;
    chunk->bits[i/8] |= (1u << (7 - (i % 8)));
}

void clear_block(chunk_t *chunk, int x, int y, int z) {
    int i = block_index(x, y, z);
    if (i == -1) return;
    chunk->bits[i/8] &= ~(1u << (7 - (i%8)));
}

void init_chunk(chunk_t *chunk, int bit) {
    uint8_t byte = bit ? ~(uint8_t)0 : (uint8_t)0;
    printf("%u\n", byte);
    memset(chunk->bits, byte, CHUNK_BYTES);
}

// block_t place_block(vector_t pos) {
    // block_t block;
    // for (int i = 0; i < 8; i++) {
        // block.vtxs[i].x = (i & 1) - 0.5f + pos.x;
        // block.vtxs[i].y = ((i >> 1) & 1) - 2 + pos.y;
        // block.vtxs[i].z = ((i >> 2) & 1) - 0.5f + pos.z;
    // }

    // block.faces[0] = (face_t){ .idxs = {0, 1, 3, 2}, .norm = {0, 0, -1}, .c = '$' };
    // block.faces[1] = (face_t){ .idxs = {0, 1, 5, 4}, .norm = {0, -1, 0}, .c = '/' };
    // block.faces[2] = (face_t){ .idxs = {0, 2, 6, 4}, .norm = {-1, 0, 0}, .c = '?' };
    // block.faces[3] = (face_t){ .idxs = {1, 3, 7, 5}, .norm = {1,  0, 0}, .c = '#' };
    // block.faces[4] = (face_t){ .idxs = {2, 3, 7, 6}, .norm = {0,  1, 0}, .c = '@' };
    // block.faces[5] = (face_t){ .idxs = {4, 5, 7, 6}, .norm = {0,  0, 1}, .c = '+' };

    // return block;
// }
