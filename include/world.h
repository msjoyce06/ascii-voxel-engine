#ifndef WORLD_H
#define WORLD_H

#include "vectors.h"
#include <stdint.h>
#include <stdbool.h>

#define CHUNK_X 16
#define CHUNK_Y 16
#define CHUNK_Z 16
#define CHUNK_BLOCKS (CHUNK_X * CHUNK_Y * CHUNK_Z)
#define CHUNK_BYTES ((CHUNK_BLOCKS + 7) / 8)

typedef struct {
    veci_t coord;
    uint8_t bits[CHUNK_BYTES];
} chunk_t;

bool is_solid_in_chunk(const chunk_t *chunk, veci_t chunk_offset);
bool is_solid_block(const chunk_t chunks[], veci_t block);

void set_block(chunk_t chunks[], veci_t block);
void clear_block(chunk_t chunks[], veci_t block);

void load_chunks(chunk_t chunks[], int render_dist);

#endif
