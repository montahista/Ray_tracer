#pragma once
#include <vector>
#include <cmath>

struct Color {
    float r, g, b;
    Color(float r=0, float g=0, float b=0)
        : r(r/255.f), g(g/255.f), b(b/255.f) {}

    Color operator+(const Color& o) const { return {(r+o.r)*255.f, (g+o.g)*255.f, (b+o.b)*255.f}; }
    Color operator*(float t)        const { return {r*t*255.f, g*t*255.f, b*t*255.f}; }
};

struct Vec3 {
    float x, y, z;
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}

    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator*(float t)       const { return {x*t,   y*t,   z*t};   }
    Vec3 operator-()              const { return {-x, -y, -z};           }

    float dot(const Vec3& o)  const { return x*o.x + y*o.y + z*o.z; }
    float length()            const { return std::sqrt(x*x + y*y + z*z); }
    Vec3  normalize()         const {
        float len = length();
        return len > 0 ? Vec3(x/len, y/len, z/len) : Vec3(0,0,0);
    }
    Vec3 cross(const Vec3& o) const {
        return { y*o.z - z*o.y,
                 z*o.x - x*o.z,
                 x*o.y - y*o.x };
    }

    Vec3 reflect(const Vec3& normal) const {
        return *this - normal * (2.f * this->dot(normal));
    }

    Vec3 refract(const Vec3& normal, float n1, float n2) const
{
    float ratio = n1 / n2;
    float cosI  = -this->dot(normal);
    float sinT2 = ratio*ratio * (1.f - cosI*cosI);


    if(sinT2 > 1.f) return Vec3(0,0,0);

    float cosT = std::sqrt(1.f - sinT2);
    return *this * ratio + normal * (ratio*cosI - cosT);
}
};

struct Resolution { int width=800, height=600; };

struct Camera {
    Vec3 position  = {0, 0, -5};
    Vec3 viewDir   = {0, 0,  1};
    Vec3 up        = {0, 1,  0};
};

struct Sphere {
    Vec3  center;
    float radius       = 1.f;
    Color color        = Color(255, 100, 100);
    float shininess    = 32.f;
    float reflectivity = 0.f;   
    float transparency = 0.f;  
    float ior          = 1.5f;  
};

struct Plane {
    Vec3  point;
    Vec3  normal;
    Color color        = Color(200, 200, 200);
    float shininess    = 8.f;
    float reflectivity = 0.f;
};

struct Light {
    Vec3  position;
    Color color = Color(255, 255, 255);
    float intensity = 1.f;
};

struct Scene {
    Resolution          resolution;
    Color               background;
    Camera              camera;
    std::vector<Sphere> spheres;
    std::vector<Plane>  planes;
    std::vector<Light>  lights;
};