#include "Renderer.h"
#include <GL/freeglut.h>
#include <cmath>
#include <algorithm>
#include <iostream>

Renderer* Renderer::instance = nullptr;

Renderer::Renderer() : width(800), height(600) {}

bool Renderer::initialize(int argc, char** argv, const Scene& scene)
{
    width        = scene.resolution.width;
    height       = scene.resolution.height;
    currentScene = scene;
    instance     = this;

    pixels.assign(width * height * 3, 0);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Ray Tracer");

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);

    render(scene);

    glutMainLoop();
    return true;
}
Renderer::Ray Renderer::makeRay(int px, int py) const
{
    const Camera& cam = currentScene.camera;

    Vec3 forward = cam.viewDir.normalize();
    Vec3 right   = cam.up.cross(forward).normalize();
    Vec3 up      = forward.cross(right);

    float aspect = static_cast<float>(width) / height;
    float scale  = std::tan(60.f * 3.14159f / 180.f / 2.f);

    float ndcX = (2.f * (px + 0.5f) / width  - 1.f) * aspect * scale;
    float ndcY = (1.f - 2.f * (py + 0.5f) / height) * scale;

    Vec3 dir = (forward + right * ndcX + up * ndcY).normalize();
    return { cam.position, dir };
}
Renderer::HitInfo Renderer::hitSphere(const Ray& ray, const Sphere& sphere) const
{
    HitInfo info;
    Vec3  oc = ray.origin - sphere.center;
    float a  = ray.direction.dot(ray.direction);
    float b  = 2.f * oc.dot(ray.direction);
    float c  = oc.dot(oc) - sphere.radius * sphere.radius;
    float d  = b*b - 4*a*c;

    if(d < 0) return info;

    float t = (-b - std::sqrt(d)) / (2.f * a);
    if(t < 0.001f) return info;

    info.hit         = true;
    info.t           = t;
    info.point       = ray.origin + ray.direction * t;
    info.normal      = (info.point - sphere.center).normalize();
    info.color       = sphere.color;
    info.shininess   = sphere.shininess;
    info.reflectivity = sphere.reflectivity;
    info.color        = sphere.color;
    info.transparency = sphere.transparency; 
    info.ior          = sphere.ior;           

    return info;
}

Renderer::HitInfo Renderer::hitPlane(const Ray& ray, const Plane& plane) const
{
    HitInfo info;
    float denom = plane.normal.dot(ray.direction);

    if(std::abs(denom) < 1e-6f) return info;

    Vec3  diff = plane.point - ray.origin;
    float t    = diff.dot(plane.normal) / denom;

    if(t < 0.001f) return info;

    info.hit         = true;
    info.t           = t;
    info.point       = ray.origin + ray.direction * t;

    info.normal      = (denom < 0) ? plane.normal : -plane.normal;

    info.color       = plane.color;
    info.shininess   = plane.shininess;
    info.reflectivity = plane.reflectivity;
    return info;
}

Renderer::HitInfo Renderer::findClosest(const Ray& ray, const Scene& scene) const
{
    HitInfo closest;
    for(const auto& s : scene.spheres) {
        HitInfo h = hitSphere(ray, s);
        if(h.hit && h.t < closest.t) closest = h;
    }
    for(const auto& p : scene.planes) {
        HitInfo h = hitPlane(ray, p);
        if(h.hit && h.t < closest.t) closest = h;
    }
    return closest;
}

