#pragma once

#include "glm/glm.hpp"
#include "object.h"
#include "material.h"
#include "intersect.h"

class Cube : public Object {
public:
    Cube(const glm::vec3& minVertex, const glm::vec3& maxVertex, const Material& material);

    Intersect rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const override;


private:
    glm::vec3 minVertex;
    glm::vec3 maxVertex;

};
