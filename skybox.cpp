#include "skybox.h"
#include <SDL_image.h>
#include <stdexcept>

Skybox::Skybox(const std::string& textureFilePath) {
    loadTexture(textureFilePath);
}

Skybox::~Skybox() {
    SDL_FreeSurface(textureSurface);
}

void Skybox::loadTexture(const std::string& textureFilePath) {
    SDL_Surface* rawTexture = IMG_Load(textureFilePath.c_str());
    if (rawTexture == nullptr) {
        throw std::runtime_error("Error loading image: " + std::string(SDL_GetError()));
    }

    // Convert the loaded image to RGB format
    textureSurface = SDL_ConvertSurfaceFormat(rawTexture, SDL_PIXELFORMAT_RGB24, 0);
    if (textureSurface == nullptr) {
        SDL_FreeSurface(rawTexture);
        throw std::runtime_error("Error converting skybox texture to RGB: " + std::string(SDL_GetError()));
    }

    SDL_FreeSurface(rawTexture);
}

Color Skybox::getColor(const glm::vec3& direction) const {
    // Convert direction vector to spherical coordinates
    float phi = atan2(direction.z, direction.x);
    float theta = acos(direction.y);

    // Convert spherical coordinates to texture coordinates
    float u = 0.5f + phi / (2 * M_PI);
    float v = theta / M_PI;

    // Map texture coordinates to pixel coordinates
    int x = static_cast<int>(u * textureSurface->w) % textureSurface->w;
    int y = static_cast<int>(v * textureSurface->h) % textureSurface->h;

    // Ensure x and y are within the valid range
    x = std::max(0, std::min(textureSurface->w - 1, x));
    y = std::max(0, std::min(textureSurface->h - 1, y));

    // Get pixel color from texture
    Uint8 r, g, b;
    Uint8* pixel = &((Uint8*)textureSurface->pixels)[3 * (y * textureSurface->w + x)];
    r = pixel[0];
    g = pixel[1];
    b = pixel[2];

    return Color(r, g, b);
}
