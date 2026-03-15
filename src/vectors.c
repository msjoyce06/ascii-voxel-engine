#include "vectors.h"
#include <math.h>

vectorf_t vf(vectori_t vi) {
    return (vectorf_t)vi;
}

vectori_t vi(vectorf_t vf) {
    return (vectori_t)vf;
}

vectorf_t v_addf(vectorf_t v1, vectorf_t v2) {
    return (vectorf_t){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

vectorf_t v_subf(vectorf_t v1, vectorf_t v2) {
    return (vectorf_t){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

float v_dotf(vectorf_t v1, vectorf_t v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

vectorf_t v_rotatef(vectorf_t v, float cost, float sint, float cosp, float sinp) {
    vectorf_t rotated;
    rotated.x = v.x*(cost) + v.z*(-sint);
    rotated.y = v.x*(-sint*sinp) + v.y*(cosp) + v.z*(-cost*sinp);
    rotated.z = v.x*(sint*cosp) + v.y*(sinp) + v.z*(cost*cosp);

    return rotated;
}
