#include "skybox.h"
#include <SDL_image.h>

Skybox::Skybox(const std::string& textureFile) {
    loadAndConvertTexture(textureFile);
}

Skybox::~Skybox() {
    if (texture != nullptr) {
        SDL_FreeSurface(texture);
        texture = nullptr;
    }
}

void Skybox::loadAndConvertTexture(const std::string& textureFile) {
    SDL_Surface* rawTexture = IMG_Load(textureFile.c_str());
    if (rawTexture == nullptr) {
        throw std::runtime_error("Error loading image: " + std::string(SDL_GetError()));
    }

    texture = SDL_ConvertSurfaceFormat(rawTexture, SDL_PIXELFORMAT_RGB24, 0);
    if (texture == nullptr) {
        SDL_FreeSurface(rawTexture);
        throw std::runtime_error("Error converting texture to RGB: " + std::string(SDL_GetError()));
    }

    SDL_FreeSurface(rawTexture);
}

Color Skybox::getColor(const glm::vec3& direction) const {
    float phi = atan2(direction.z, direction.x);
    float theta = acos(direction.y);

    float u = 0.5f + phi / (2 * M_PI);
    float v = theta / M_PI;

    int x = static_cast<int>(u * texture->w) % texture->w;
    int y = static_cast<int>(v * texture->h) % texture->h;

    x = std::clamp(x, 0, texture->w - 1);
    y = std::clamp(y, 0, texture->h - 1);

    Uint8 r, g, b;
    Uint8* pixel = &((Uint8*)texture->pixels)[3 * (y * texture->w + x)];
    r = pixel[0];
    g = pixel[1];
    b = pixel[2];

    return Color(r, g, b);
}


