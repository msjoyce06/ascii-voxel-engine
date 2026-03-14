#include "vectors.h"
#include <math.h>

vector_t v_add(vector_t v1, vector_t v2) {
    return (vector_t){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

vector_t v_sub(vector_t v1, vector_t v2) {
    return (vector_t){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

float v_dot(vector_t v1, vector_t v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

// float v_dist(vector_t v1, vector_t v2) {
    // return sqrtf((v2.z-v1.z)*(v2.z-v1.z) +
                 // (v2.y-v1.y)*(v2.y-v1.y) +
                 // (v2.z-v1.z)*(v2.z-v1.z));
// }

vector_t v_rotate(vector_t v, float cost, float sint, float cosp, float sinp) {
    vector_t rotated;

    rotated.x = v.x*(cost) + v.z*(-sint);
    rotated.y = v.x*(-sint*sinp) + v.y*(cosp) + v.z*(-cost*sinp);
    rotated.z = v.x*(sint*cosp) + v.y*(sinp) + v.z*(cost*cosp);

    return rotated;
}
