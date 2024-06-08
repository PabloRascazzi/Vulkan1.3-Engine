#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = uint32_t;
#endif

const int SPHERICAL_HARMONICS_DEGREE = 3; // Range [0, 3]
const int SPHERICAL_HARMONICS_MAX_COEF = 3 * (SPHERICAL_HARMONICS_DEGREE == 0 ? 1 :
                                              SPHERICAL_HARMONICS_DEGREE == 1 ? 4 : 
                                              SPHERICAL_HARMONICS_DEGREE == 2 ? 9 : 16);
const int BLOCK_X = 32, BLOCK_Y = 32;
const int BLOCK_SIZE = BLOCK_X * BLOCK_Y;

// Spherical harmonics coefficients.
// From https://github.com/graphdeco-inria/diff-gaussian-rasterization.
const float SH_C0 = 0.28209479177387814f;
const float SH_C1 = 0.4886025119029199f;
const float SH_C2[] = {
	1.0925484305920792f,
	-1.0925484305920792f,
	0.31539156525252005f,
	-1.0925484305920792f,
	0.5462742152960396f
};
const float SH_C3[] = {
	-0.5900435899266435f,
	2.890611442640554f,
	-0.4570457994644658f,
	0.3731763325901154f,
	-0.4570457994644658f,
	1.445305721320277f,
	-0.5900435899266435f
};

struct Gaussian {
    vec3 position;
    vec3 scale;
    vec4 rotation;
#ifdef __cplusplus
    float sh[SPHERICAL_HARMONICS_MAX_COEF];
#else 
    float[SPHERICAL_HARMONICS_MAX_COEF] sh;
#endif
	float opacity;
};

struct ProcessedGaussian {
    vec2 mean2D;
    vec3 conic;
    vec3 color;
    float opacity;
};

struct Rect2D {
	uint left, bottom, right, top;
};