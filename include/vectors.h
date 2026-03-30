#ifndef VECTORS_H
#define VECTORS_H

typedef struct {
    int x;
    int y;
    int z;
} veci_t;

typedef struct {
    float x;
    float y;
    float z;
} vecf_t;

vecf_t vf(veci_t vi);
veci_t vi(vecf_t vf);

vecf_t v_addf(vecf_t v1, vecf_t v2);
vecf_t v_subf(vecf_t v1, vecf_t v2);

float v_dotf(vecf_t v1, vecf_t v2);

vecf_t v_rotatef(vecf_t v, float cost, float sint, float cosp, float sinp);

#endif
