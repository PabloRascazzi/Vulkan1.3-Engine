#ifdef __cplusplus
#include <glm/glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = uint32_t;
#endif

const int BLOCK_X = 32, BLOCK_Y = 32;
const int BLOCK_SIZE = BLOCK_X * BLOCK_Y;

struct Gaussian {
    vec2 mean2D;
    vec3 conic;
    vec3 color;
    float opacity;
};