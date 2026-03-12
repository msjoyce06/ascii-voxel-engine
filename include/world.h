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
    vector_t offset;
    uint8_t bits[CHUNK_BYTES];
} chunk_t;

/** returns 1 if block is present,
  * 0 if not, -1 if out of bounds  */
int block_present(const chunk_t *chunk, int x, int y, int z);

void set_block(chunk_t *chunk, int x, int y, int z);

void clear_block(chunk_t *chunk, int x, int y, int z);

void load_chunks(chunk_t *chunk, int render_dist);

// block_t place_block(vector_t pos);

#endif
