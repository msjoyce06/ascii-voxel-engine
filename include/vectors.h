#ifndef VECTORS_H
#define VECTORS_H

typedef struct {
    float x;
    float y;
    float z;
} vector_t;

vector_t v_add(vector_t v1, vector_t v2);

vector_t v_sub(vector_t v1, vector_t v2);

float v_dot(vector_t v1, vector_t v2);

vector_t v_rotate(vector_t v, float cost, float sint, float cosp, float sinp);

#endif
