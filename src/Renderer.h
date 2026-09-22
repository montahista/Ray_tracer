#pragma once
#include <thread>
#include "Scene.h"
#include <vector>
#include <chrono>
#include <iostream>

class Renderer {
public:
    Renderer();
    bool initialize(int argc, char** argv, const Scene& scene);
    void render(const Scene& scene);
    void run();

    static void displayCallback();
    static void reshapeCallback(int w, int h);

private:
    struct Ray { Vec3 origin, direction; };

    struct HitInfo {
        bool  hit         = false;
        float t           = 1e9f;
        Vec3  point;
        Vec3  normal;
        Color color;
        float shininess   = 32.f;
        float reflectivity = 0.f;
        float transparency = 0.f;  
        float ior          = 1.5f;  
    };

    Ray     makeRay(int px, int py) const;
    Color   traceRay(const Ray& ray, const Scene& scene, int depth) const;
    HitInfo hitSphere(const Ray& ray, const Sphere& sphere) const;
    HitInfo hitPlane (const Ray& ray, const Plane&  plane)  const;
    HitInfo findClosest(const Ray& ray, const Scene& scene) const;
    Color   shade(const HitInfo& hit, const Ray& ray, const Scene& scene, int depth) const;

    int   width  = 800;
    int   height = 600;
    Scene currentScene;
    std::vector<unsigned char> pixels;

    static Renderer* instance;
    static const int MAX_DEPTH = 4;  
    static const int NUM_THREADS = 4;

    void renderStrip(const Scene& scene,
                 int startY, int endY,
                 int threadID); 
};