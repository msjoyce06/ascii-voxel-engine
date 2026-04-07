#include "vectors.h"
#include <math.h>

vecf_t vf(veci_t vi) {
    return (vecf_t){vi.x, vi.y, vi.z};
}

veci_t vi(vecf_t vf) {
    return (veci_t){vf.x, vf.y, vf.z};
}

vecf_t v_addf(vecf_t v1, vecf_t v2) {
    return (vecf_t){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

vecf_t v_subf(vecf_t v1, vecf_t v2) {
    return (vecf_t){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

float v_dotf(vecf_t v1, vecf_t v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

float v_distf(vecf_t v1, vecf_t v2) {
    return sqrtf((v2.x - v1.x) * (v2.x - v1.x) +
                 (v2.y - v1.y) * (v2.y - v1.y) +
                 (v2.z - v1.z) * (v2.z - v1.z));
}

vecf_t v_rotatef(vecf_t v, float cost, float sint, float cosp, float sinp) {
    return (vecf_t){
        v.x*(cost) + v.z*(-sint),
        v.x*(-sint*sinp) + v.y*(cosp) + v.z*(-cost*sinp),
        v.x*(sint*cosp) + v.y*(sinp) + v.z*(cost*cosp)
    };
}