Color Renderer::shade(const HitInfo& hit, const Ray& ray,
                       const Scene& scene, int depth) const
{
    float ambR = hit.color.r * 0.1f;
    float ambG = hit.color.g * 0.1f;
    float ambB = hit.color.b * 0.1f;

    float totalR = ambR, totalG = ambG, totalB = ambB;

    for(const auto& light : scene.lights)
    {
        Vec3  toLight   = (light.position - hit.point).normalize();
        float dist      = (light.position - hit.point).length();

        Ray shadowRay { hit.point + hit.normal * 0.001f, toLight };
        HitInfo shadow = findClosest(shadowRay, scene);
        if(shadow.hit && shadow.t < dist) continue;  

        float diff = std::max(0.f, hit.normal.dot(toLight));

        Vec3  viewDir = (-ray.direction).normalize();
        Vec3  halfway = (toLight + viewDir).normalize();
        float spec    = std::pow(std::max(0.f, hit.normal.dot(halfway)),
                                 hit.shininess);

        float attn = light.intensity / (1.f + 0.01f * dist * dist);

        totalR += (hit.color.r * diff + spec) * light.color.r * attn;
        totalG += (hit.color.g * diff + spec) * light.color.g * attn;
        totalB += (hit.color.b * diff + spec) * light.color.b * attn;
    }


    if(depth < MAX_DEPTH && hit.reflectivity > 0.f)
    {
        Vec3 reflDir = ray.direction.reflect(hit.normal).normalize();
        Ray  reflRay { hit.point + hit.normal * 0.001f, reflDir };
        Color reflColor = traceRay(reflRay, scene, depth + 1);

        float k = hit.reflectivity;
        totalR = totalR * (1-k) + reflColor.r * k;
        totalG = totalG * (1-k) + reflColor.g * k;
        totalB = totalB * (1-k) + reflColor.b * k;
    }
    if(depth < MAX_DEPTH && hit.transparency > 0.f)
    {

        bool  entering = ray.direction.dot(hit.normal) < 0;
        float n1       = entering ? 1.0f : hit.ior; 
        float n2       = entering ? hit.ior : 1.0f;
        Vec3  n        = entering ? hit.normal : -hit.normal;

        Vec3 refractDir = ray.direction.refract(n, n1, n2);

        Color refractColor;

        if(refractDir.length() < 0.001f)
        {
            
            Vec3 reflDir = ray.direction.reflect(hit.normal).normalize();
            Ray  reflRay { hit.point + hit.normal * 0.01f, reflDir };
            refractColor = traceRay(reflRay, scene, depth + 1);
        }
         else
        {
            
            Ray refractRay {
                hit.point - n * 0.001f,   
                refractDir.normalize()
            };
            refractColor = traceRay(refractRay, scene, depth + 1);
        }

        float t = hit.transparency;
        totalR = totalR*(1-t) + refractColor.r*t;
        totalG = totalG*(1-t) + refractColor.g*t;
        totalB = totalB*(1-t) + refractColor.b*t;
    }

    return Color(
        std::min(totalR, 1.f) * 255.f,
        std::min(totalG, 1.f) * 255.f,
        std::min(totalB, 1.f) * 255.f
    );
}

Color Renderer::traceRay(const Ray& ray, const Scene& scene, int depth = 0) const
{
    HitInfo hit = findClosest(ray, scene);
    if(!hit.hit) return scene.background;
    return shade(hit, ray, scene, depth);
}
void Renderer::render(const Scene& scene)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    int rowsPerThread = height / NUM_THREADS;

    for(int t = 0; t < NUM_THREADS; ++t)
    {
        int startY = t * rowsPerThread;
        int endY   = (t == NUM_THREADS - 1) ? height : startY + rowsPerThread;

        threads.emplace_back(&Renderer::renderStrip,
                              this,
                              std::cref(scene),
                              startY, endY, t);
    }

    for(auto& th : threads)
        th.join();

    auto endTime = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    std::cout << "\n================================\n";
    std::cout << "  Multi-Thread Render\n";
    std::cout << "================================\n";
    std::cout << "  Resolution : " << width << " x " << height << "\n";
    std::cout << "  Threads    : " << NUM_THREADS << "\n";
    std::cout << "  Objects    : " << scene.spheres.size()
                                   + scene.planes.size() << "\n";
    std::cout << "  Time       : " << ms << " ms";
    if(ms > 1000)
        std::cout << " (" << ms / 1000.0 << " s)";
    std::cout << "\n  Rays/sec   : "
              << (long long)(width * height) * 1000 / (ms > 0 ? ms : 1)
              << "\n";
    std::cout << "================================\n\n";
}

void Renderer::displayCallback()
{
    if(!instance) return;
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(instance->width, instance->height,
                 GL_RGB, GL_UNSIGNED_BYTE, instance->pixels.data());
    glFlush();
}

void Renderer::renderStrip(const Scene& scene,
                            int startY, int endY,
                            int threadID)
{
    for(int y = startY; y < endY; ++y)
    for(int x = 0; x < width;  ++x)
    {
        Ray   ray   = makeRay(x, y);
        Color color = traceRay(ray, scene, 0);

        int idx = ((height - 1 - y) * width + x) * 3;
        pixels[idx+0] = static_cast<unsigned char>(std::min(color.r * 255.f, 255.f));
        pixels[idx+1] = static_cast<unsigned char>(std::min(color.g * 255.f, 255.f));
        pixels[idx+2] = static_cast<unsigned char>(std::min(color.b * 255.f, 255.f));
    }
}

void Renderer::reshapeCallback(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
}