// cube.cpp
#include "cube.h"

Cube::Cube(const glm::vec3& position, float size, const Material& mat)
        : position(position), size(size), Object(mat) {}

Intersect Cube::rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
    glm::vec3 halfSize = glm::vec3(size) * 0.5f;
    glm::vec3 minBound = position - halfSize;
    glm::vec3 maxBound = position + halfSize;

    glm::vec3 invDirection = 1.0f / rayDirection;

    float t1 = (minBound.x - rayOrigin.x) * invDirection.x;
    float t2 = (maxBound.x - rayOrigin.x) * invDirection.x;
    float t3 = (minBound.y - rayOrigin.y) * invDirection.y;
    float t4 = (maxBound.y - rayOrigin.y) * invDirection.y;
    float t5 = (minBound.z - rayOrigin.z) * invDirection.z;
    float t6 = (maxBound.z - rayOrigin.z) * invDirection.z;

    float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
    float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

    if (tmax < 0 || tmin > tmax) {
        return Intersect{false};
    }

    float t = (tmin < 0.0f) ? tmax : tmin;

    glm::vec3 point = rayOrigin + t * rayDirection;

    // Compute normal based on the closest axis
    glm::vec3 normal;
    if (t == t1) normal = glm::vec3(-1, 0, 0);
    else if (t == t2) normal = glm::vec3(1, 0, 0);
    else if (t == t3) normal = glm::vec3(0, -1, 0);
    else if (t == t4) normal = glm::vec3(0, 1, 0);
    else if (t == t5) normal = glm::vec3(0, 0, -1);
    else normal = glm::vec3(0, 0, 1);

    return Intersect{true, t, point, normal};
}
