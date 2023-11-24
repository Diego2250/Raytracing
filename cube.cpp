// Nuevo cube.cpp
#include "cube.h"

Cube::Cube(const glm::vec3& minVertex, const glm::vec3& maxVertex, const Material& material)
        : minVertex(minVertex), maxVertex(maxVertex), Object(material) {}

Intersect Cube::rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
    float tMin = (minVertex.x - rayOrigin.x) / rayDirection.x;
    float tMax = (maxVertex.x - rayOrigin.x) / rayDirection.x;

    if (tMin > tMax) {
        std::swap(tMin, tMax);
    }

    float tyMin = (minVertex.y - rayOrigin.y) / rayDirection.y;
    float tyMax = (maxVertex.y - rayOrigin.y) / rayDirection.y;

    if (tyMin > tyMax) {
        std::swap(tyMin, tyMax);
    }

    if ((tMin > tyMax) || (tyMin > tMax)) {
        return Intersect{false};
    }

    if (tyMin > tMin) {
        tMin = tyMin;
    }

    if (tyMax < tMax) {
        tMax = tyMax;
    }

    float tzMin = (minVertex.z - rayOrigin.z) / rayDirection.z;
    float tzMax = (maxVertex.z - rayOrigin.z) / rayDirection.z;

    if (tzMin > tzMax) {
        std::swap(tzMin, tzMax);
    }

    if ((tMin > tzMax) || (tzMin > tMax)) {
        return Intersect{false};
    }

    if (tzMin > tMin) {
        tMin = tzMin;
    }

    if (tzMax < tMax) {
        tMax = tzMax;
    }

    if (tMin < 0 && tMax < 0) {
        return Intersect{false};
    }

    glm::vec3 hitPoint = rayOrigin + tMin * rayDirection;
    glm::vec3 hitNormal = glm::normalize(hitPoint - minVertex);

    glm::vec3 center = (minVertex + maxVertex) / 2.0f;
    float edgeLength = maxVertex.x - minVertex.x;

    glm::vec3 delta = rayOrigin + tMin * rayDirection - center;
    glm::vec3 absDelta = glm::abs(delta);

    // Determine which face was hit
    if (absDelta.x > absDelta.y && absDelta.x > absDelta.z) {
        hitNormal = glm::vec3(delta.x > 0 ? 1 : -1, 0, 0);
    } else if (absDelta.y > absDelta.z) {
        hitNormal = glm::vec3(0, delta.y > 0 ? 1 : -1, 0);
    } else {
        hitNormal = glm::vec3(0, 0, delta.z > 0 ? 1 : -1);
    }

    // Calculate UV coordinates for the hit face
    float tx, ty;
    if (absDelta.x > absDelta.y && absDelta.x > absDelta.z) {
        hitNormal = glm::vec3(delta.x > 0 ? 1 : -1, 0, 0);
        tx = (delta.y + edgeLength / 2) / edgeLength;
        ty = (delta.z + edgeLength / 2) / edgeLength;
    } else if (absDelta.y > absDelta.z) {
        hitNormal = glm::vec3(0, delta.y > 0 ? 1 : -1, 0);
        tx = (delta.x + edgeLength / 2) / edgeLength;
        ty = (delta.z + edgeLength / 2) / edgeLength;
    } else {
        hitNormal = glm::vec3(0, 0, delta.z > 0 ? 1 : -1);
        tx = (delta.x + edgeLength / 2) / edgeLength;
        ty = (delta.y + edgeLength / 2) / edgeLength;
    }

    tx = glm::clamp(tx, 0.0f, 1.0f);
    ty = glm::clamp(ty, 0.0f, 1.0f);

    return Intersect{true, tMin, hitPoint, hitNormal, tx, ty};
}
