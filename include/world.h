#ifndef WORLD_H
#define WORLD_H

#include "vectors.h"
#include <stdint.h>

#define CHUNK_X 16
#define CHUNK_Y 16
#define CHUNK_Z 16
#define CHUNK_BLOCKS (CHUNK_X * CHUNK_Y * CHUNK_Z)
#define CHUNK_BYTES ((CHUNK_BLOCKS + 7) / 8)

typedef struct {
    uint8_t bits[CHUNK_BYTES];
} chunk_t;

/** returns -1 if out of bounds */
int block_index(int x, int y, int z);

int get_block(const chunk_t *chunk, int x, int y, int z);

int set_block(chunk_t *chunk, int x, int y, int z);

void clear_block(chunk_t *chunk, int x, int y, int z);

void init_chunk(chunk_t *chunk);

block_t place_block(vector_t pos);

#endif
