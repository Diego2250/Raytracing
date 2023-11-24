#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include "glm/geometric.hpp"
#include <string>
#include "glm/glm.hpp"
#include <vector>
#include <SDL_image.h>
#include "skybox.h"

#include "color.h"
#include "intersect.h"
#include "object.h"
#include "light.h"
#include "camera.h"
#include "cube.h"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float ASPECT_RATIO = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
const int MAX_RECURSION = 1;
const float BIAS = 0.0001f;

SDL_Renderer* renderer;
std::vector<Object*> objects;
Light light(glm::vec3(-5.0, 6.0, 15.0f), 1.5f, Color(255, 255, 255));
Camera camera(glm::vec3(-5.0, 3.0, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);

SDL_Surface* loadTexture(const std::string& file) {
    SDL_Surface* surface = IMG_Load(file.c_str());
    if (surface == nullptr) {
        std::cerr << "Unable to load image: " << IMG_GetError() << std::endl;
    }
    return surface;
}

Skybox skybox("../assets/sky.png");


Color SurfaceColor(SDL_Surface* surface, float u, float v) {
    Color color = {0, 0, 0, 0};

    if (surface != nullptr) {
        if (u < 0) u += 1.0f;
        if (v < 0) v += 1.0f;


        int x = static_cast<int>(u * surface->w);
        int y = static_cast<int>(v * surface->h);

        Uint32 pixel = 0;
        Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
        memcpy(&pixel, p, surface->format->BytesPerPixel);

        SDL_GetRGBA(pixel, surface->format, &color.r, &color.g, &color.b, &color.a);
    }

    return color;
}


void point(glm::vec2 position, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, position.x, position.y);
}

float castShadow(const glm::vec3& shadowOrigin, const glm::vec3& lightDir, Object* hitObject) {
    for (auto& obj : objects) {
        if (obj != hitObject) {
            Intersect shadowIntersect = obj->rayIntersect(shadowOrigin, lightDir);
            if (shadowIntersect.isIntersecting && shadowIntersect.dist > 0) {
                float shadowRatio = shadowIntersect.dist / glm::length(light.position - shadowOrigin);
                shadowRatio = glm::min(1.0f, shadowRatio);
                return 1.0f - shadowRatio;
            }
        }
    }
    return 1.0f;
}

Color castRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const short recursion = 0) {
    float zBuffer = 99999;
    Object* hitObject = nullptr;
    Intersect intersect;

    for (const auto& object : objects) {
        Intersect i = object->rayIntersect(rayOrigin, rayDirection);
        if (i.isIntersecting && i.dist < zBuffer) {
            zBuffer = i.dist;
            hitObject = object;
            intersect = i;
        }
    }

    if (!intersect.isIntersecting || recursion >= MAX_RECURSION) {
        return skybox.getColor(rayDirection);
    }

    glm::vec3 lightDir = glm::mat3(glm::transpose(glm::inverse(hitObject->getTransformMatrix()))) * glm::normalize(light.position - intersect.point);
    glm::vec3 viewDir = glm::mat3(glm::transpose(glm::inverse(hitObject->getTransformMatrix()))) * glm::normalize(rayOrigin - intersect.point);

    glm::vec3 reflectDir = glm::reflect(-lightDir, intersect.normal);

    float shadowIntensity = castShadow(intersect.point, lightDir, hitObject);

    float diffuseLightIntensity = glm::max(0.0f, glm::dot(intersect.normal, lightDir));
    float specLightIntensity = std::pow(glm::max(0.0f, glm::dot(viewDir, reflectDir)), hitObject->material.specularCoefficient);

    Color reflectedColor(0.0f, 0.0f, 0.0f);
    if (hitObject->material.reflectivity > 0) {
        glm::vec3 origin = intersect.point + intersect.normal * BIAS;
        glm::vec3 reflectedRayDirObjSpace = glm::mat3(glm::transpose(glm::inverse(hitObject->getTransformMatrix()))) * reflectDir;
        reflectedColor = castRay(origin, reflectedRayDirObjSpace, recursion + 1);
    }

    Color RefractedC(0.0f, 0.0f, 0.0f);
    if (hitObject->material.transparency > 0) {
        glm::vec3 origin = intersect.point - intersect.normal * BIAS;
        glm::vec3 refractDirObjSpace = glm::mat3(glm::transpose(glm::inverse(hitObject->getTransformMatrix()))) * glm::refract(rayDirection, intersect.normal, hitObject->material.refractionIndex);
        RefractedC = castRay(origin, refractDirObjSpace, recursion + 1);
    }

    Material mat = hitObject->material;
    Color diffuseC;
    if (mat.texture != nullptr) {
        diffuseC = SurfaceColor(mat.texture, intersect.u, intersect.v);
    } else {
        diffuseC = mat.diffuse;
    }

    Color diffuseL = diffuseC * light.intensity * diffuseLightIntensity * hitObject->material.albedo * shadowIntensity;
    Color SpecularL = light.color * light.intensity * specLightIntensity * hitObject->material.specularAlbedo * shadowIntensity;

    Color color = (diffuseL + SpecularL) * (1.0f - hitObject->material.reflectivity - hitObject->material.transparency) + reflectedColor * hitObject->material.reflectivity + RefractedC * hitObject->material.transparency;
    return color;
}


