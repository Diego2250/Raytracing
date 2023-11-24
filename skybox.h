#pragma once

#include <string>
#include "glm/glm.hpp"
#include "color.h"

class Skybox {
public:
    Skybox(const std::string& textureFilePath);
    ~Skybox();

    Color getColor(const glm::vec3& direction) const;

private:
    SDL_Surface* texture;
    void loadAndConvertTexture(const std::string& textureFilePath);
};
