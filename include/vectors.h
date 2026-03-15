#ifndef VECTORS_H
#define VECTORS_H

typedef struct {
    int x;
    int y;
    int z;
} vectori_t;

typedef struct {
    float x;
    float y;
    float z;
} vectorf_t;

vectorf_t vf(vectori_t vi);
vectori_t vi(vectorf_t vf);

vectorf_t v_addf(vectorf_t v1, vectorf_t v2);
vectorf_t v_subf(vectorf_t v1, vectorf_t v2);

float v_dot(vectorf_t v1, vectorf_t v2);

vectorf_t v_rotate(vectorf_t v, float cost, float sint, float cosp, float sinp);

#endif