void setUp() {

    Material stone = {
            Color(80, 0, 0),   // diffuse
            0.3,
            0.5,
            3.0f,
            0.0f,
            0.0f,
            1.6f,
            loadTexture("../assets/stone.png")
    };

    //Era lava pero termino pareciendo esmeralda :/
    Material lava = {
            Color(80, 0, 0),
            0.9f,
            1.0f,
            150.0f,
            0.2f,
            0.0f,
            0.0f,
            loadTexture("../assets/lava.png")
    };

    //diamond que terminó siendo carbon :/

    Material diamond(
            Color(80, 0, 0),   // diffuse
            0.3,
            0.5,
            3.0f,
            0.0f,
            0.0f,
            1.6f,
            loadTexture("../assets/diamond.png")

    );

    //iron
    Material iron(
            Color(80, 0, 0),   // diffuse
            0.3,
            0.5,
            3.0f,
            0.0f,
            0.0f,
            1.6f,
            loadTexture("../assets/iron.png")

    );

    // diamond block
    Material obsidian(
            Color(80, 0, 0),   // diffuse
            0.3,
            0.5,
            3.0f,
            0.0f,
            0.0f,
            1.6f,
            loadTexture("../assets/obsidian.png")

    );

    // dirt block
    Material dirt(
            Color(255, 255, 255),   // diffuse
            0.3,
            0.5,
            3.0f,
            0.0f,
            0.0f,
            1.6f,
            loadTexture("../assets/dirt.png")

    );

    // portal
    Material portal(
            Color(75,0,130),   // diffuse
            0.3,
            0.5,
            3.0f,
            0.0f,
            0.2f,
            0.0f,
            loadTexture("../assets/portal.png")

    );

    // Piso
    objects.push_back(new Cube(glm::vec3(1.0f, -3.0f, 1.0f), glm::vec3(2.0f, -2.0f, 2.0f), lava));
    objects.push_back(new Cube(glm::vec3(1.0f, -3.0f, 0.0f), glm::vec3(2.0f, -2.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(1.0f, -3.0f, 2.0f), glm::vec3(2.0f, -2.0f, 3.0f), diamond));
    objects.push_back(new Cube(glm::vec3(1.0f, -3.0f, 3.0f), glm::vec3(2.0f, -2.0f, 4.0f), stone));
    objects.push_back(new Cube(glm::vec3(1.0f, -3.0f, 4.0f), glm::vec3(2.0f, -2.0f, 5.0f), diamond));
    objects.push_back(new Cube(glm::vec3(1.0f, -3.0f, -1.0f), glm::vec3(2.0f, -2.0f, 0.0f), stone));

    objects.push_back(new Cube(glm::vec3(2.0f, -3.0f, -1.0f), glm::vec3(3.0f, -2.0f, 0.0f), lava));
    objects.push_back(new Cube(glm::vec3(2.0f, -3.0f, 2.0f), glm::vec3(3.0f, -2.0f, 3.0f), stone));
    objects.push_back(new Cube(glm::vec3(2.0f, -3.0f, 1.0f), glm::vec3(3.0f, -2.0f, 2.0f), diamond));
    objects.push_back(new Cube(glm::vec3(2.0f, -3.0f, 0.0f), glm::vec3(3.0f, -2.0f, 1.0f), lava));
    objects.push_back(new Cube(glm::vec3(2.0f, -3.0f, 3.0f), glm::vec3(3.0f, -2.0f, 4.0f), stone));

    objects.push_back(new Cube(glm::vec3(3.0f, -3.0f, -1.0f), glm::vec3(4.0f, -2.0f, 0.0f), stone));
    objects.push_back(new Cube(glm::vec3(3.0f, -3.0f, 0.0f), glm::vec3(4.0f, -2.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(3.0f, -3.0f, 1.0f), glm::vec3(4.0f, -2.0f, 2.0f), diamond));
    objects.push_back(new Cube(glm::vec3(3.0f, -3.0f, 2.0f), glm::vec3(4.0f, -2.0f, 3.0f), stone));

    objects.push_back(new Cube(glm::vec3(0.0f, -3.0f, -1.0f), glm::vec3(1.0f, -2.0f, 0.0f), diamond));
    objects.push_back(new Cube(glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(1.0f, -2.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(0.0f, -3.0f, 1.0f), glm::vec3(1.0f, -2.0f, 2.0f), iron));
    objects.push_back(new Cube(glm::vec3(0.0f, -3.0f, 2.0f), glm::vec3(1.0f, -2.0f, 3.0f), stone));
    objects.push_back(new Cube(glm::vec3(0.0f, -3.0f, 3.0f), glm::vec3(1.0f, -2.0f, 4.0f), iron));
    objects.push_back(new Cube(glm::vec3(0.0f, -3.0f, 4.0f), glm::vec3(1.0f, -2.0f, 5.0f), stone));

    // Estructura portal

    objects.push_back(new Cube(glm::vec3(0.0f, -2.0f, 1.0f), glm::vec3(1.0f, -1.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(1.0f, -2.0f, 1.0f), glm::vec3(2.0f, -1.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(-1.0f, -2.0f, 1.0f), glm::vec3(0.0f, -1.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(-2.0f, -2.0f, 1.0f), glm::vec3(-1.0f, -1.0f, 2.0f), obsidian));

    objects.push_back(new Cube(glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(2.0f, 0.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(2.0f, 1.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(2.0f, 2.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(1.0f, 2.0f, 1.0f), glm::vec3(2.0f, 3.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(1.0f, 2.0f, 1.0f), glm::vec3(2.0f, 3.0f, 2.0f), obsidian));

    objects.push_back(new Cube(glm::vec3(-2.0f, -1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(-2.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 1.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(-2.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 2.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(-2.0f, 2.0f, 1.0f), glm::vec3(-1.0f, 3.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(-2.0f, 2.0f, 1.0f), glm::vec3(-1.0f, 3.0f, 2.0f), obsidian));

    //portal
    objects.push_back(new Cube(glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 2.0f), portal));
    objects.push_back(new Cube(glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 2.0f), portal));
    objects.push_back(new Cube(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 2.0f, 2.0f), portal));
    objects.push_back(new Cube(glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 2.0f), portal));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 2.0f), portal));
    objects.push_back(new Cube(glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(1.0f, 2.0f, 2.0f), portal));

    objects.push_back(new Cube(glm::vec3(0.0f, 2.0f, 1.0f), glm::vec3(1.0f, 3.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(1.0f, 2.0f, 1.0f), glm::vec3(2.0f, 3.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(-1.0f, 2.0f, 1.0f), glm::vec3(0.0f, 3.0f, 2.0f), obsidian));
    objects.push_back(new Cube(glm::vec3(-2.0f, 2.0f, 1.0f), glm::vec3(-1.0f, 3.0f, 2.0f), obsidian));

    objects.push_back(new Cube(glm::vec3(-1.0f, -3.0f, -1.0f), glm::vec3(0.0f, -2.0f, 0.0f), dirt));
    objects.push_back(new Cube(glm::vec3(-1.0f, -3.0f, 0.0f), glm::vec3(0.0f, -2.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-1.0f, -3.0f, 1.0f), glm::vec3(0.0f, -2.0f, 2.0f), dirt));
    objects.push_back(new Cube(glm::vec3(-1.0f, -3.0f, 2.0f), glm::vec3(0.0f, -2.0f, 3.0f), diamond));
    objects.push_back(new Cube(glm::vec3(-1.0f, -3.0f, 3.0f), glm::vec3(0.0f, -2.0f, 4.0f), dirt));
    objects.push_back(new Cube(glm::vec3(-1.0f, -3.0f, 4.0f), glm::vec3(0.0f, -2.0f, 5.0f), stone));


    objects.push_back(new Cube(glm::vec3(-2.0f, -3.0f, -1.0f), glm::vec3(-1.0f, -2.0f, 0.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, -3.0f, 0.0f), glm::vec3(-1.0f, -2.0f, 1.0f), dirt));
    objects.push_back(new Cube(glm::vec3(-2.0f, -3.0f, 1.0f), glm::vec3(-1.0f, -2.0f, 2.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, -3.0f, 2.0f), glm::vec3(-1.0f, -2.0f, 3.0f), diamond));
    objects.push_back(new Cube(glm::vec3(-2.0f, -3.0f, 3.0f), glm::vec3(-1.0f, -2.0f, 4.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, -3.0f, 4.0f), glm::vec3(-1.0f, -2.0f, 5.0f), stone));

    objects.push_back(new Cube(glm::vec3(-3.0f, -3.0f, -1.0f), glm::vec3(-2.0f, -2.0f, 0.0f), dirt));
    objects.push_back(new Cube(glm::vec3(-3.0f, -3.0f, 0.0f), glm::vec3(-2.0f, -2.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-3.0f, -3.0f, 1.0f), glm::vec3(-2.0f, -2.0f, 2.0f), dirt));
    objects.push_back(new Cube(glm::vec3(-3.0f, -3.0f, 2.0f), glm::vec3(-2.0f, -2.0f, 3.0f), stone));
    objects.push_back(new Cube(glm::vec3(-3.0f, -3.0f, 3.0f), glm::vec3(-2.0f, -2.0f, 4.0f), stone));



    //pared
    objects.push_back(new Cube(glm::vec3(-3.0f, -2.0f, -2.0f), glm::vec3(-2.0f, -1.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, -2.0f, -2.0f), glm::vec3(-1.0f, -1.0f, -1.0f), diamond));
    objects.push_back(new Cube(glm::vec3(-1.0f, -2.0f, -2.0f), glm::vec3(0.0f, -1.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(0.0f, -2.0f, -2.0f), glm::vec3(1.0f, -1.0f, -1.0f), diamond));
    objects.push_back(new Cube(glm::vec3(1.0f, -2.0f, -2.0f), glm::vec3(2.0f, -1.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(2.0f, -2.0f, -2.0f), glm::vec3(3.0f, -1.0f, -1.0f), dirt));
    objects.push_back(new Cube(glm::vec3(3.0f, -2.0f, -2.0f), glm::vec3(4.0f, -1.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, -2.0f, -2.0f), glm::vec3(5.0f, -1.0f, -1.0f), stone));

    objects.push_back(new Cube(glm::vec3(-3.0f, -1.0f, -2.0f), glm::vec3(-2.0f, 0.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, -1.0f, -2.0f), glm::vec3(-1.0f, 0.0f, -1.0f), lava));
    objects.push_back(new Cube(glm::vec3(-1.0f, -1.0f, -2.0f), glm::vec3(0.0f, 0.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(0.0f, -1.0f, -2.0f), glm::vec3(1.0f, 0.0f, -1.0f), lava));
    objects.push_back(new Cube(glm::vec3(1.0f, -1.0f, -2.0f), glm::vec3(2.0f, 0.0f, -1.0f), dirt));
    objects.push_back(new Cube(glm::vec3(2.0f, -1.0f, -2.0f), glm::vec3(3.0f, 0.0f, -1.0f), diamond));
    objects.push_back(new Cube(glm::vec3(3.0f, -1.0f, -2.0f), glm::vec3(4.0f, 0.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, -1.0f, -2.0f), glm::vec3(5.0f, 0.0f, -1.0f), stone));

    objects.push_back(new Cube(glm::vec3(-3.0f, 0.0f, -2.0f), glm::vec3(-2.0f, 1.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, 0.0f, -2.0f), glm::vec3(-1.0f, 1.0f, -1.0f), lava));
    objects.push_back(new Cube(glm::vec3(-1.0f, 0.0f, -2.0f), glm::vec3(0.0f, 1.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(1.0f, 1.0f, -1.0f), diamond));
    objects.push_back(new Cube(glm::vec3(1.0f, 0.0f, -2.0f), glm::vec3(2.0f, 1.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(2.0f, 0.0f, -2.0f), glm::vec3(3.0f, 1.0f, -1.0f), lava));
    objects.push_back(new Cube(glm::vec3(3.0f, 0.0f, -2.0f), glm::vec3(4.0f, 1.0f, -1.0f), stone));

    objects.push_back(new Cube(glm::vec3(-3.0f, 1.0f, -2.0f), glm::vec3(-2.0f, 2.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, 1.0f, -2.0f), glm::vec3(-1.0f, 2.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-1.0f, 1.0f, -2.0f), glm::vec3(0.0f, 2.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(0.0f, 1.0f, -2.0f), glm::vec3(1.0f, 2.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(1.0f, 1.0f, -2.0f), glm::vec3(2.0f, 2.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(2.0f, 1.0f, -2.0f), glm::vec3(3.0f, 2.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(3.0f, 1.0f, -2.0f), glm::vec3(4.0f, 2.0f, -1.0f), stone));

    objects.push_back(new Cube(glm::vec3(-3.0f, 2.0f, -2.0f), glm::vec3(-2.0f, 3.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, 2.0f, -2.0f), glm::vec3(-1.0f, 3.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-1.0f, 2.0f, -2.0f), glm::vec3(0.0f, 3.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(1.0f, 3.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(1.0f, 2.0f, -2.0f), glm::vec3(2.0f, 3.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(2.0f, 2.0f, -2.0f), glm::vec3(3.0f, 3.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(3.0f, 2.0f, -2.0f), glm::vec3(4.0f, 3.0f, -1.0f), stone));

    objects.push_back(new Cube(glm::vec3(-3.0f, 3.0f, -2.0f), glm::vec3(-2.0f, 4.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(-2.0f, 3.0f, -2.0f), glm::vec3(-1.0f, 4.0f, -1.0f), lava));
    objects.push_back(new Cube(glm::vec3(-1.0f, 3.0f, -2.0f), glm::vec3(0.0f, 4.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(0.0f, 3.0f, -2.0f), glm::vec3(1.0f, 4.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(1.0f, 3.0f, -2.0f), glm::vec3(2.0f, 4.0f, -1.0f), diamond));
    objects.push_back(new Cube(glm::vec3(2.0f, 3.0f, -2.0f), glm::vec3(3.0f, 4.0f, -1.0f), stone));
    objects.push_back(new Cube(glm::vec3(3.0f, 3.0f, -2.0f), glm::vec3(4.0f, 4.0f, -1.0f), stone));


    objects.push_back(new Cube(glm::vec3(4.0f, 1.0f, -1.0f), glm::vec3(5.0f, 2.0f, 0.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, 1.0f, 0.0f), glm::vec3(5.0f, 2.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, 1.0f, 1.0f), glm::vec3(5.0f, 2.0f, 2.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, 1.0f, 2.0f), glm::vec3(5.0f, 2.0f, 3.0f), stone));

    objects.push_back(new Cube(glm::vec3(4.0f, 2.0f, -1.0f), glm::vec3(5.0f, 3.0f, 0.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, 2.0f, 0.0f), glm::vec3(5.0f, 3.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, 2.0f, 1.0f), glm::vec3(5.0f, 3.0f, 2.0f), diamond));

    objects.push_back(new Cube(glm::vec3(4.0f, 3.0f, -1.0f), glm::vec3(5.0f, 4.0f, 0.0f), diamond));
    objects.push_back(new Cube(glm::vec3(4.0f, 3.0f, 0.0f), glm::vec3(5.0f, 4.0f, 1.0f), stone));


    objects.push_back(new Cube(glm::vec3(4.0f, 0.0f, -1.0f), glm::vec3(5.0f, 1.0f, 0.0f), dirt));
    objects.push_back(new Cube(glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(5.0f, 1.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, 0.0f, 1.0f), glm::vec3(5.0f, 1.0f, 2.0f), lava));
    objects.push_back(new Cube(glm::vec3(4.0f, 0.0f, 2.0f), glm::vec3(5.0f, 1.0f, 3.0f), stone));


    objects.push_back(new Cube(glm::vec3(4.0f, -1.0f, -1.0f), glm::vec3(5.0f, 0.0f, 0.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, -1.0f, 0.0f), glm::vec3(5.0f, 0.0f, 1.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, -1.0f, 1.0f), glm::vec3(5.0f, 0.0f, 2.0f), diamond));
    objects.push_back(new Cube(glm::vec3(4.0f, -1.0f, 2.0f), glm::vec3(5.0f, 0.0f, 3.0f), stone));

    objects.push_back(new Cube(glm::vec3(4.0f, -2.0f, -1.0f), glm::vec3(5.0f, -1.0f, 0.0f), stone));
    objects.push_back(new Cube(glm::vec3(4.0f, -2.0f, 0.0f), glm::vec3(5.0f, -1.0f, 1.0f), dirt));
    objects.push_back(new Cube(glm::vec3(4.0f, -2.0f, 1.0f), glm::vec3(5.0f, -1.0f, 2.0f), diamond));
    objects.push_back(new Cube(glm::vec3(4.0f, -2.0f, 2.0f), glm::vec3(5.0f, -1.0f, 3.0f), stone));



}

void render() {
    float fov = 3.1415/3;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {

            float screenX = (2.0f * (x + 0.5f)) / SCREEN_WIDTH - 1.0f;
            float screenY = -(2.0f * (y + 0.5f)) / SCREEN_HEIGHT + 1.0f;
            screenX *= ASPECT_RATIO;
            screenX *= tan(fov/2.0f);
            screenY *= tan(fov/2.0f);


            glm::vec3 cameraDir = glm::normalize(camera.target - camera.position);

            glm::vec3 cameraX = glm::normalize(glm::cross(cameraDir, camera.up));
            glm::vec3 cameraY = glm::normalize(glm::cross(cameraX, cameraDir));
            glm::vec3 rayDirection = glm::normalize(
                    cameraDir + cameraX * screenX + cameraY * screenY
            );


            Color pixelColor = castRay(camera.position, rayDirection);

            if (pixelColor.i != 1) {
                point(glm::vec2(x, y), pixelColor);
            }
        }
    }
}


int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow("Raytracing - FPS: 0",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    int frameCount = 0;
    Uint32 startTime = SDL_GetTicks();
    Uint32 currentTime = startTime;

    setUp();
    bool reRender = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        camera.move(1.0f);
                        reRender = true;
                        break;
                    case SDLK_DOWN:
                        camera.move(-1.0f);
                        reRender = true;
                        break;
                    case SDLK_LEFT:
                        camera.rotate(-1.0f, 0.0f);
                        reRender = true;
                        break;
                    case SDLK_RIGHT:
                        camera.rotate(1.0f, 0.0f);
                        reRender = true;
                        break;
                }
            }


        }

        if (reRender) {
            reRender = false;

            render();
        }



        // Present the renderer
        SDL_RenderPresent(renderer);

        frameCount++;

        // Calculate and display FPS
        if (SDL_GetTicks() - currentTime >= 1000) {
            currentTime = SDL_GetTicks();
            std::string title = "Raytracing - FPS: " + std::to_string(frameCount);
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
