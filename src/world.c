#include "world.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

static int get_block_idx(int x, int y, int z) {
    if (x < 0 || CHUNK_X <= x || y < 0 || CHUNK_Y <= y || z < 0 || CHUNK_Z <= z)
        return -1;
    return y*CHUNK_Z*CHUNK_X + z*CHUNK_X + x;
}

static int floor_div(int a, int b) {
    int q = a / b;
    int r = a % b;
    if (r != 0 && a < 0)
        q--;
    return q;
}

static veci_t get_chunk_coord(veci_t block) {
    return (veci_t){
        .x = floor_div(block.x, CHUNK_X),
        .y = floor_div(block.y, CHUNK_Y),
        .z = floor_div(block.z, CHUNK_Z),
    };
}

static veci_t get_chunk_offset(veci_t block) {
    veci_t offset = {
        .x = block.x % CHUNK_X,
        .y = block.y % CHUNK_Y,
        .z = block.z % CHUNK_Z,
    };
    offset.x = (offset.x < 0) ? offset.x + 16 : offset.x;
    offset.y = (offset.y < 0) ? offset.y + 16 : offset.y;
    offset.z = (offset.z < 0) ? offset.z + 16 : offset.z;
    return offset;
}

// int block_pos_from_idx(int i) {
    // if (i < 0 || CHUNK_BLOCKS < i) return -1;
    // int y = i / (CHUNK_X*CHUNK_Z);
    // int z = (i / CHUNK_X) % CHUNK_Z;
    // int x = i % CHUNK_X;
// }

bool is_solid_in_chunk(const chunk_t *chunk, veci_t chunk_offset) {
    int i = get_block_idx(chunk_offset.x, chunk_offset.y, chunk_offset.z);
    if (i == -1) return false;
    return (bool)((chunk->bits[i/8] >> (7 - (i % 8))) & 1);
}

bool is_solid_block(const chunk_t chunks[], veci_t block) {
    veci_t chunk_coord = get_chunk_coord(block);
    veci_t chunk_offset = get_chunk_offset(block);

    int cx = chunk_coord.x + 1;
    int cy = chunk_coord.y + 1;
    int cz = chunk_coord.z + 1;

    if (cx < 0 || cx >= 2 || cy < 0 || cy >= 2 || cz < 0 || cz >= 2)
        return 0;

    int chunk_idx = cy*4 + cz*2 + cx;
    return is_solid_in_chunk(&chunks[chunk_idx], chunk_offset);
}

static void set_block_in_chunk(chunk_t *chunk, veci_t block) {
    int i = get_block_idx(block.x, block.y, block.z);
    if (i == -1) return;
    chunk->bits[i/8] |= (1u << (7 - (i % 8)));
}

void set_block(chunk_t chunks[], veci_t block) {
    veci_t chunk_coord = get_chunk_coord(block);
    veci_t chunk_offset = get_chunk_offset(block);

    int cx = chunk_coord.x + 1;
    int cy = chunk_coord.y + 1;
    int cz = chunk_coord.z + 1;

    if (cx < 0 || cx >= 2 || cy < 0 || cy >= 2 || cz < 0 || cz >= 2)
        return;

    int chunk_idx = cy*4 + cz*2 + cx;
    set_block_in_chunk(&chunks[chunk_idx], chunk_offset);
}

static void clear_block_in_chunk(chunk_t *chunk, veci_t block) {
    int i = get_block_idx(block.x, block.y, block.z);
    if (i == -1) return;
    chunk->bits[i/8] &= ~(1u << (7 - (i%8)));
}

void clear_block(chunk_t chunks[], veci_t block) {
    veci_t chunk_coord = get_chunk_coord(block);
    veci_t chunk_offset = get_chunk_offset(block);

    int cx = chunk_coord.x + 1;
    int cy = chunk_coord.y + 1;
    int cz = chunk_coord.z + 1;

    if (cx < 0 || cx >= 2 || cy < 0 || cy >= 2 || cz < 0 || cz >= 2)
        return;

    int chunk_idx = cy*4 + cz*2 + cx;
    clear_block_in_chunk(&chunks[chunk_idx], chunk_offset);
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
