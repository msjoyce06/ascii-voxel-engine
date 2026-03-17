#include "world.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

static int block_index(int x, int y, int z) {
    if (x < 0 || CHUNK_X <= x || y < 0 || CHUNK_Y <= y || z < 0 || CHUNK_Z <= z)
        return -1;
    return y*CHUNK_Z*CHUNK_X + z*CHUNK_X + x;
}

// int block_pos_from_idx(int i) {
    // if (i < 0 || CHUNK_BLOCKS < i) return -1;
    // int y = i / (CHUNK_X*CHUNK_Z);
    // int z = (i / CHUNK_X) % CHUNK_Z;
    // int x = i % CHUNK_X;
// }

static int block_present_chunk(const chunk_t *chunk, veci_t chunk_offset) {
    int i = block_index(x, y, z);
    if (i == -1) return -1;
    return (chunk->bits[i/8] >> (7 - (i % 8))) & 1;
}

bool block_present_world(vecf_t world_pos, chunk_t chunks[] {
    int x = (int)world_pos.x;
    int y = (int)world_pos.y;
    int z = (int)world_pos.z;
    veci_t chunk_coord = {x/CHUNK_X, y/CHUNK_Y, z/CHUNK_Z}; // key
    veci_t chunk_offset = {
        .x = world_pos.x - chunk_coord.x,
        .y = world_pos.y - chunk_coord.y,
        .z = world_pos.z - chunk_coord.z
    };
    int chunk_idx = chunk_coord.y*4 + chunk_coord.z*2 + chunk_coord.x;
    return block_present(chunks[chunk_idx], chunk_offset);
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

static void init_chunk(chunk_t *chunk) {
    memset(chunk->bits, 0, CHUNK_BYTES);
}

void load_chunks(chunk_t chunks[], int render_dist) {
    for (int y = -1; y < render_dist; y++) {
        for (int z = -1; z < render_dist; z++) {
            for (int x = -1; x < render_dist; x++) {
                int y_n = y + 1;
                int z_n = z + 1;
                int x_n = x + 1;
                int chunk_idx = y_n*4 + z_n*2 + x_n;
                assert(0 <= chunk_idx && chunk_idx < 8);

                chunk_t new_chunk;
                new_chunk.coord = (veci_t){x, y, z};
                init_chunk(&new_chunk);
                chunks[chunk_idx] = new_chunk;
            }
        }
    }
}
