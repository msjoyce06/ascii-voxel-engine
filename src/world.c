#include "world.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

static int block_index(int x, int y, int z) {
    if (x < 0 || CHUNK_X < x || y < 0 || CHUNK_Y < y || z < 0 || CHUNK_Z < z)
        return -1;
    return y*CHUNK_Z*CHUNK_X + z*CHUNK_X + x;
}

// int block_pos_from_idx(int i) {
    // if (i < 0 || CHUNK_BLOCKS < i) return -1;
    // int y = i / (CHUNK_X*CHUNK_Z);
    // int z = (i / CHUNK_X) % CHUNK_Z;
    // int x = i % CHUNK_X;
// }

int block_present(const chunk_t *chunk, int x, int y, int z) {
    int i = block_index(x, y, z);
    if (i == -1) return -1;
    return (chunk->bits[i/8] >> (7 - (i % 8))) & 1;
}

// int get_block_from_idx(const chunk_t *chunk, int i) {
    // if (i < 0 || CHUNK_BLOCKS < i) return -1;
    // return (chunk->bits[i/8] >> (7 - (i % 8))) & 1;
// }

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

static void init_chunk(chunk_t *chunk, int bit) {
    uint8_t byte = bit ? ~(uint8_t)0 : (uint8_t)0;
    memset(chunk->bits, byte, CHUNK_BYTES);
}

void load_chunks(chunk_t chunks[], int render_dist) {
    for (int y = -1; y < render_dist; y++) {
        for (int z = -1; z < render_dist; z++) {
            for (int x = -1; x < render_dist; y++) {
                int y_n = y + 1;
                int z_n = z + 1;
                int x_n = x + 1;
                int chunk_idx = y_n*CHUNK_Z*CHUNK_X + z_n*CHUNK_X + x_n;
                assert(0 <= chunk_idx && chunk_idx < 8);

                chunk_t new_chunk;
                new_chunk.offset = {x*CHUNK_X, y*CHUNK_Y, z*CHUNK_Z};
                int bit = (z < 0) ? 1 : 0;
                chunks[chunk_idx] = init_chunk(&new_chunk, bit);
            }
        }
    }
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
